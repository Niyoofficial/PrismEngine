#include "pcpch.h"
#include "RenderContext.h"

#include "Prism/Base/Application.h"
#include "Prism/Render/RenderCommands.h"
#include "Prism/Render/RenderConstants.h"
#include "Prism/Render/RenderResourceCreation.h"
#include "Prism/Render/RenderUtils.h"


namespace Prism::Render
{
void RenderContext::Draw(DrawCommandDesc desc)
{
	m_commandRecorder.AllocateCommand<Commands::DrawRenderCommand>(desc);
}
void RenderContext::DrawIndexed(DrawIndexedCommandDesc desc)
{
	m_commandRecorder.AllocateCommand<Commands::DrawIndexedRenderCommand>(desc);
}

void RenderContext::Dispatch(glm::int3 threadGroupCount)
{
	m_commandRecorder.AllocateCommand<Commands::DispatchRenderCommand>(threadGroupCount);
}

void RenderContext::SetPSO(const GraphicsPipelineStateDesc& pso)
{
	m_commandRecorder.AllocateCommand<Commands::SetGraphicsPSORenderCommand>(pso);
}

void RenderContext::SetPSO(const ComputePipelineStateDesc& pso)
{
	m_commandRecorder.AllocateCommand<Commands::SetComputePSORenderCommand>(pso);
}

void RenderContext::SetStencilRef(uint32_t ref)
{
	m_commandRecorder.AllocateCommand<Commands::CustomRenderCommand>("SetStencilRef",
		[ref](RenderCommandList* cmdList)
		{
			cmdList->SetStencilRef(ref);
		});
}

void RenderContext::SetRenderTarget(const Ref<TextureView>& rtv, const Ref<TextureView>& dsv)
{
	std::vector<Ref<TextureView>> rtvs;
	if (rtv)
		rtvs.push_back(rtv);
	SetRenderTargets(rtvs, dsv);
}

void RenderContext::SetRenderTargets(std::vector<Ref<TextureView>> rtvs, const Ref<TextureView>& dsv)
{
	for (auto& rtv : rtvs)
	{
		PE_ASSERT(rtv);
		SafeReleaseResource(rtv->GetTexture());
	}

	m_commandRecorder.AllocateCommand<Commands::SetRenderTargetsRenderCommand>(rtvs, dsv);
}

void RenderContext::SetViewport(Viewport viewport)
{
	SetViewports({ viewport });
}

void RenderContext::SetViewports(std::vector<Viewport> viewports)
{
	m_commandRecorder.AllocateCommand<Commands::SetViewportsRenderCommand>(viewports);
}

void RenderContext::SetScissor(Scissor scissor)
{
	SetScissors({ scissor });
}

void RenderContext::SetScissors(std::vector<Scissor> scissors)
{
	m_commandRecorder.AllocateCommand<Commands::SetScissorsRenderCommand>(scissors);
}

void RenderContext::SetVertexBuffer(const Ref<Buffer>& buffer, int64_t vertexSizeInBytes)
{
	m_commandRecorder.AllocateCommand<Commands::SetVertexBufferRenderCommand>(buffer, vertexSizeInBytes);
}

void RenderContext::SetIndexBuffer(const Ref<Buffer>& buffer, IndexBufferFormat format)
{
	m_commandRecorder.AllocateCommand<Commands::SetIndexBufferRenderCommand>(buffer, format);
}

void RenderContext::SetTexture(const std::wstring& paramName, const Ref<TextureView>& textureView)
{
	SetTextures(paramName, {textureView});
}

void RenderContext::SetTextures(const std::wstring& paramName, const std::vector<Ref<TextureView>>& textureViews)
{
	for (TextureView* textureView : textureViews)
	{
		if (textureView)
		{
			PE_ASSERT(textureView->GetTexture(), "View to invalid resource");

			SafeReleaseResource(textureView->GetTexture());
		}
	}

	m_commandRecorder.AllocateCommand<Commands::SetTexturesRenderCommand>(textureViews, paramName);
}

void RenderContext::SetBuffer(const std::wstring& paramName, const Ref<BufferView>& bufferView)
{
	SetBuffers(paramName, {bufferView});
}

void RenderContext::SetBuffers(const std::wstring& paramName, const std::vector<Ref<BufferView>>& bufferViews)
{
	std::vector<std::any> dynamicAllocations;
	dynamicAllocations.reserve(bufferViews.size());
	for (BufferView* bufferView : bufferViews)
	{
		auto& allocation = dynamicAllocations.emplace_back();
		if (bufferView)
		{
			PE_ASSERT(bufferView->GetBuffer(), "View to invalid resource");

			SafeReleaseResource(bufferView->GetBuffer());

			if (bufferView->IsViewOfDynamicResource())
				allocation = bufferView->GetBuffer()->GetDynamicAllocation();
		}
	}

	m_commandRecorder.AllocateCommand<Commands::SetBuffersRenderCommand>(bufferViews, dynamicAllocations, paramName);
}

void RenderContext::SetUniformBuffer(const std::wstring& paramName, void* data, int64_t size)
{
	PE_ASSERT(IsAligned(data, Constants::UNIFORM_BUFFER_ALIGNMENT), "Data is not aligned to Constants::UNIFORM_BUFFER_ALIGNMENT");

	if (!m_uniformBuffers.contains(paramName))
	{
		m_uniformBuffers[paramName] = Buffer::Create({
			.bufferName = paramName + L"_UniformBuffer",
			.size = size,
			.bindFlags = BindFlags::UniformBuffer,
			.usage = ResourceUsage::Dynamic,
			.cpuAccess = CPUAccess::Write
		});
	}

	Buffer* uniformBuffer = m_uniformBuffers[paramName];
	PE_ASSERT(uniformBuffer->GetBufferDesc().size == size, "This uniform buffer was already created with a different size");

	void* destData = uniformBuffer->Map(CPUAccess::Write);

	memcpy_s(destData, uniformBuffer->GetBufferDesc().size, data, size);

	uniformBuffer->Unmap();

	SetBuffer(paramName, uniformBuffer->CreateDefaultUniformBufferView());
}

void RenderContext::ClearRenderTargetView(const Ref<TextureView>& rtv, glm::float4* clearColor)
{
	PE_ASSERT(rtv);
	SafeReleaseResource(rtv->GetTexture());

	m_commandRecorder.AllocateCommand<Commands::ClearRenderTargetViewRenderCommand>(rtv, clearColor);
}

void RenderContext::ClearDepthStencilView(const Ref<TextureView>& dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue)
{
	PE_ASSERT(dsv);
	SafeReleaseResource(dsv->GetTexture());

	m_commandRecorder.AllocateCommand<Commands::ClearDepthStencilViewRenderCommand>(dsv, flags, clearValue);
}

void RenderContext::ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::float4 values)
{
	PE_ASSERT(uav);
	SafeReleaseResource(uav->GetTexture());

	m_commandRecorder.AllocateCommand<Commands::CustomRenderCommand>("ClearUnorderedAccessView",
		[values, uav](RenderCommandList* cmdList)
		{
			cmdList->ClearUnorderedAccessView(uav, values);
		});
}

