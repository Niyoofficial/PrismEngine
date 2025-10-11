#include "pcpch.h"
#include "D3D12RenderDevice.h"

#include "Prism/Base/Application.h"
#include "RenderAPI/D3D12/D3D12Buffer.h"
#include "RenderAPI/D3D12/D3D12RenderCommandQueue.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

#if PE_USE_PIX
#include "pix3.h"
#endif

#include "imgui_impl_dx12.h"
#include "RenderAPI/D3D12/D3D12RootSignature.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"

// AgilitySDK
extern "C"
{
__declspec(dllexport) extern const UINT D3D12SDKVersion = 616;
}

extern "C"
{
__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}


namespace Prism::Render::D3D12
{
D3D12RenderDevice* g_renderDevice = nullptr;

D3D12RenderDevice& D3D12RenderDevice::Get()
{
	return static_cast<D3D12RenderDevice&>(RenderDevice::Get());
}

D3D12RenderDevice* D3D12RenderDevice::TryGet()
{
	return static_cast<D3D12RenderDevice*>(RenderDevice::TryGet());
}

D3D12RenderDevice::D3D12RenderDevice(RenderDeviceParams params)
	: RenderDevice(params)
{
#if PE_USE_PIX
	if (params.initPixLibrary)
	{
		// In order to use pix library include wrl/client.h and pix3.h after it
		m_pixGpuCaptureModule = PIXLoadLatestWinPixGpuCapturerLibrary();
		m_pixTimingCaptureModule = PIXLoadLatestWinPixTimingCapturerLibrary();
	}
#endif

#if PE_BUILD_DEBUG || PE_BUILD_PROFILE
	if (params.enableDebugLayer)
	{
		ComPtr<ID3D12Debug6> d3d12DebugController;
		PE_ASSERT_HR(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12DebugController)));
		d3d12DebugController->EnableDebugLayer();
		d3d12DebugController->SetEnableAutoName(TRUE);
		//d3d12DebugController->SetEnableGPUBasedValidation(TRUE);
		d3d12DebugController->SetEnableSynchronizedCommandQueueValidation(TRUE);

		ComPtr<IDXGIDebug1> dxgiDebugController;
		PE_ASSERT_HR(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebugController)));
		dxgiDebugController->EnableLeakTrackingForThread();
	}
#endif

	g_renderDevice = this;

	uint32_t dxgiFactoryFlags = 0;
#if PE_BUILD_DEBUG || PE_BUILD_PROFILE
	if (params.enableDebugLayer)
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	PE_ASSERT_HR(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

	DXGI_ADAPTER_DESC1 desc;
	{
		uint32_t adapter = 0;
		SIZE_T maxDedicatedVideoMemory = 0;
		for (uint32_t i = 0; m_dxgiFactory->EnumAdapters1(i, &m_dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++)
		{
			PE_ASSERT_HR(m_dxgiAdapter->GetDesc1(&desc));
			SIZE_T dedicatedVideoMemory = desc.DedicatedVideoMemory;

			if (dedicatedVideoMemory > maxDedicatedVideoMemory && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
			{
				maxDedicatedVideoMemory = dedicatedVideoMemory;
				adapter = i;
			}
		}

		PE_ASSERT_HR(m_dxgiFactory->EnumAdapters1(adapter, &m_dxgiAdapter));

		PE_ASSERT_HR(m_dxgiAdapter->GetDesc1(&desc));
	}

	PE_ASSERT(m_dxgiAdapter, "Could not find a suitable GPU to render the application");
	PE_RENDER_LOG(Info, "Selected GPU for rendering: {}", WStringToString(std::wstring(desc.Description)));

	PE_ASSERT_HR(D3D12CreateDevice(m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3dDevice)));

	// Make sure that required features are supported
	D3D12_FEATURE_DATA_D3D12_OPTIONS12 requiredFeatureSet = {};
	PE_ASSERT_HR(m_d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_D3D12_OPTIONS12, &requiredFeatureSet, sizeof(requiredFeatureSet)));

	PE_ASSERT(requiredFeatureSet.EnhancedBarriersSupported == TRUE, "Enhanced barriers not supported!");

#if PE_BUILD_DEBUG || PE_BUILD_PROFILE
	if (params.enableDebugLayer)
	{
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{
			PE_ASSERT_HR(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE));
			PE_ASSERT_HR(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE));
			//PE_ASSERT_HR(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, TRUE));
		}

		ComPtr<ID3D12InfoQueue> d3d12InfoQueue;
		if (SUCCEEDED(m_d3dDevice->QueryInterface(IID_PPV_ARGS(&d3d12InfoQueue))))
		{
			PE_ASSERT_HR(d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
			PE_ASSERT_HR(d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE));
			//PE_ASSERT_HR(d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE));
		}
	}
