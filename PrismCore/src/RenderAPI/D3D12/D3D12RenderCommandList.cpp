#include "pcpch.h"
#include "D3D12RenderCommandList.h"

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "Prism/Base/Application.h"
#include "RenderAPI/D3D12/D3D12Buffer.h"
#include "RenderAPI/D3D12/D3D12BufferView.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"
#include "Prism/Render/RenderTypes.h"
#include "RenderAPI/D3D12/D3D12RootSignature.h"

namespace Prism::Render::D3D12
{
void BuildDynamicResourceDescriptor(RenderResourceView* view, CD3DX12_CPU_DESCRIPTOR_HANDLE d3d12DescriptorHandle)
{
	if (view->GetResourceType() == ResourceType::Buffer)
	{
		view->GetSubType<D3D12BufferView>()->BuildDynamicDescriptor(d3d12DescriptorHandle);
		return;
	}
	else if (view->GetResourceType() == ResourceType::Texture)
	{
		view->GetSubType<D3D12TextureView>()->BuildDynamicDescriptor(d3d12DescriptorHandle);
		return;
	}

	PE_ASSERT_NO_ENTRY();
}

#pragma warning(suppress: 4172)
const DescriptorHeapAllocation& GetDescriptorFromView(RenderResourceView* view)
{
	if (view->GetResourceType() == ResourceType::Buffer)
		return view->GetSubType<D3D12BufferView>()->GetDescriptor();
	else if (view->GetResourceType() == ResourceType::Texture)
		return view->GetSubType<D3D12TextureView>()->GetDescriptor();

	PE_ASSERT_NO_ENTRY();
	return {};
}

D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorHeapTypeFromView(RenderResourceView* view)
{
	if (view->GetResourceType() == ResourceType::Buffer)
		return view->GetSubType<D3D12BufferView>()->GetDescriptorHeapType();
	else if (view->GetResourceType() == ResourceType::Texture)
		return view->GetSubType<D3D12TextureView>()->GetDescriptorHeapType();

	PE_ASSERT_NO_ENTRY();
	return {};
}

D3D12RenderCommandList::D3D12RenderCommandList()
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
	m_commandList->SetDescriptorHeaps((UINT)heaps.size(), heaps.data());
	m_commandList->SetGraphicsRootSignature(D3D12RenderDevice::Get().GetGlobalRootSignature(PipelineStateType::Graphics)->GetD3D12RootSignature());
	m_commandList->SetComputeRootSignature(D3D12RenderDevice::Get().GetGlobalRootSignature(PipelineStateType::Compute)->GetD3D12RootSignature());
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

void D3D12RenderCommandList::SetPSO(const GraphicsPipelineStateDesc& desc)
{
	m_currentGraphicsPSO = desc;

	auto* pso = m_pipelineStateCache.GetOrCreatePipelineState(desc);

	m_commandList->SetPipelineState(pso);
	m_commandList->IASetPrimitiveTopology(GetD3D12PrimitiveTopology(desc.primitiveTopologyType));
}

void D3D12RenderCommandList::SetPSO(const ComputePipelineStateDesc& desc)
{
	m_currentComputePSO = desc;

	auto* pso = m_pipelineStateCache.GetOrCreatePipelineState(desc);

	m_commandList->SetPipelineState(pso);
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

void D3D12RenderCommandList::SetTexture(TextureView* textureView, const std::wstring& paramName)
{
	PE_ASSERT(textureView);
	PE_ASSERT(!paramName.empty(), "Param name cannot be empty");

	// We need to save the previous resource descriptor because GPU might still be using it
	auto it = m_rootResources.find(paramName);
	if (it != m_rootResources.end() && it->second)
		m_overriddenRootResources.push_back(it->second);

	m_rootResources[paramName] = textureView;
}

void D3D12RenderCommandList::SetBuffer(BufferView* bufferView, const std::wstring& paramName)
{
	PE_ASSERT(bufferView);
	PE_ASSERT(!paramName.empty(), "Param name cannot be empty");

	// We need to save the previous resource descriptor because GPU might still be using it
	auto it = m_rootResources.find(paramName);
	if (it != m_rootResources.end() && it->second)
		m_overriddenRootResources.push_back(it->second);

	m_rootResources[paramName] = bufferView;
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

	CopyBufferRegion(texture, {0, 0, 0}, subresourceIndex, uploadBuffer, 0);

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

void D3D12RenderCommandList::CopyBufferRegion(Texture* dest, glm::int3 destLoc, int32_t subresourceIndex,
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

void D3D12RenderCommandList::CopyTextureRegion(Buffer* dest, int64_t destOffset,
											   Texture* src, int32_t srcSubresourceIndex, Box srcBox)
{
	PE_ASSERT(dest && src);
	PE_ASSERT(dest->GetResourceType() == ResourceType::Buffer);
	PE_ASSERT(src->GetResourceType() == ResourceType::Texture);

	auto* d3d12Texture = static_cast<D3D12Texture*>(src)->GetD3D12Resource();
	auto d3d12TextureDesc = d3d12Texture->GetDesc();

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferLayout;
	UINT64 srcRowPitch;
	D3D12RenderDevice::Get().GetD3D12Device()->GetCopyableFootprints(&d3d12TextureDesc, srcSubresourceIndex, 1, (UINT64)destOffset,
																	 &bufferLayout,
																	 nullptr,
																	 &srcRowPitch,
																	 nullptr);

	CD3DX12_TEXTURE_COPY_LOCATION destTexLoc(static_cast<D3D12Buffer*>(dest)->GetD3D12Resource(), bufferLayout);
	CD3DX12_TEXTURE_COPY_LOCATION srcTexLoc(static_cast<D3D12Texture*>(src)->GetD3D12Resource(), srcSubresourceIndex);

	auto d3d12Box = GetD3D12Box(srcBox, src);
	m_commandList->CopyTextureRegion(&destTexLoc, 0, 0, 0, &srcTexLoc, &d3d12Box);
}

void D3D12RenderCommandList::CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex,
											   Texture* src, int32_t srcSubresourceIndex, Box srcBox)
{
	PE_ASSERT(dest && src);
	PE_ASSERT(dest->GetResourceType() == ResourceType::Texture);
	PE_ASSERT(src->GetResourceType() == ResourceType::Texture);

	CD3DX12_TEXTURE_COPY_LOCATION destTexLoc(static_cast<D3D12Texture*>(dest)->GetD3D12Resource(), destSubresourceIndex);
	CD3DX12_TEXTURE_COPY_LOCATION srcTexLoc(static_cast<D3D12Texture*>(src)->GetD3D12Resource(), srcSubresourceIndex);

	auto d3d12Box = GetD3D12Box(srcBox, src);
	m_commandList->CopyTextureRegion(&destTexLoc, (UINT)destLoc.x, (UINT)destLoc.y, (UINT)destLoc.z, &srcTexLoc, &d3d12Box);
}

void D3D12RenderCommandList::RenderImGui()
{
	ImGui::Render();

	const auto& windows = Core::Application::Get().GetWindows();
	if (!windows.empty())
	{
		PE_ASSERT(windows.front().IsValid());

		// TODO: We don't have a concept of main window so we just get the first one for now
		auto mainWindow = windows.front();
		auto* currentBackBuffer = static_cast<D3D12TextureView*>(mainWindow->GetSwapchain()->GetCurrentBackBufferRTV());

		auto descriptor = currentBackBuffer->GetDescriptor().GetCPUHandle();
		m_commandList->OMSetRenderTargets(1, &descriptor, true, nullptr);
	}

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
}

void D3D12RenderCommandList::Close()
{
	RenderCommandList::Close();

	PE_ASSERT_HR(m_commandList->Close());
}

void D3D12RenderCommandList::SetupDrawOrDispatch(PipelineStateType type)
{
	PE_ASSERT(
		type == PipelineStateType::Graphics && m_currentGraphicsPSO.IsValid() ||
		type == PipelineStateType::Compute && m_currentComputePSO.IsValid());

	auto setupDescriptors = [this]<typename T>(const T& pso) requires std::is_same_v<T, GraphicsPipelineStateDesc> || std::is_same_v<T, ComputePipelineStateDesc>
	{
		using ShaderArray = std::conditional_t<std::is_same_v<T, GraphicsPipelineStateDesc>,
		   std::array<const ShaderDesc*, 2>,
		   std::array<const ShaderDesc*, 1>
		>;

		ShaderArray shaders;
		if constexpr (std::is_same_v<T, GraphicsPipelineStateDesc>)
		{
			shaders[0] = &pso.vs;
			shaders[1] = &pso.ps;
		}
		else
		{
			shaders[0] = &pso.cs;
		}

		std::vector<D3D12_ROOT_PARAMETER> rootParams;

		for (const ShaderDesc* shader : shaders)
		{
			int32_t rootParamIndex = -1;
			if constexpr (std::is_same_v<T, GraphicsPipelineStateDesc>)
				rootParamIndex = D3D12RenderDevice::Get().GetGlobalRootSignature(PipelineStateType::Graphics)->GetRootParamIndexForShader(shader->shaderType);
			else if constexpr (std::is_same_v<T, ComputePipelineStateDesc>)
				rootParamIndex = D3D12RenderDevice::Get().GetGlobalRootSignature(PipelineStateType::Compute)->GetRootParamIndexForShader(shader->shaderType);

			ComPtr<ID3D12ShaderReflection> reflection = D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetOrCreateShader(*shader).reflection;

			D3D12_SHADER_DESC shaderDesc;
			PE_ASSERT_HR(reflection->GetDesc(&shaderDesc));

			PE_ASSERT(shaderDesc.ConstantBuffers <= 1, "You are only supposed to use 1 constant buffer to get indices to a descriptor heap");

			if (shaderDesc.ConstantBuffers == 0)
				continue;

			ID3D12ShaderReflectionConstantBuffer* cbReflection = reflection->GetConstantBufferByIndex(0);
			PE_ASSERT(cbReflection);

			D3D12_SHADER_BUFFER_DESC cbDesc;
			PE_ASSERT_HR(cbReflection->GetDesc(&cbDesc));

			for (int32_t i = 0; i < (int32_t)cbDesc.Variables; ++i)
			{
				ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(i);
				PE_ASSERT(varReflection);
				D3D12_SHADER_VARIABLE_DESC varDesc;
				PE_ASSERT_HR(varReflection->GetDesc(&varDesc));

				ID3D12ShaderReflectionType* varTypeReflection = varReflection->GetType();
				PE_ASSERT(varTypeReflection);
				D3D12_SHADER_TYPE_DESC varTypeDesc;
				PE_ASSERT_HR(varTypeReflection->GetDesc(&varTypeDesc));

				PE_ASSERT(varTypeDesc.Type == D3D_SVT_INT, "Indices must by of type int");

				UINT data = 0;

				auto rootResIt = m_rootResources.find(StringToWString(varDesc.Name));
				if (rootResIt != m_rootResources.end())
				{
					PE_ASSERT(rootResIt->second);

					if (rootResIt->second->IsViewOfDynamicResource())
					{
						auto allocation = D3D12RenderDevice::Get().AllocateDescriptors(GetDescriptorHeapTypeFromView(rootResIt->second));
						BuildDynamicResourceDescriptor(rootResIt->second, allocation.GetCPUHandle());
						int32_t handleIndex = allocation.GetHandleIndexInHeap();

						data = std::bit_cast<UINT>(handleIndex);

						m_dynamicDescriptors.push_back(std::move(allocation));
					}
					else
					{
						int32_t handleIndex = GetDescriptorFromView(rootResIt->second).GetHandleIndexInHeap();

						data = std::bit_cast<UINT>(handleIndex);
					}
				}
				else
				{
					//PE_RENDER_LOG(Info, "{} resource wasn't set in shader {} with entry name \"{}\", index will be set to -1",
					//	varDesc.Name, shader->GetCreateInfo().filepath, shader->GetCreateInfo().entryName);

					// We set the handle to -1 to indicate that nothing is bound to it
					int32_t handleIndex = -1;
					data = std::bit_cast<UINT>(handleIndex);
				}

				if constexpr (std::is_same_v<T, GraphicsPipelineStateDesc>)
					m_commandList->SetGraphicsRoot32BitConstant(rootParamIndex, data, i);
				else if constexpr (std::is_same_v<T, ComputePipelineStateDesc>)
					m_commandList->SetComputeRoot32BitConstant(rootParamIndex, data, i);
			}
		}
	};

	if (type == PipelineStateType::Graphics)
		setupDescriptors(m_currentGraphicsPSO);
	else if (type == PipelineStateType::Compute)
		setupDescriptors(m_currentComputePSO);
}
}
