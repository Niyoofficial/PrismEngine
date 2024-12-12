#include "pcpch.h"
#include "D3D12RenderCommandList.h"

#include "RenderAPI/D3D12/D3D12Buffer.h"
#include "RenderAPI/D3D12/D3D12BufferView.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"
#include "Prism-Core/Render/RenderTypes.h"
#include "RenderAPI/D3D12/D3D12PipelineState.h"

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

D3D12RenderCommandList::D3D12RenderCommandList(uint64_t fenceValue)
	: RenderCommandList(fenceValue)
{
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)));
	PE_ASSERT_HR(m_commandList->SetName(L"Render Cmd List"));

	auto heaps = D3D12RenderDevice::Get().GetGPUDescriptorHeaps();
	m_commandList->SetDescriptorHeaps(heaps.size(), heaps.data());
}

void D3D12RenderCommandList::Draw(DrawCommandDesc desc)
{
	SetupDrawOrDispatch(PipelineStateType::Graphics);

	m_commandList->DrawInstanced((UINT)desc.numVertices, (UINT)desc.numInstances, (UINT)desc.startVertexLocation, 0);
}

void D3D12RenderCommandList::DrawIndexed(DrawIndexedCommandDesc desc)
{
	SetupDrawOrDispatch(PipelineStateType::Graphics);

	m_commandList->DrawIndexedInstanced((UINT)desc.numIndices, (UINT)desc.numInstances, (UINT)desc.startIndexLocation, (UINT)desc.baseVertexLocation, 0);
}

void D3D12RenderCommandList::Dispatch(int32_t threadGroupCountX, int32_t threadGroupCountY, int32_t threadGroupCountZ)
{
	SetupDrawOrDispatch(PipelineStateType::Compute);

	m_commandList->Dispatch((UINT)threadGroupCountX, (UINT)threadGroupCountY, (UINT)threadGroupCountZ);
}

void D3D12RenderCommandList::SetPSO(GraphicsPipelineState* pso)
{
	auto* graphicsPSO = static_cast<D3D12GraphicsPipelineState*>(pso);
	m_currentRootSig = D3D12RenderDevice::Get().GetRootSignatureCache().GetOrCreateRootSignature(pso->GetDesc());

	m_commandList->SetPipelineState(graphicsPSO->GetD3D12PipelineState());
	m_commandList->SetGraphicsRootSignature(m_currentRootSig->GetD3D12RootSignature());
	m_commandList->IASetPrimitiveTopology(GetD3D12PrimitiveTopology(pso->GetDesc().primitiveTopologyType));
}

void D3D12RenderCommandList::SetPSO(ComputePipelineState* pso)
{
	auto* computePSO = static_cast<D3D12ComputePipelineState*>(pso);
	m_currentRootSig = D3D12RenderDevice::Get().GetRootSignatureCache().GetOrCreateRootSignature(pso->GetDesc());

	m_commandList->SetPipelineState(computePSO->GetD3D12PipelineState());
	m_commandList->SetComputeRootSignature(m_currentRootSig->GetD3D12RootSignature());
}

void D3D12RenderCommandList::SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv)
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

void D3D12RenderCommandList::SetViewports(std::vector<Viewport> viewports)
{
	std::vector<D3D12_VIEWPORT> d3d12Viewports;
	d3d12Viewports.reserve(viewports.size());
	for (Viewport viewport : viewports)
		d3d12Viewports.push_back(GetD3D12Viewport(viewport));

	m_commandList->RSSetViewports((UINT)d3d12Viewports.size(), d3d12Viewports.data());
}

void D3D12RenderCommandList::SetScissors(std::vector<Scissor> scissors)
{
	std::vector<D3D12_RECT> d3d12Rects;
	d3d12Rects.reserve(scissors.size());
	for (Scissor scissor : scissors)
		d3d12Rects.push_back(GetD3D12Rect(scissor));

	m_commandList->RSSetScissorRects((UINT)d3d12Rects.size(), d3d12Rects.data());
}

void D3D12RenderCommandList::SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes)
{
	D3D12_VERTEX_BUFFER_VIEW view = {
		.BufferLocation = static_cast<D3D12Buffer*>(buffer)->GetD3D12Resource()->GetGPUVirtualAddress(),
		.SizeInBytes = (UINT)buffer->GetBufferDesc().size,
		.StrideInBytes = (UINT)vertexSizeInBytes
	};
	m_commandList->IASetVertexBuffers(0, 1, &view);
}

