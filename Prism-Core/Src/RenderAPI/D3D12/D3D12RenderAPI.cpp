#include "pcpch.h"
#include "D3D12RenderAPI.h"

#include "Prism-Core/Render/Renderer.h"
#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"


namespace Prism::D3D12
{
D3D12RenderAPI* g_d3d12RenderAPI = nullptr;

D3D12RenderAPI* D3D12RenderAPI::Get()
{
	return g_d3d12RenderAPI;
}

D3D12RenderAPI::D3D12RenderAPI()
{
	g_d3d12RenderAPI = this;

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

	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_descriptorHandleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	PE_ASSERT_HR(m_d3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue)));

	PE_ASSERT_HR(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	PE_ASSERT_HR(m_d3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)));

	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

ID3D12Device* D3D12RenderAPI::GetD3DDevice() const
{
	PE_ASSERT(m_d3dDevice.Get());
	return m_d3dDevice.Get();
}

IDXGIFactory* D3D12RenderAPI::GetDXGIFactory() const
{
	PE_ASSERT(m_dxgiFactory.Get());
	return m_dxgiFactory.Get();
}

ID3D12CommandQueue* D3D12RenderAPI::GetCommandQueue() const
{
	PE_ASSERT(m_commandQueue.Get());
	return m_commandQueue.Get();
}

CPUDescriptorHeapManager& D3D12RenderAPI::GetCPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	PE_ASSERT(m_cpuDescriptorHeapManagers.contains(type));
	return m_cpuDescriptorHeapManagers.at(type);
}

const CPUDescriptorHeapManager& D3D12RenderAPI::GetCPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
	PE_ASSERT(m_cpuDescriptorHeapManagers.contains(type));
	return m_cpuDescriptorHeapManagers.at(type);
}

GPUDescriptorHeapManager& D3D12RenderAPI::GetGPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	PE_ASSERT(m_gpuDescriptorHeapManagers.contains(type));
	return m_gpuDescriptorHeapManagers.at(type);
}

const GPUDescriptorHeapManager& D3D12RenderAPI::GetGPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
	PE_ASSERT(m_gpuDescriptorHeapManagers.contains(type));
	return m_gpuDescriptorHeapManagers.at(type);
}

uint32_t D3D12RenderAPI::GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
	PE_ASSERT(m_descriptorHandleSizes.contains(type));
	return m_descriptorHandleSizes.at(type);
}

void D3D12RenderAPI::Begin()
{
	PE_ASSERT_HR(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
}

void D3D12RenderAPI::End()
{
	PE_ASSERT_HR(m_commandList->Close());
}

void D3D12RenderAPI::Draw(Render::DrawCommandDesc desc)
{
	m_commandList->DrawInstanced(desc.numVertices, desc.numInstances, desc.startVertexLocation, 0);
}

void D3D12RenderAPI::DrawIndexed(Render::DrawIndexedCommandDesc desc)
{
	m_commandList->DrawIndexedInstanced(desc.numIndices, desc.numInstances, desc.startIndexLocation, desc.baseVertexLocation, 0);
}

void D3D12RenderAPI::SetPSO(Render::GraphicsPipelineState* pso)
{
	m_commandList->SetPipelineState(static_cast<D3D12GraphicsPipelineState*>(pso)->GetD3D12PipelineState());
}
}