void RenderContext::ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::uint4 values)
{
	PE_ASSERT(uav);
	SafeReleaseResource(uav->GetTexture());

	m_commandRecorder.AllocateCommand<Commands::CustomRenderCommand>("ClearUnorderedAccessView",
		[values, uav](RenderCommandList* cmdList)
		{
			cmdList->ClearUnorderedAccessView(uav, values);
		});
}

void RenderContext::Barrier(BufferBarrier barrier)
{
	m_commandRecorder.AllocateCommand<Commands::BufferBarrierRenderCommand>(barrier);
}

void RenderContext::Barrier(TextureBarrier barrier)
{
	m_commandRecorder.AllocateCommand<Commands::TextureBarrierRenderCommand>(barrier);
}

void RenderContext::UpdateBuffer(const Ref<Buffer>& buffer, RawData data)
{
	m_commandRecorder.AllocateCommand<Commands::UpdateBufferRenderCommand>(buffer, data);
}

void RenderContext::UpdateTexture(const Ref<Texture>& texture, RawData data, int32_t subresourceIndex)
{
	m_commandRecorder.AllocateCommand<Commands::UpdateTextureRenderCommand>(texture, data, subresourceIndex);
}

void RenderContext::CopyBufferRegion(const Ref<Buffer>& dest, int64_t destOffset, const Ref<Buffer>& src, int64_t srcOffset,
									 int64_t numBytes)
{
	m_commandRecorder.AllocateCommand<Commands::CopyBufferRegionRenderCommand>(dest, destOffset, src, srcOffset, numBytes);
}