void D3D12RenderCommandList::SetIndexBuffer(Buffer* buffer, IndexBufferFormat format)
{
	D3D12_INDEX_BUFFER_VIEW view = {
		.BufferLocation = static_cast<D3D12Buffer*>(buffer)->GetD3D12Resource()->GetGPUVirtualAddress(),
		.SizeInBytes = (UINT)buffer->GetBufferDesc().size,
		.Format = GetIndexBufferDXGIFormat(format)
	};
	m_commandList->IASetIndexBuffer(&view);
}

void D3D12RenderCommandList::SetTexture(TextureView* textureView, const std::wstring& paramName, int32_t index)
{
	PE_ASSERT(textureView);
	PE_ASSERT(!paramName.empty(), "Param name cannot be empty");
	m_rootResources[paramName][index] = textureView;
}

void D3D12RenderCommandList::SetBuffer(BufferView* bufferView, const std::wstring& paramName)
{
	PE_ASSERT(bufferView);
	PE_ASSERT(!paramName.empty(), "Param name cannot be empty");
	m_rootResources[paramName][0] = bufferView;
}

void D3D12RenderCommandList::ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor)
{
	float rtClearColor[4] = { 0.f };
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
		PE_ASSERT(texDesc.optimizedClearValue.has_value());
		glm::float4 color = std::get<RenderTargetClearValue>(texDesc.optimizedClearValue.value()).color;
		rtClearColor[0] = color.r;
		rtClearColor[1] = color.g;
		rtClearColor[2] = color.b;
		rtClearColor[3] = color.a;
	}
	m_commandList->ClearRenderTargetView(static_cast<D3D12TextureView*>(rtv)->GetDescriptor().GetCPUHandle(),
		rtClearColor, 0, nullptr);
}

void D3D12RenderCommandList::ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue)
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
		PE_ASSERT(texDesc.optimizedClearValue.has_value());
		auto [depth, stencil] = std::get<DepthStencilClearValue>(texDesc.optimizedClearValue.value()).depthStencil;
		depthValue = depth;
		stencilValue = stencil;
	}
	m_commandList->ClearDepthStencilView(static_cast<D3D12TextureView*>(dsv)->GetDescriptor().GetCPUHandle(),
		GetD3D12ClearFlags(flags), depthValue, stencilValue, 0, nullptr);
}

void D3D12RenderCommandList::Barrier(BufferBarrier barrier)
{
	auto d3d12Barrier = GetD3D12BufferBarrier(barrier);
	CD3DX12_BARRIER_GROUP barrierGroups[] = {{1, &d3d12Barrier}};
	m_commandList->Barrier(1, barrierGroups);
}

void D3D12RenderCommandList::Barrier(TextureBarrier barrier)
{
	auto d3d12Barrier = GetD3D12TextureBarrier(barrier);
	CD3DX12_BARRIER_GROUP barrierGroups[] = {{1, &d3d12Barrier}};
	m_commandList->Barrier(1, barrierGroups);
}

void D3D12RenderCommandList::UpdateBuffer(Buffer* buffer, RawData data)
{
	PE_ASSERT(buffer);
	PE_ASSERT(buffer->GetBufferDesc().usage == ResourceUsage::Default ||
			  (buffer->GetBufferDesc().usage == ResourceUsage::Staging && buffer->GetBufferDesc().cpuAccess.HasAllFlags(CPUAccess::Write)),
			  "Buffer must have either Default usage or Staging usage with CPUAccess::Write to be able to be updated through render context");

	auto uploadBufferDesc = buffer->GetBufferDesc();
	uploadBufferDesc.bufferName = L"UploadBuffer";
	uploadBufferDesc.usage = ResourceUsage::Staging;
	uploadBufferDesc.cpuAccess = CPUAccess::Write;
	auto uploadBuffer = Buffer::Create(uploadBufferDesc, data);

	CopyBufferRegion(buffer, 0, uploadBuffer, 0, (int32_t)buffer->GetBufferDesc().size);

	SafeReleaseResource(std::move(uploadBuffer));
}

void D3D12RenderCommandList::UpdateTexture(Texture* texture, RawData data, int32_t subresourceIndex)
{
	PE_ASSERT(texture);
	PE_ASSERT(texture->GetTextureDesc().usage == ResourceUsage::Default,
		"Texture must have Default usage to be updated through render context");

	int64_t intermediateSize = (int64_t)GetRequiredIntermediateSize(static_cast<D3D12Texture*>(texture)->GetD3D12Resource(), subresourceIndex, 1);
	BufferDesc uploadBufferDesc = {
		.bufferName = L"UploadBuffer",
		.size = intermediateSize,
		.usage = ResourceUsage::Staging,
		.cpuAccess = CPUAccess::Write
	};
	auto uploadBuffer = Buffer::Create(uploadBufferDesc, data);

	CopyTextureRegion(texture, {0, 0, 0}, subresourceIndex, uploadBuffer, 0);

	SafeReleaseResource(std::move(uploadBuffer));
}

