#include "pcpch.h"
#include "D3D12RenderAPI.h"

#include "Prism-Core/Render/Renderer.h"
#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
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
	PE_ASSERT_HR(m_commandList->Close());

	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_cpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_gpuDescriptorHeapManagers.try_emplace(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	PE_ASSERT_HR(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_mainFence)));
}

ID3D12Device* D3D12RenderAPI::GetD3DDevice() const
{
	PE_ASSERT(m_d3dDevice.Get());
	return m_d3dDevice.Get();
}

IDXGIFactory2* D3D12RenderAPI::GetDXGIFactory() const
{
	PE_ASSERT(m_dxgiFactory.Get());
	return m_dxgiFactory.Get();
}

ID3D12CommandQueue* D3D12RenderAPI::GetCommandQueue() const
{
	PE_ASSERT(m_commandQueue.Get());
	return m_commandQueue.Get();
}

DescriptorHeapAllocation D3D12RenderAPI::AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t count)
{
	PE_ASSERT(m_cpuDescriptorHeapManagers.contains(type));
	return m_cpuDescriptorHeapManagers.at(type).Allocate(count);
}

uint32_t D3D12RenderAPI::GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
	PE_ASSERT(m_descriptorHandleSizes.contains(type));
	return m_descriptorHandleSizes.at(type);
}

void D3D12RenderAPI::Begin()
{
	PE_ASSERT_HR(m_commandAllocator->Reset());
	PE_ASSERT_HR(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
}

void D3D12RenderAPI::End()
{
	PE_ASSERT_HR(m_commandList->Close());
	std::array<ID3D12CommandList*, 1> commandLists = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(commandLists.size(), commandLists.data());
}

void D3D12RenderAPI::FlushCommandQueue()
{
	++m_mainFenceValue;

	PE_ASSERT_HR(m_commandQueue->Signal(m_mainFence.Get(), m_mainFenceValue));

	if (m_mainFence->GetCompletedValue() < m_mainFenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

		PE_ASSERT_HR(m_mainFence->SetEventOnCompletion(m_mainFenceValue, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void D3D12RenderAPI::Draw(DrawCommandDesc desc)
{
	m_commandList->DrawInstanced(desc.numVertices, desc.numInstances, desc.startVertexLocation, 0);
}

void D3D12RenderAPI::DrawIndexed(DrawIndexedCommandDesc desc)
{
	m_commandList->DrawIndexedInstanced(desc.numIndices, desc.numInstances, desc.startIndexLocation, desc.baseVertexLocation, 0);
}

void D3D12RenderAPI::SetPSO(GraphicsPipelineState* pso)
{
	m_commandList->SetPipelineState(static_cast<D3D12GraphicsPipelineState*>(pso)->GetD3D12PipelineState());
}

void D3D12RenderAPI::SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv)
{
	PE_ASSERT(!rtvs.empty());

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
	handles.reserve(rtvs.size());
	for (TextureView* rtv : rtvs)
	{
		if (rtv)
			handles.push_back(static_cast<D3D12TextureView*>(rtv)->GetDescriptor().GetCPUHandle());
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
	if (dsv)
		dsvHandle = static_cast<D3D12TextureView*>(dsv)->GetDescriptor().GetCPUHandle();

	m_commandList->OMSetRenderTargets((UINT)handles.size(), handles.data(), false, dsv ? &dsvHandle : nullptr);
}

void D3D12RenderAPI::SetViewports(std::vector<Viewport> viewports)
{
	std::vector<D3D12_VIEWPORT> d3d12Viewports;
	d3d12Viewports.reserve(viewports.size());
	for (Viewport viewport : viewports)
		d3d12Viewports.push_back(GetD3D12Viewport(viewport));

	m_commandList->RSSetViewports((UINT)d3d12Viewports.size(), d3d12Viewports.data());
}

void D3D12RenderAPI::SetScissors(std::vector<Scissor> scissors)
{
	std::vector<D3D12_RECT> d3d12Rects;
	d3d12Rects.reserve(scissors.size());
	for (Scissor scissor : scissors)
		d3d12Rects.push_back(GetD3D12Rect(scissor));

	m_commandList->RSSetScissorRects((UINT)d3d12Rects.size(), d3d12Rects.data());
}

void D3D12RenderAPI::ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor)
{
	float rtClearColor[4] = {0.f};
	if (clearColor)
	{
		rtClearColor[0] = clearColor->r;
		rtClearColor[1] = clearColor->g;
		rtClearColor[2] = clearColor->b;
		rtClearColor[3] = clearColor->a;
	}
	else
	{
		TextureDesc texDesc = rtv->GetTexture()->GetTextureDesc();
		glm::float4 color = std::get<RenderTargetClearValue>(texDesc.optimizedClearValue).color;
		rtClearColor[0] = color.r;
		rtClearColor[1] = color.g;
		rtClearColor[2] = color.b;
		rtClearColor[3] = color.a;
	}
	m_commandList->ClearRenderTargetView(static_cast<D3D12TextureView*>(rtv)->GetDescriptor().GetCPUHandle(),
										 rtClearColor, 0, nullptr);
}

void D3D12RenderAPI::ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue)
{
	float depthValue = 0.f;
	uint8_t stencilValue = 0.f;
	if (clearValue)
	{
		depthValue = clearValue->depth;
		stencilValue = clearValue->stencil;
	}
	else
	{
		TextureDesc texDesc = dsv->GetTexture()->GetTextureDesc();
		auto [depth, stencil] = std::get<DepthStencilClearValue>(texDesc.optimizedClearValue).depthStencil;
		depthValue = depth;
		stencilValue = stencil;
	}
	m_commandList->ClearDepthStencilView(static_cast<D3D12TextureView*>(dsv)->GetDescriptor().GetCPUHandle(),
										 GetD3D12ClearFlags(flags), depthValue, stencilValue, 0, nullptr);
}

void D3D12RenderAPI::Transition(StateTransitionDesc desc)
{
	CD3DX12_RESOURCE_BARRIER barrier = GetD3D12ResourceBarrier(desc);
	m_commandList->ResourceBarrier(1, &barrier);
}
}