void RenderContext::CopyBufferRegion(const Ref<Texture>& dest, glm::int3 destLoc, int32_t destSubresourceIndex, const Ref<Buffer>& src,
									 int64_t srcOffset)
{
	m_commandRecorder.AllocateCommand<Commands::CopyBufferRegionToTextureRenderCommand>(
		dest, destLoc, destSubresourceIndex, src, srcOffset);
}

void RenderContext::CopyTextureRegion(const Ref<Buffer>& dest, int64_t destOffset, const Ref<Texture>& src,
									  int32_t srcSubresourceIndex, Box3I srcBox)
{
	m_commandRecorder.AllocateCommand<Commands::CopyTextureRegionToBufferRenderCommand>(
		dest, destOffset,
		src, srcSubresourceIndex, srcBox);
}

void RenderContext::CopyTextureRegion(const Ref<Texture>& dest, glm::int3 destLoc, int32_t destSubresourceIndex,
									  const Ref<Texture>& src, int32_t srcSubresourceIndex, Box3I srcBox)
{
	m_commandRecorder.AllocateCommand<Commands::CopyTextureRegionToTextureRenderCommand>(
		dest, destLoc, destSubresourceIndex,
		src, srcSubresourceIndex, srcBox);
}

Buffer* RenderContext::ReadbackBuffer(const Ref<Buffer>& bufferToReadback)
{
	PE_ASSERT(bufferToReadback);
	PE_ASSERT(bufferToReadback->GetBufferDesc().usage == ResourceUsage::Default);

	auto readbackBuffer = Buffer::Create({
		.bufferName = bufferToReadback->GetBufferDesc().bufferName + L"_ReadbackBuffer",
		.size = RenderDevice::Get().GetTotalSizeInBytes(bufferToReadback->GetBufferDesc()),
		.bindFlags = BindFlags::None,
		.usage = ResourceUsage::Staging,
		.cpuAccess = CPUAccess::Read
		});

	CopyBufferRegion(readbackBuffer, 0, bufferToReadback, 0, bufferToReadback->GetBufferDesc().size);

	auto readbackBufferTemp = readbackBuffer;
	SafeReleaseResource(readbackBufferTemp);

	return readbackBuffer;
}