void D3D12RenderCommandList::CopyBufferRegion(Buffer* dest, int64_t destOffset, Buffer* src, int64_t srcOffset, int64_t numBytes)
{
	PE_ASSERT(dest && src);
	PE_ASSERT(dest->GetResourceType() == ResourceType::Buffer);
	PE_ASSERT(src->GetResourceType() == ResourceType::Buffer);

	auto* d3d12Dest = static_cast<D3D12Buffer*>(dest);
	auto* d3d12Src = static_cast<D3D12Buffer*>(src);
	m_commandList->CopyBufferRegion(d3d12Dest->GetD3D12Resource(), d3d12Dest->GetDefaultOffset() + destOffset,
		d3d12Src->GetD3D12Resource(), d3d12Src->GetDefaultOffset() + srcOffset, numBytes);
}

void D3D12RenderCommandList::CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t subresourceIndex,
											   Buffer* src, int64_t srcOffset)
{
	PE_ASSERT(dest && src);
	PE_ASSERT(dest->GetResourceType() == ResourceType::Texture);
	PE_ASSERT(src->GetResourceType() == ResourceType::Buffer);

	auto destDesc = static_cast<D3D12Texture*>(dest)->GetD3D12Resource()->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	UINT numRows = 0;
	UINT64 rowSizeInBytes = 0;
	UINT64 totalBytes = 0;
	D3D12RenderDevice::Get().GetD3D12Device()->GetCopyableFootprints(&destDesc, subresourceIndex, 1, srcOffset,
		&layout, &numRows,
		&rowSizeInBytes, &totalBytes);

	CD3DX12_TEXTURE_COPY_LOCATION destTexLoc(static_cast<D3D12Texture*>(dest)->GetD3D12Resource(), subresourceIndex);
	CD3DX12_TEXTURE_COPY_LOCATION srcTexLoc(static_cast<D3D12Buffer*>(src)->GetD3D12Resource(), layout);

	m_commandList->CopyTextureRegion(&destTexLoc, (UINT)destLoc.x, (UINT)destLoc.y, (UINT)destLoc.z, &srcTexLoc, nullptr);
}

void D3D12RenderCommandList::CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex,
											   Texture* src, int32_t srcSubresourceIndex, glm::int3 srcLoc, glm::int3 srcSize)
{
	PE_ASSERT(dest && src);
	PE_ASSERT(dest->GetResourceType() == ResourceType::Texture);
	PE_ASSERT(src->GetResourceType() == ResourceType::Texture);

	CD3DX12_TEXTURE_COPY_LOCATION destTexLoc(static_cast<D3D12Texture*>(dest)->GetD3D12Resource(), destSubresourceIndex);
	CD3DX12_TEXTURE_COPY_LOCATION srcTexLoc(static_cast<D3D12Texture*>(src)->GetD3D12Resource(), srcSubresourceIndex);

	D3D12_BOX box = {
		.left = (UINT)srcLoc.x,
		.top = (UINT)srcLoc.y,
		.front = (UINT)srcLoc.z,
		.right = (UINT)(srcSize.x == -1 ? src->GetTextureDesc().GetWidth() - srcLoc.x : srcLoc.x + srcSize.x),
		.bottom = (UINT)(srcSize.y == -1 ? src->GetTextureDesc().GetHeight() - srcLoc.y : (UINT)srcLoc.y + srcSize.y),
		.back = (UINT)(srcSize.z == -1 ? src->GetTextureDesc().GetDepth() - srcLoc.z : (UINT)srcLoc.z + srcSize.z)
	};
	m_commandList->CopyTextureRegion(&destTexLoc, (UINT)destLoc.x, (UINT)destLoc.y, (UINT)destLoc.z, &srcTexLoc, &box);
}

void D3D12RenderCommandList::Close()
{
	RenderCommandList::Close();

	PE_ASSERT_HR(m_commandList->Close());
}

