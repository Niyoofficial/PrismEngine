#include "pcpch.h"
#include "D3D12RenderContext.h"

#include "Prism-Core/Render/RenderDevice.h"
#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
D3D12RenderContext::D3D12RenderContext()
{
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)));
}

void D3D12RenderContext::Draw(DrawCommandDesc desc)
{
	m_commandList->DrawInstanced(desc.numVertices, desc.numInstances, desc.startVertexLocation, 0);
}

void D3D12RenderContext::DrawIndexed(DrawIndexedCommandDesc desc)
{
	m_commandList->DrawIndexedInstanced(desc.numIndices, desc.numInstances, desc.startIndexLocation, desc.baseVertexLocation, 0);
}

void D3D12RenderContext::SetPSO(GraphicsPipelineState* pso)
{
	m_commandList->SetPipelineState(static_cast<D3D12GraphicsPipelineState*>(pso)->GetD3D12PipelineState());
}

void D3D12RenderContext::SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv)
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

void D3D12RenderContext::SetViewports(std::vector<Viewport> viewports)
{
	std::vector<D3D12_VIEWPORT> d3d12Viewports;
	d3d12Viewports.reserve(viewports.size());
	for (Viewport viewport : viewports)
		d3d12Viewports.push_back(GetD3D12Viewport(viewport));

	m_commandList->RSSetViewports((UINT)d3d12Viewports.size(), d3d12Viewports.data());
}

void D3D12RenderContext::SetScissors(std::vector<Scissor> scissors)
{
	std::vector<D3D12_RECT> d3d12Rects;
	d3d12Rects.reserve(scissors.size());
	for (Scissor scissor : scissors)
		d3d12Rects.push_back(GetD3D12Rect(scissor));

	m_commandList->RSSetScissorRects((UINT)d3d12Rects.size(), d3d12Rects.data());
}

void D3D12RenderContext::ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor)
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

void D3D12RenderContext::ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue)
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

void D3D12RenderContext::Transition(StateTransitionDesc desc)
{
	CD3DX12_RESOURCE_BARRIER barrier = GetD3D12ResourceBarrier(desc);
	m_commandList->ResourceBarrier(1, &barrier);
}
}
