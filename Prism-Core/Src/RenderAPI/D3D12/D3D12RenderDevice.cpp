#include "pcpch.h"
#include "D3D12RenderDevice.h"

#include "Prism-Core/Base/Application.h"
#include "RenderAPI/D3D12/D3D12RenderContext.h"

#if USE_PIX
#include "WinPixEventRuntime/pix3.h"
#endif

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }


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
	{
		ComPtr<ID3D12Debug> debugController;
		PE_ASSERT_HR(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	uint32_t dxgiFactoryFlags = 0;
#ifdef PE_BUILD_DEBUG
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

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	PE_ASSERT_HR(m_d3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue)));
	PE_ASSERT_HR(m_commandQueue->SetName(L"Main Cmd Queue"));

	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	PE_ASSERT_HR(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_mainFence)));
	PE_ASSERT_HR(m_mainFence->SetName(L"Main Fence"));

	m_dynamicBufferAllocator.Init(1024);
}

D3D12RenderDevice::~D3D12RenderDevice()
{
	D3D12RenderDevice::FlushCommandQueue();
	m_dynamicBufferAllocator.CloseCmdListAllocations(m_mainFenceValue);
	m_dynamicBufferAllocator.ReleaseStaleAllocations(D3D12RenderDevice::GetLastCompletedCmdListFenceValue());

#if USE_PIX
	FreeLibrary(m_pixGpuCaptureModule);
	FreeLibrary(m_pixTimingCaptureModule);
#endif
}

uint64_t D3D12RenderDevice::SubmitContext(RenderContext* context)
{
	auto* d3d12Context = static_cast<D3D12RenderContext*>(context);
	d3d12Context->CloseContext();

	m_contextReleaseQueue.AddResource(Ref(context), m_mainFenceValue + 1);

	std::array<ID3D12CommandList*, 1> commandLists = {d3d12Context->GetCommandList()};
	m_commandQueue->ExecuteCommandLists((UINT)commandLists.size(), commandLists.data());

	++m_mainFenceValue;
	PE_ASSERT_HR(m_commandQueue->Signal(m_mainFence.Get(), m_mainFenceValue));

	return GetLastSubmittedCmdListFenceValue();
}

void D3D12RenderDevice::WaitForCmdListToComplete(uint64_t fenceValue)
{
	if (m_mainFence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

		PE_ASSERT_HR(m_mainFence->SetEventOnCompletion(m_mainFenceValue, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void D3D12RenderDevice::FlushCommandQueue()
{
	WaitForCmdListToComplete(m_mainFenceValue);
}

void D3D12RenderDevice::ReleaseStaleResources()
{
	m_contextReleaseQueue.PurgeReleaseQueue(GetLastCompletedCmdListFenceValue());

	m_dynamicBufferAllocator.CloseCmdListAllocations(m_mainFenceValue);
	m_dynamicBufferAllocator.ReleaseStaleAllocations(GetLastCompletedCmdListFenceValue());
}

uint64_t D3D12RenderDevice::GetLastSubmittedCmdListFenceValue() const
{
	return m_mainFenceValue;
}

uint64_t D3D12RenderDevice::GetLastCompletedCmdListFenceValue() const
{
	return m_mainFence->GetCompletedValue();
}

ID3D12Device* D3D12RenderDevice::GetD3D12Device() const
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
	PE_ASSERT(m_commandQueue.Get());

	return m_commandQueue.Get();
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