void D3D12RenderCommandList::SetupDrawOrDispatch(PipelineStateType type)
{
	PE_ASSERT(m_currentRootSig);

	for (const auto& [name, rootIndex] : m_currentRootSig->GetRootParamsIndexMap())
	{
		auto nullDescriptor = GetNullDescriptorForRootParam(m_currentRootSig, rootIndex);

		auto rootResIt = m_rootResources.find(name);
		if (rootResIt != m_rootResources.end())
		{
			auto& rootParamViews = rootResIt->second;

			std::vector<const CPUDescriptorHeapAllocation*> descriptorAllocations;
			int32_t desctriptorArrayCount = (int32_t)m_currentRootSig->GetReflectionForRootParam(rootIndex).BindCount;
			for (int32_t i = 0; i < desctriptorArrayCount;)
			{
				auto viewIt = rootParamViews.find(i);
				if (viewIt != rootParamViews.end())
				{
					auto& resView = viewIt->second;
					BuildResourceView(resView);
					const auto& descriptorAllocation = GetDescriptorFromView(resView);
					descriptorAllocations.push_back(&descriptorAllocation);

					i += descriptorAllocation.GetNumHandles();
				}
				else
				{
					descriptorAllocations.push_back(&nullDescriptor);
					++i;
				}
			}
			SetRootParam(rootIndex, descriptorAllocations, type);
		}
		else
		{
			SetRootParam(rootIndex, nullDescriptor, type);
		}
	}
}

CPUDescriptorHeapAllocation D3D12RenderCommandList::GetNullDescriptorForRootParam(D3D12RootSignature* rootSignature, int32_t rootParamIndex)
{
	auto resDesc = rootSignature->GetReflectionForRootParam(rootParamIndex);

	// TODO: Add sampler support
	auto nullDescriptor = D3D12RenderDevice::Get().AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	switch (resDesc.Type)
	{
	case D3D_SIT_CBUFFER:
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {
			.BufferLocation = 0,
			.SizeInBytes = 0
		};
		D3D12RenderDevice::Get().GetD3D12Device()->CreateConstantBufferView(&viewDesc, nullDescriptor.GetCPUHandle());
	}
	break;
	case D3D_SIT_TEXTURE:
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = GetD3D12SRVDimension(resDesc.Dimension),
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		};

		D3D12RenderDevice::Get().GetD3D12Device()->CreateShaderResourceView(nullptr, &viewDesc, nullDescriptor.GetCPUHandle());
	}
	break;
	case D3D_SIT_UAV_RWTYPED:
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = GetD3D12UAVDimension(resDesc.Dimension)
		};

		D3D12RenderDevice::Get().GetD3D12Device()->CreateUnorderedAccessView(
			nullptr, nullptr, &viewDesc, nullDescriptor.GetCPUHandle());
	}
	break;
	case D3D_SIT_UAV_RWSTRUCTURED:
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {
			.Format = DXGI_FORMAT_R32_FLOAT,
			.ViewDimension = GetD3D12UAVDimension(resDesc.Dimension)
		};

		D3D12RenderDevice::Get().GetD3D12Device()->CreateUnorderedAccessView(
			nullptr, nullptr, &viewDesc, nullDescriptor.GetCPUHandle());
	}
	break;
	case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {
			.Format = DXGI_FORMAT_R32_FLOAT,
			.ViewDimension = GetD3D12UAVDimension(resDesc.Dimension)
		};

		D3D12RenderDevice::Get().GetD3D12Device()->CreateUnorderedAccessView(
			nullptr, nullptr, &viewDesc, nullDescriptor.GetCPUHandle());
	}
	break;
	default:
		PE_ASSERT_NO_ENTRY();
	}

	return nullDescriptor;
}

void D3D12RenderCommandList::SetRootParam(int32_t rootIndex, const CPUDescriptorHeapAllocation& cpuAllocation, PipelineStateType type)
{
	const auto* allocPtr = &cpuAllocation;
	SetRootParam(rootIndex, std::span(&allocPtr, 1), type);
}

void D3D12RenderCommandList::SetRootParam(int32_t rootIndex, std::span<const CPUDescriptorHeapAllocation*> cpuAllocations, PipelineStateType type)
{
	auto gpuDescriptorHandle = D3D12RenderDevice::Get().CopyToGPUHeap(cpuAllocations);
	if (type == PipelineStateType::Graphics)
		m_commandList->SetGraphicsRootDescriptorTable(rootIndex, gpuDescriptorHandle.GetGPUHandle());
	else if (type == PipelineStateType::Compute)
		m_commandList->SetComputeRootDescriptorTable(rootIndex, gpuDescriptorHandle.GetGPUHandle());

	// Save the descriptor allocation because destructor will free the descriptor handle otherwise
	m_gpuDescriptors.push_back(std::move(gpuDescriptorHandle));
}
}