#endif

	m_graphicsRootSignature = new D3D12RootSignature(PipelineStateType::Graphics);
	m_computeRootSignature = new D3D12RootSignature(PipelineStateType::Compute);
	m_commandQueue = new D3D12RenderCommandQueue;

	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = m_d3dDevice->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = m_d3dDevice->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = m_d3dDevice->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = m_d3dDevice->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Constants::DESCRIPTOR_COUNT_PER_CPU_HEAP);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, Constants::DESCRIPTOR_COUNT_PER_CPU_HEAP);
	// This heap is required by the ClearUnorderedAccessView* functions
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Constants::DESCRIPTOR_COUNT_PER_CPU_HEAP);

	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Constants::DESCRIPTOR_COUNT_PER_GPU_HEAP);
	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, Constants::DESCRIPTOR_COUNT_PER_GPU_SAMPLER_HEAP);

	m_dynamicBufferAllocator.Init(1024);
}

D3D12RenderDevice::~D3D12RenderDevice()
{
	D3D12RenderDevice::ReleaseStaleResources();

#if PE_USE_PIX
	FreeLibrary(m_pixGpuCaptureModule);
	FreeLibrary(m_pixTimingCaptureModule);
#endif
}

void D3D12RenderDevice::ImGuiNewFrame()
{
	ImGui_ImplDX12_NewFrame();
}

RenderCommandQueue* D3D12RenderDevice::GetRenderCommandQueue() const
{
	return m_commandQueue;
}

void D3D12RenderDevice::ReleaseStaleResources()
{
	RenderDevice::ReleaseStaleResources();

	m_dynamicBufferAllocator.CloseCmdListAllocations(GetRenderCommandQueue()->GetLastSubmittedCmdListFenceValue());
	m_dynamicBufferAllocator.ReleaseStaleAllocations(GetRenderCommandQueue()->GetCompletedFenceValue());
}

int64_t D3D12RenderDevice::GetTotalSizeInBytes(BufferDesc buffDesc) const
{
	auto d3d12Desc = GetD3D12ResourceDesc(buffDesc);
	UINT64 size;
	GetD3D12Device()->GetCopyableFootprints1(&d3d12Desc, 0, 1, 0, nullptr, nullptr, nullptr, &size);
	return (int64_t)size;
}

int64_t D3D12RenderDevice::GetTotalSizeInBytes(TextureDesc texDesc, int32_t firstSubresource, int32_t numSubresources) const
{
	auto d3d12Desc = GetD3D12ResourceDesc(texDesc);
	UINT64 size;
	GetD3D12Device()->GetCopyableFootprints1(&d3d12Desc,
											 firstSubresource, numSubresources == -1 ? texDesc.GetSubresourceCount() : numSubresources,
											 0, nullptr, nullptr, nullptr, &size);

	return (int64_t)size;
}

SubresourceFootprint D3D12RenderDevice::GetSubresourceFootprint(TextureDesc texDesc, int32_t subresourceIndex) const
{
	auto d3d12Desc = GetD3D12ResourceDesc(texDesc);
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	GetD3D12Device()->GetCopyableFootprints1(&d3d12Desc, subresourceIndex, 1,
											 0, &layout, nullptr, nullptr, nullptr);

	return {
		.size = {layout.Footprint.Width, layout.Footprint.Height, layout.Footprint.Depth},
		.rowPitch = layout.Footprint.RowPitch
	};
}

int64_t D3D12RenderDevice::GetTexturePitchAlignment() const
{
	return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}

D3D12ShaderCompiler* D3D12RenderDevice::GetD3D12ShaderCompiler() const
{
	return static_cast<D3D12ShaderCompiler*>(GetShaderCompiler());
}

ID3D12Device10* D3D12RenderDevice::GetD3D12Device() const
{
	PE_ASSERT(m_d3dDevice.Get());
	return m_d3dDevice.Get();
}

IDXGIFactory2* D3D12RenderDevice::GetDXGIFactory() const
{
	PE_ASSERT(m_dxgiFactory.Get());
	return m_dxgiFactory.Get();
}

ID3D12CommandQueue* D3D12RenderDevice::GetD3D12CommandQueue() const
{
	return static_cast<D3D12RenderCommandQueue*>(GetRenderCommandQueue())->GetD3D12CommandQueue();
}

