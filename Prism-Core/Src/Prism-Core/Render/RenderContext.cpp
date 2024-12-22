#include "pcpch.h"
#include "RenderContext.h"

#include "Prism-Core/Render/RenderCommands.h"
#include "Prism-Core/Render/RenderResourceCreation.h"


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

void RenderContext::Dispatch(int32_t threadGroupCountX, int32_t threadGroupCountY, int32_t threadGroupCountZ)
{
	m_commandRecorder.AllocateCommand<Commands::DispatchRenderCommand>(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void RenderContext::SetPSO(GraphicsPipelineState* pso)
{
	m_commandRecorder.AllocateCommand<Commands::SetGraphicsPSORenderCommand>(pso);
}

void RenderContext::SetPSO(ComputePipelineState* pso)
{
	m_commandRecorder.AllocateCommand<Commands::SetComputePSORenderCommand>(pso);
}

void RenderContext::SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv)
{
	m_commandRecorder.AllocateCommand<Commands::SetRenderTargetsRenderCommand>(rtvs, dsv);
}

void RenderContext::SetViewports(std::vector<Viewport> viewports)
{
	m_commandRecorder.AllocateCommand<Commands::SetViewportsRenderCommand>(viewports);
}

void RenderContext::SetScissors(std::vector<Scissor> scissors)
{
	m_commandRecorder.AllocateCommand<Commands::SetScissorsRenderCommand>(scissors);
}

void RenderContext::SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes)
{
	m_commandRecorder.AllocateCommand<Commands::SetVertexBufferRenderCommand>(buffer, vertexSizeInBytes);
}

void RenderContext::SetIndexBuffer(Buffer* buffer, IndexBufferFormat format)
{
	m_commandRecorder.AllocateCommand<Commands::SetIndexBufferRenderCommand>(buffer, format);
}

void RenderContext::SetTexture(TextureView* textureView, const std::wstring& paramName, int32_t index)
{
	m_commandRecorder.AllocateCommand<Commands::SetTextureRenderCommand>(textureView, paramName, index);
}

void RenderContext::SetBuffer(BufferView* bufferView, const std::wstring& paramName)
{
	m_commandRecorder.AllocateCommand<Commands::SetBufferRenderCommand>(bufferView, paramName);
}

void RenderContext::ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor)
{
	m_commandRecorder.AllocateCommand<Commands::ClearRenderTargetViewRenderCommand>(rtv, clearColor);
}

void RenderContext::ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue)
{
	m_commandRecorder.AllocateCommand<Commands::ClearDepthStencilViewRenderCommand>(dsv, flags, clearValue);
}

void RenderContext::Barrier(BufferBarrier barrier)
{
	m_commandRecorder.AllocateCommand<Commands::BufferBarrierRenderCommand>(barrier);
}

void RenderContext::Barrier(TextureBarrier barrier)
{
	m_commandRecorder.AllocateCommand<Commands::TextureBarrierRenderCommand>(barrier);
}

void RenderContext::UpdateBuffer(Buffer* buffer, RawData data)
{
	m_commandRecorder.AllocateCommand<Commands::UpdateBufferRenderCommand>(buffer, data);
}

void RenderContext::UpdateTexture(Texture* texture, RawData data, int32_t subresourceIndex)
{
	m_commandRecorder.AllocateCommand<Commands::UpdateTextureRenderCommand>(texture, data, subresourceIndex);
}

Buffer* RenderContext::ReadbackBuffer(Buffer* bufferToReadback)
{
	PE_ASSERT(bufferToReadback);
	PE_ASSERT(bufferToReadback->GetBufferDesc().usage == ResourceUsage::Default);

	auto readbackBuffer = Buffer::Create({
		.bufferName = bufferToReadback->GetBufferDesc().bufferName + L"_ReadbackBuffer",
		.size = RenderDevice::Get().GetAlignedSizeInBytes(bufferToReadback->GetBufferDesc()),
		.bindFlags = BindFlags::None,
		.usage = ResourceUsage::Staging,
		.cpuAccess = CPUAccess::Read
	});

	CopyBufferRegion(readbackBuffer, 0, bufferToReadback, 0, bufferToReadback->GetBufferDesc().size);

	auto readbackBufferTemp = readbackBuffer;
	SafeReleaseResource(readbackBufferTemp);

	return readbackBuffer;
}

Buffer* RenderContext::ReadbackTexture(Texture* textureToReadback)
{
	PE_ASSERT(textureToReadback);
	PE_ASSERT(textureToReadback->GetTextureDesc().usage == ResourceUsage::Default);

	auto readbackBuffer = Buffer::Create({
		.bufferName = textureToReadback->GetTextureDesc().textureName + L"_ReadbackBuffer",
		.size = RenderDevice::Get().GetAlignedSizeInBytes(textureToReadback->GetTextureDesc()),
		.bindFlags = BindFlags::None,
		.usage = ResourceUsage::Staging,
		.cpuAccess = CPUAccess::Read
	});

	int64_t bufferOffset = 0;
	for (int32_t i = 0; i < textureToReadback->GetTextureDesc().GetSubresourceCount(); ++i)
	{
		CopyTextureRegion(readbackBuffer, bufferOffset, textureToReadback, i);
		bufferOffset += RenderDevice::Get().GetAlignedSizeInBytes(textureToReadback->GetTextureDesc(), i, 1);
	}

	auto readbackBufferTemp = readbackBuffer;
	SafeReleaseResource(readbackBufferTemp);

	return readbackBuffer;
}

void RenderContext::CopyBufferRegion(Buffer* dest, int64_t destOffset, Buffer* src, int64_t srcOffset, int64_t numBytes)
{
	m_commandRecorder.AllocateCommand<Commands::CopyBufferRegionRenderCommand>(dest, destOffset, src, srcOffset, numBytes);
}

void RenderContext::CopyBufferRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex, Buffer* src, int64_t srcOffset)
{
	m_commandRecorder.AllocateCommand<Commands::CopyBufferRegionToTextureRenderCommand>(dest, destLoc, destSubresourceIndex, src, srcOffset);
}

void RenderContext::CopyTextureRegion(Buffer* dest, int64_t destOffset, Texture* src, int32_t srcSubresourceIndex, Box srcBox)
{
	m_commandRecorder.AllocateCommand<Commands::CopyTextureRegionToBufferRenderCommand>(
		dest, destOffset,
		src, srcSubresourceIndex, srcBox);
}

void RenderContext::CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex,
									  Texture* src, int32_t srcSubresourceIndex, Box srcBox)
{
	m_commandRecorder.AllocateCommand<Commands::CopyTextureRegionToTextureRenderCommand>(
		dest, destLoc, destSubresourceIndex,
		src, srcSubresourceIndex, srcBox);
}

void RenderContext::SetRenderTarget(TextureView* rtv, TextureView* dsv)
{
	PE_ASSERT(rtv != nullptr);

	SetRenderTargets({rtv}, dsv);
}

void RenderContext::SetViewport(Viewport viewport)
{
	SetViewports({viewport});
}

void RenderContext::SetScissor(Scissor scissor)
{
	SetScissors({scissor});
}

void RenderContext::CloseContext()
{
}
}