void RenderContext::ReadbackTexture(const Ref<Texture>& textureToReadback, int32_t subresource,
									std::function<void(std::vector<glm::float4>)> callback)
{
	PE_ASSERT(textureToReadback);
	PE_ASSERT(textureToReadback->GetTextureDesc().usage == ResourceUsage::Default);

	auto textureDesc = textureToReadback->GetTextureDesc();

	auto readbackBuffer = Buffer::Create({
		.bufferName = textureDesc.textureName + L"_ReadbackBuffer",
		.size = RenderDevice::Get().GetTotalSizeInBytes(textureDesc),
		.bindFlags = BindFlags::None,
		.usage = ResourceUsage::Staging,
		.cpuAccess = CPUAccess::Read
	});

	CopyTextureRegion(readbackBuffer, 0, textureToReadback, subresource);

	AddGPUCompletionCallback(
		[callback, readbackBuffer, textureDesc, subresource]()
		{
			SubresourceFootprint subresourceFootprint = RenderDevice::Get().GetSubresourceFootprint(textureDesc, subresource);

			std::vector<glm::float4> output;
			uint8_t* data = (uint8_t*)readbackBuffer->Map(CPUAccess::Read);

			glm::int3 currentPixel = {0, 0, 0};
			while (
				currentPixel.x < subresourceFootprint.size.x &&
				currentPixel.y < subresourceFootprint.size.y &&
				currentPixel.z < subresourceFootprint.size.z)
			{
				glm::float4 color = {0, 0, 0, 0};
				switch (textureDesc.format)
				{
				case TextureFormat::RGBA32_Float:
					color.r = ReadBitsAsFloat(data, 32);
					color.g = ReadBitsAsFloat(data + 4, 32);
					color.b = ReadBitsAsFloat(data + 8, 32);
					color.a = ReadBitsAsFloat(data + 12, 32);
					data += 16;
					break;
				case TextureFormat::RGB32_Float:
					color.r = ReadBitsAsFloat(data, 32);
					color.g = ReadBitsAsFloat(data + 4, 32);
					color.b = ReadBitsAsFloat(data + 8, 32);
					data += 12;
					break;
				case TextureFormat::RGBA16_Float:
					color.r = ReadBitsAsFloat(data, 16);
					color.g = ReadBitsAsFloat(data + 2, 16);
					color.b = ReadBitsAsFloat(data + 4, 16);
					color.a = ReadBitsAsFloat(data + 6, 16);
					data += 8;
					break;
				case TextureFormat::RG32_Float:
					color.r = ReadBitsAsFloat(data, 32);
					color.g = ReadBitsAsFloat(data + 4, 32);
					data += 8;
					break;
				case TextureFormat::RG16_Float:
					color.r = ReadBitsAsFloat(data, 16);
					color.g = ReadBitsAsFloat(data + 2, 16);
					data += 4;
					break;
				case TextureFormat::D32_Float:
				case TextureFormat::R32_Float:
					color.r = ReadBitsAsFloat(data, 32);
					data += 4;
					break;
				case TextureFormat::R16_Float:
					color.r = ReadBitsAsFloat(data, 16);
					data += 2;
					break;
				default:
					PE_ASSERT_NO_ENTRY();
				}

				output.emplace_back(color);

				++currentPixel.x;
				if (currentPixel.x == subresourceFootprint.size.x)
				{
					currentPixel.x = 0;
					++currentPixel.y;
					if (currentPixel.y == subresourceFootprint.size.y)
					{
						currentPixel.y = 0;
						++currentPixel.z;
					}

					int64_t widthAlignment = RenderDevice::Get().GetTexturePitchAlignment();
					data = Align(data, widthAlignment);
				}
			}

			readbackBuffer->Unmap();

			callback(output);
		});
}

void RenderContext::AddCustomRenderCommand(const char* commandName, const std::function<void(RenderCommandList*)>& func)
{
	m_commandRecorder.AllocateCommand<Commands::CustomRenderCommand>(commandName, func);
}

void RenderContext::RenderImGui(ImDrawData* drawData)
{
	auto window = Core::Application::Get().GetMainWindow(); // TODO: hardcoded main window
	Swapchain* swapchain = window->GetSwapchain();
	int32_t index = swapchain->GetCurrentBackBufferIndex();
	m_commandRecorder.AllocateCommand<Commands::CustomRenderCommand>("RenderImGui",
		[swapchain, index, drawData](RenderCommandList* cmdList)
		{
			cmdList->RenderImGui(swapchain, index, drawData);
		});
}

void RenderContext::SetMarker(std::wstring string, glm::float3 color)
{
	m_commandRecorder.AllocateCommand<Commands::CustomRenderCommand>("SetMarker",
		[markerColor = color, formatString = string](RenderCommandList* cmdList)
		{
			cmdList->SetMarker(markerColor, formatString);
		});
}

void RenderContext::BeginEvent(std::wstring string, glm::float3 color)
{
	m_commandRecorder.AllocateCommand<Commands::CustomRenderCommand>("BeginEvent",
		[markerColor = color, formatString = string](RenderCommandList* cmdList)
		{
			cmdList->BeginEvent(markerColor, formatString);
		});
}

void RenderContext::EndEvent()
{
	m_commandRecorder.AllocateCommand<Commands::CustomRenderCommand>("EndEvent",
		[](RenderCommandList* cmdList)
		{
			cmdList->EndEvent();
		});
}

void RenderContext::AddGPUCompletionCallback(std::function<void()> callback)
{
	m_gpuCompletionCallbacks.emplace_back(callback);
}

RenderContext::RenderContext(std::wstring debugName)
	: m_debugName(debugName)
{
}

void RenderContext::CloseContext()
{
	m_commandRecorder.Close();
}

void RenderContext::ExecuteGPUCompletionCallbacks()
{
	for (auto& callback : m_gpuCompletionCallbacks)
		callback();
}
}
