#include "pcpch.h"
#include "D3D12RenderDevice.h"

#include "Prism-Core/Base/Application.h"
#include "RenderAPI/D3D12/D3D12Buffer.h"
#include "RenderAPI/D3D12/D3D12RenderCommandQueue.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

#if USE_PIX
#include "WinPixEventRuntime/pix3.h"
#endif

// AgilitySDK
extern "C"
{
__declspec(dllexport) extern const UINT D3D12SDKVersion = 614;
}

extern "C"
{
__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}


namespace Prism::Render::D3D12
{
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
#if USE_PIX
	if (params.initPixLibrary)
	{
		// In order to use pix library include wrl/client.h and WinPixEventRuntime/pix3.h after it
		m_pixGpuCaptureModule = PIXLoadLatestWinPixGpuCapturerLibrary();
		m_pixTimingCaptureModule = PIXLoadLatestWinPixTimingCapturerLibrary();
	}
#endif

#ifdef PE_BUILD_DEBUG
	if (params.enableDebugLayer)
	{
		ComPtr<ID3D12Debug> debugController;
		PE_ASSERT_HR(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	uint32_t dxgiFactoryFlags = 0;
#ifdef PE_BUILD_DEBUG
	if (params.enableDebugLayer)
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	PE_ASSERT_HR(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

	PE_ASSERT_HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3dDevice)));

	// Make sure that required features are supported
	D3D12_FEATURE_DATA_D3D12_OPTIONS12 requiredFeatureSet = {};
	PE_ASSERT_HR(m_d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_D3D12_OPTIONS12, &requiredFeatureSet, sizeof(requiredFeatureSet)));

	PE_ASSERT(requiredFeatureSet.EnhancedBarriersSupported == TRUE);

#ifdef PE_BUILD_DEBUG
	if (params.enableDebugLayer)
	{
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{
			PE_ASSERT_HR(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE));
			PE_ASSERT_HR(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE));
			PE_ASSERT_HR(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, TRUE));
		}

		ComPtr<ID3D12InfoQueue> d3d12InfoQueue;
		if (SUCCEEDED(m_d3dDevice->QueryInterface(IID_PPV_ARGS(&d3d12InfoQueue))))
		{
			PE_ASSERT_HR(d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
			PE_ASSERT_HR(d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE));
			PE_ASSERT_HR(d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE));
		}
	}
#endif

	m_commandQueue = new D3D12RenderCommandQueue;

	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = m_d3dDevice->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = m_d3dDevice->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = m_d3dDevice->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = m_d3dDevice->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	m_dynamicBufferAllocator.Init(1024);
}

D3D12RenderDevice::~D3D12RenderDevice()
{
	m_dynamicBufferAllocator.CloseCmdListAllocations(D3D12RenderDevice::GetRenderQueue()->GetLastSubmittedCmdListFenceValue());
	m_dynamicBufferAllocator.ReleaseStaleAllocations(D3D12RenderDevice::GetRenderQueue()->GetLastCompletedCmdListFenceValue());

#if USE_PIX
	FreeLibrary(m_pixGpuCaptureModule);
	FreeLibrary(m_pixTimingCaptureModule);
#endif
}

RenderCommandQueue* D3D12RenderDevice::GetRenderQueue() const
{
	return m_commandQueue;
}

void D3D12RenderDevice::ReleaseStaleResources()
{
	RenderDevice::ReleaseStaleResources();

	m_dynamicBufferAllocator.CloseCmdListAllocations(GetRenderQueue()->GetLastSubmittedCmdListFenceValue());
	m_dynamicBufferAllocator.ReleaseStaleAllocations(GetRenderQueue()->GetLastCompletedCmdListFenceValue());
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
	return static_cast<D3D12RenderCommandQueue*>(GetRenderQueue())->GetD3D12CommandQueue();
}

CPUDescriptorHeapAllocation D3D12RenderDevice::AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t count)
{
	PE_ASSERT(m_cpuDescriptorHeapManagers.contains(type));
	return m_cpuDescriptorHeapManagers.at(type).Allocate(count);
}

GPUDescriptorHeapAllocation D3D12RenderDevice::CopyToGPUHeap(const CPUDescriptorHeapAllocation& cpuAllocation)
{
	auto heapType = cpuAllocation.GetOwningHeap()->GetHeapType();
	PE_ASSERT(m_cpuDescriptorHeapManagers.contains(heapType));

	auto gpuAllocation = m_gpuDescriptorHeapManagers.at(heapType).Allocate(cpuAllocation.GetNumHandles());
	m_d3dDevice->CopyDescriptorsSimple(gpuAllocation.GetNumHandles(),
									   gpuAllocation.GetCPUHandle(), cpuAllocation.GetCPUHandle(),
									   gpuAllocation.GetOwningHeap()->GetHeapType());

	return gpuAllocation;
}

GPUDescriptorHeapAllocation D3D12RenderDevice::CopyToGPUHeap(std::span<const CPUDescriptorHeapAllocation*> cpuAllocations)
{
	if (cpuAllocations.empty())
		return {};

	D3D12_DESCRIPTOR_HEAP_TYPE heapType = cpuAllocations[0]->GetOwningHeap()->GetHeapType();
	int32_t totalDescriptors = 0;
	for (auto& cpuAllocation : cpuAllocations)
	{
		PE_ASSERT(cpuAllocation);
		PE_ASSERT(heapType == cpuAllocation->GetOwningHeap()->GetHeapType());
		PE_ASSERT(m_cpuDescriptorHeapManagers.contains(heapType));
		totalDescriptors += cpuAllocation->GetNumHandles();
	}

	auto gpuAllocation = m_gpuDescriptorHeapManagers.at(heapType).Allocate(totalDescriptors);
	int32_t currentHandle = 0;
	for (auto& cpuAllocation : cpuAllocations)
	{
		PE_ASSERT(cpuAllocation);

		m_d3dDevice->CopyDescriptorsSimple(cpuAllocation->GetNumHandles(),
										   gpuAllocation.GetCPUHandle(currentHandle), cpuAllocation->GetCPUHandle(),
										   gpuAllocation.GetOwningHeap()->GetHeapType());

		currentHandle += cpuAllocation->GetNumHandles();
	}

	return gpuAllocation;
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

DynamicGPURingBuffer::DynamicAllocation D3D12RenderDevice::AllocateDynamicBufferMemory(int64_t size)
{
	return m_dynamicBufferAllocator.Allocate(size);
}
}