D3D12RootSignature* D3D12RenderDevice::GetGlobalRootSignature(PipelineStateType type) const
{
	return type == PipelineStateType::Graphics ? m_graphicsRootSignature : m_computeRootSignature;
}

D3D12PipelineStateCache& D3D12RenderDevice::GetPipelineStateCache()
{
	return m_pipelineStateCache;
}

const D3D12PipelineStateCache& D3D12RenderDevice::GetPipelineStateCache() const
{
	return m_pipelineStateCache;
}

DescriptorHeapAllocation D3D12RenderDevice::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t count)
{
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
	{
		return AllocateDescriptors(type, HeapDeviceType::GPU, count);
	}
	else if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
	{
		return AllocateDescriptors(type, HeapDeviceType::CPU, count);
	}
	else
	{
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

DescriptorHeapAllocation D3D12RenderDevice::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, HeapDeviceType deviceType, int32_t count)
{
	if (deviceType == HeapDeviceType::GPU)
	{
		PE_ASSERT(m_gpuDescriptorHeapManagers.contains(type));
		return m_gpuDescriptorHeapManagers.at(type).Allocate(count);
	}
	else if (deviceType == HeapDeviceType::CPU)
	{
		PE_ASSERT(m_cpuDescriptorHeapManagers.contains(type));
		return m_cpuDescriptorHeapManagers.at(type).Allocate(count);
	}
	else
	{
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

std::array<ID3D12DescriptorHeap*, 2> D3D12RenderDevice::GetGPUDescriptorHeaps() const
{
	std::array<ID3D12DescriptorHeap*, 2> gpuHeaps;

	int32_t index = 0;
	for (auto& [type, manager] : m_gpuDescriptorHeapManagers)
	{
		gpuHeaps[index] = manager.GetDescriptorHeap().GetD3D12DescriptorHeap();
		++index;
	}

	return gpuHeaps;
}

uint32_t D3D12RenderDevice::GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
	PE_ASSERT(m_descriptorHandleSizes.contains(type));
	return m_descriptorHandleSizes.at(type);
}

DynamicBufferAllocator::Allocation D3D12RenderDevice::AllocateDynamicBufferMemory(int64_t size)
{
	return m_dynamicBufferAllocator.Allocate(size);
}

ID3D12Resource* D3D12RenderDevice::GetD3D12ResourceForDynamicAllocation(int64_t ringBufferID) const
{
	return m_dynamicBufferAllocator.GetD3D12Resource(ringBufferID);
}

void D3D12RenderDevice::InitializeImGui(Core::Window* window, TextureFormat depthFormat)
{
	DXGI_FORMAT format = GetDXGIFormat(window->GetSwapchain()->GetSwapchainDesc().format);

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = GetD3D12Device();
	initInfo.CommandQueue = GetD3D12CommandQueue();
	initInfo.NumFramesInFlight = Constants::MAX_FRAMES_IN_FLIGHT;
	initInfo.RTVFormat = format;
	initInfo.DSVFormat = GetDXGIFormat(depthFormat);
	initInfo.SrvDescriptorHeap = m_gpuDescriptorHeapManagers.at(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetDescriptorHeap().GetD3D12DescriptorHeap();
	initInfo.SrvDescriptorAllocFn = 
		[](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
		{
			DescriptorHeapAllocation descriptor = g_renderDevice->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			*out_cpu_desc_handle = descriptor.GetCPUHandle();
			*out_gpu_desc_handle = descriptor.GetGPUHandle();
			g_renderDevice->m_imGuiAllocations.push_back(std::move(descriptor));
		};
	initInfo.SrvDescriptorFreeFn = 
		[](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle)
		{
			PE_ASSERT(!g_renderDevice->m_imGuiAllocations.empty());
			const auto [first, last] = std::ranges::remove_if(g_renderDevice->m_imGuiAllocations,
				[cpu_desc_handle, gpu_desc_handle](const DescriptorHeapAllocation& allocation)
				{
					return
						allocation.GetCPUHandle().ptr == cpu_desc_handle.ptr &&
						allocation.GetGPUHandle().ptr == gpu_desc_handle.ptr;
				});
			g_renderDevice->m_imGuiAllocations.erase(first, last);
		};
	ImGui_ImplDX12_Init(&initInfo);

	initializedImGui = true;
}

void D3D12RenderDevice::ShutdownImGui()
{
	if (initializedImGui)
	{
		ImGui_ImplDX12_Shutdown();
		initializedImGui = false;
	}
}
}
