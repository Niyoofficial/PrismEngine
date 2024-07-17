#include "pcpch.h"
#include "D3D12RenderContext.h"

#include "Prism-Core/Render/RenderDevice.h"
#include "RenderAPI/D3D12/D3D12Buffer.h"
#include "RenderAPI/D3D12/D3D12BufferView.h"
#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
void BuildResourceView(RenderResourceView* view)
{
	if (view->GetResourceType() == ResourceType::Buffer)
	{
		view->GetSubType<D3D12BufferView>()->BuildView();
		return;
	}
	else if (view->GetResourceType() == ResourceType::Texture)
	{
		view->GetSubType<D3D12TextureView>()->BuildView();
		return;
	}

	PE_ASSERT_NO_ENTRY();
}

#pragma warning(suppress: 4172)
const CPUDescriptorHeapAllocation& GetDescriptorFromView(RenderResourceView* view)
{
	if (view->GetResourceType() == ResourceType::Buffer)
		return view->GetSubType<D3D12BufferView>()->GetDescriptor();
	else if (view->GetResourceType() == ResourceType::Texture)
		return view->GetSubType<D3D12TextureView>()->GetDescriptor();

	PE_ASSERT_NO_ENTRY();
	return {};
}

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
	PE_ASSERT_HR(m_commandList->SetName(L"Render Context Cmd List"));

	auto heaps = D3D12RenderDevice::Get().GetGPUDescriptorHeaps();
	m_commandList->SetDescriptorHeaps((UINT)heaps.size(), heaps.data());
}

void D3D12RenderContext::Draw(DrawCommandDesc desc)
{
	PrepareDraw();

	m_commandList->DrawInstanced(desc.numVertices, desc.numInstances, desc.startVertexLocation, 0);
}

void D3D12RenderContext::DrawIndexed(DrawIndexedCommandDesc desc)
{
	PrepareDraw();

	m_commandList->DrawIndexedInstanced(desc.numIndices, desc.numInstances, desc.startIndexLocation, desc.baseVertexLocation, 0);
}

void D3D12RenderContext::SetPSO(GraphicsPipelineState* pso)
{
	m_currentPSO = static_cast<D3D12GraphicsPipelineState*>(pso);
	m_currentRootSig = D3D12RenderDevice::Get().GetRootSignatureCache().GetOrCreateRootSignature(pso->GetDesc());

	m_commandList->SetPipelineState(m_currentPSO->GetD3D12PipelineState());
	m_commandList->SetGraphicsRootSignature(m_currentRootSig->GetD3D12RootSignature());
	m_commandList->IASetPrimitiveTopology(GetD3D12PrimitiveTopology(pso->GetDesc().primitiveTopologyType));
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

void D3D12RenderContext::SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes)
{
	D3D12_VERTEX_BUFFER_VIEW view = {
		.BufferLocation = static_cast<D3D12Buffer*>(buffer)->GetD3D12Resource()->GetGPUVirtualAddress(),
		.SizeInBytes = (UINT)buffer->GetBufferDesc().size,
		.StrideInBytes = (UINT)vertexSizeInBytes
	};
	m_commandList->IASetVertexBuffers(0, 1, &view);
}

void D3D12RenderContext::SetIndexBuffer(Buffer* buffer, IndexBufferFormat format)
{
	D3D12_INDEX_BUFFER_VIEW view = {
		.BufferLocation = static_cast<D3D12Buffer*>(buffer)->GetD3D12Resource()->GetGPUVirtualAddress(),
		.SizeInBytes = (UINT)buffer->GetBufferDesc().size,
		.Format = GetIndexBufferDXGIFormat(format)
	};
	m_commandList->IASetIndexBuffer(&view);
}

void D3D12RenderContext::SetTexture(TextureView* textureView, const std::wstring& paramName)
{
	SetResource(textureView, paramName);
}

void D3D12RenderContext::SetCBuffer(BufferView* bufferView, const std::wstring& paramName)
{
	SetResource(bufferView, paramName);
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
	uint8_t stencilValue = 0;
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

void D3D12RenderContext::UpdateBuffer(Buffer* buffer, BufferData data)
{
	PE_ASSERT(buffer);
	PE_ASSERT(buffer->GetBufferDesc().usage == ResourceUsage::Default,
			  "Buffer must have Default usage to be updated through render context");

	auto uploadBufferDesc = buffer->GetBufferDesc();
	uploadBufferDesc.bufferName = L"UploadBuffer";
	uploadBufferDesc.usage = ResourceUsage::Dynamic;
	uploadBufferDesc.cpuAccess = CPUAccess::Write;
	auto uploadBuffer = Buffer::Create(uploadBufferDesc, data, ResourceStateFlags::GenericRead);

	CopyBufferRegion(buffer, 0, uploadBuffer, 0, (int32_t)buffer->GetBufferDesc().size);

	SafeReleaseResource(std::move(uploadBuffer));
}

void D3D12RenderContext::CopyBufferRegion(Buffer* dest, int32_t destOffset, Buffer* src, int32_t srcOffset, int32_t numBytes)
{
	PE_ASSERT(dest && src);
	PE_ASSERT(dest->GetResourceType() == ResourceType::Buffer);
	PE_ASSERT(src->GetResourceType() == ResourceType::Buffer);

	auto* d3d12Dest = static_cast<D3D12Buffer*>(dest);
	auto* d3d12Src = static_cast<D3D12Buffer*>(src);
	m_commandList->CopyBufferRegion(d3d12Dest->GetD3D12Resource(), d3d12Dest->GetDefaultOffset() + destOffset,
									d3d12Src->GetD3D12Resource(), d3d12Src->GetDefaultOffset() + srcOffset, numBytes);
}

void D3D12RenderContext::CloseContext()
{
	PE_ASSERT_HR(m_commandList->Close());
}

void D3D12RenderContext::PrepareDraw()
{
	PE_ASSERT(m_currentRootSig);

	for (auto [name, index] : m_currentRootSig->GetRootParamsIndexMap())
	{
		if (auto* view = GetResourceView(name))
		{
			BuildResourceView(view);

			auto gpuDescriptorHandle = D3D12RenderDevice::Get().CopyToGPUHeap(GetDescriptorFromView(view));
			m_commandList->SetGraphicsRootDescriptorTable(index, gpuDescriptorHandle.GetGPUHandle());
			// Save the descriptor allocation because destructor will free the descriptor handle otherwise
			m_gpuDescriptors.push_back(std::move(gpuDescriptorHandle));
		}
	}
}

void D3D12RenderContext::SetResource(RenderResourceView* view, const std::wstring& paramName)
{
	PE_ASSERT(view);
	PE_ASSERT(!paramName.empty(), "Param name cannot be empty");
	m_rootResources[paramName] = view;
}

RenderResourceView* D3D12RenderContext::GetResourceView(const std::wstring& name)
{
	auto it = m_rootResources.find(name);
	if (it == m_rootResources.end())
		return nullptr;
	return it->second;
}
}
