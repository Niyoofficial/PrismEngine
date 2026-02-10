#pragma once
#include "Prism/Render/RenderCommandList.h"
#include "Prism/Render/Texture.h"
#include "Prism/Render/Buffer.h"

namespace Prism::Render::Commands
{
struct RenderCommandBase
{
	virtual void ExecuteAndDestruct(RenderCommandList* cmdList) = 0;
	virtual ~RenderCommandBase() = default;

	RenderCommandBase* next = nullptr;
};

template<typename T, typename StringT>
struct RenderCommand : public RenderCommandBase
{
	static const wchar_t* GetCommandString()
	{
		return StringT::GetCommandString();
	}

	virtual void ExecuteAndDestruct(RenderCommandList* cmdList) override final
	{
		// TODO: Add debug markers using command string from StringT

		T* command = static_cast<T*>(this);
		command->Execute(cmdList);
		command->~T();
	}
};

#define DEFINE_RENDER_COMMAND(commandName)																			\
	struct PREPROCESSOR_JOIN(commandName##String, __LINE__) 														\
	{																												\
		static const wchar_t* GetCommandString() { return PREPROCESSOR_TO_WIDE_STRING(commandName); }				\
	};																												\
	struct commandName final : public RenderCommand<commandName, PREPROCESSOR_JOIN(commandName##String, __LINE__)>

DEFINE_RENDER_COMMAND(CustomRenderCommand)
{
	explicit CustomRenderCommand(std::function<void(RenderCommandList*)> inFunc)
		: func(inFunc)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		func(cmdList);
	}

	std::function<void(RenderCommandList*)> func;
};

DEFINE_RENDER_COMMAND(DrawRenderCommand)
{
	explicit DrawRenderCommand(DrawCommandDesc inDesc)
		: desc(inDesc)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->Draw(desc);
	}

	DrawCommandDesc desc = {};
};

DEFINE_RENDER_COMMAND(DrawIndexedRenderCommand)
{
	explicit DrawIndexedRenderCommand(DrawIndexedCommandDesc inDesc)
		: desc(inDesc)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->DrawIndexed(desc);
	}

	DrawIndexedCommandDesc desc = {};
};

DEFINE_RENDER_COMMAND(DispatchRenderCommand)
{
	DispatchRenderCommand(glm::int3 inThreadGroupCount)
		: threadGroupCount(inThreadGroupCount)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->Dispatch(threadGroupCount);
	}

	glm::int3 threadGroupCount = {-1, -1, -1};
};

DEFINE_RENDER_COMMAND(SetGraphicsPSORenderCommand)
{
	explicit SetGraphicsPSORenderCommand(const GraphicsPipelineStateDesc& inPSO)
		: pso(inPSO)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetPSO(pso);
	}

	GraphicsPipelineStateDesc pso;
};

DEFINE_RENDER_COMMAND(SetComputePSORenderCommand)
{
	explicit SetComputePSORenderCommand(const ComputePipelineStateDesc& inPSO)
		: pso(inPSO)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetPSO(pso);
	}

	ComputePipelineStateDesc pso;
};

DEFINE_RENDER_COMMAND(SetRenderTargetsRenderCommand)
{
	SetRenderTargetsRenderCommand(const std::vector<Ref<TextureView>>& inRtvs, const Ref<TextureView> inDsv)
		: rtvs(inRtvs), dsv(inDsv)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetRenderTargets(rtvs, dsv);
	}

	std::vector<Ref<TextureView>> rtvs = {};
	Ref<TextureView> dsv;
};

DEFINE_RENDER_COMMAND(SetViewportsRenderCommand)
{
	explicit SetViewportsRenderCommand(std::vector<Viewport> inViewports)
		: viewports(inViewports)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetViewports(viewports);
	}

	std::vector<Viewport> viewports = {};
};

DEFINE_RENDER_COMMAND(SetScissorsRenderCommand)
{
	explicit SetScissorsRenderCommand(std::vector<Scissor> inScissors)
		: scissors(inScissors)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetScissors(scissors);
	}

	std::vector<Scissor> scissors = {};
};

DEFINE_RENDER_COMMAND(SetVertexBufferRenderCommand)
{
SetVertexBufferRenderCommand(const Ref<Buffer>& inBuffer, int64_t inVertexSizeInBytes)
    : buffer(inBuffer), vertexSizeInBytes(inVertexSizeInBytes)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetVertexBuffer(buffer, vertexSizeInBytes);
	}

	Ref<Buffer> buffer;
	int64_t vertexSizeInBytes = -1;
};

DEFINE_RENDER_COMMAND(SetIndexBufferRenderCommand)
{
SetIndexBufferRenderCommand(const Ref<Buffer>& inBuffer, IndexBufferFormat inFormat)
    : buffer(inBuffer), format(inFormat)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetIndexBuffer(buffer, format);
	}

	Ref<Buffer> buffer;
	IndexBufferFormat format = {};
};

DEFINE_RENDER_COMMAND(SetTextureRenderCommand)
{
SetTextureRenderCommand(const Ref<TextureView>& inTextureView, const std::wstring& inParamName)
    : textureView(inTextureView), paramName(inParamName)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetTexture(textureView, paramName);
	}

	Ref<TextureView> textureView;
	std::wstring paramName = {};
};

DEFINE_RENDER_COMMAND(SetTexturesRenderCommand)
{
	SetTexturesRenderCommand(const std::vector<Ref<TextureView>>& inTextureViews, const std::wstring& inParamName)
		: textureViews(inTextureViews), paramName(inParamName)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetTextures(textureViews, paramName);
	}

	std::vector<Ref<TextureView>> textureViews;
	std::wstring paramName = {};
};

DEFINE_RENDER_COMMAND(SetBufferRenderCommand)
{
SetBufferRenderCommand(const Ref<BufferView>& inBufferView, const std::wstring& inParamName)
    : bufferView(inBufferView), paramName(inParamName)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetBuffer(bufferView, paramName);
	}

	Ref<BufferView> bufferView;
	std::wstring paramName = {};
};

DEFINE_RENDER_COMMAND(SetBuffersRenderCommand)
{
	SetBuffersRenderCommand(const std::vector<Ref<BufferView>>& inBufferViews, const std::wstring& inParamName)
		: bufferViews(inBufferViews), paramName(inParamName)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->SetBuffers(bufferViews, paramName);
	}

	std::vector<Ref<BufferView>> bufferViews;
	std::wstring paramName = {};
};

DEFINE_RENDER_COMMAND(ClearRenderTargetViewRenderCommand)
{
ClearRenderTargetViewRenderCommand(const Ref<TextureView>& inRtv, glm::float4* inClearColor = nullptr)
    : rtv(inRtv)
	{
		if (inClearColor)
			clearColor = *inClearColor;
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->ClearRenderTargetView(rtv, clearColor.has_value() ? &clearColor.value() : nullptr);
	}

	Ref<TextureView> rtv;
	std::optional<glm::float4> clearColor;
};

DEFINE_RENDER_COMMAND(ClearDepthStencilViewRenderCommand)
{
ClearDepthStencilViewRenderCommand(const Ref<TextureView>& inDsv, Flags<ClearFlags> inFlags, DepthStencilValue* inClearValue = nullptr)
    : dsv(inDsv), flags(inFlags)
	{
		if (inClearValue)
			clearValue = *inClearValue;
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->ClearDepthStencilView(dsv, flags, clearValue.has_value() ? &clearValue.value() : nullptr);
	}

	Ref<TextureView> dsv;
	Flags<ClearFlags> flags = {};
	std::optional<DepthStencilValue> clearValue;
};

DEFINE_RENDER_COMMAND(BufferBarrierRenderCommand)
{
	explicit BufferBarrierRenderCommand(BufferBarrier inBufferBarrier)
		: bufferBarrier(inBufferBarrier)
	{
		buffer = bufferBarrier.buffer;
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->Barrier(bufferBarrier);
	}

	BufferBarrier bufferBarrier = {};
	Ref<Buffer> buffer;
};

DEFINE_RENDER_COMMAND(TextureBarrierRenderCommand)
{
	explicit TextureBarrierRenderCommand(TextureBarrier inTextureBarrier)
		: textureBarrier(inTextureBarrier)
	{
		texture = textureBarrier.texture;
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->Barrier(textureBarrier);
	}

	TextureBarrier textureBarrier = {};
	Ref<Texture> texture;
};

DEFINE_RENDER_COMMAND(UpdateBufferRenderCommand)
{
UpdateBufferRenderCommand(const Ref<Buffer>& inBuffer, RawData inData)
    : buffer(inBuffer), data(inData)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->UpdateBuffer(buffer, data);
	}

	Ref<Buffer> buffer;
	RawData data = {};
};

DEFINE_RENDER_COMMAND(UpdateTextureRenderCommand)
{
UpdateTextureRenderCommand(const Ref<Texture>& inTexture, RawData inData, int32_t inSubresourceIndex)
    : texture(inTexture), data(inData), subresourceIndex(inSubresourceIndex)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->UpdateTexture(texture, data, subresourceIndex);
	}

	Ref<Texture> texture;
	RawData data = {};
	int32_t subresourceIndex = -1;
};

DEFINE_RENDER_COMMAND(CopyBufferRegionRenderCommand)
{
CopyBufferRegionRenderCommand(const Ref<Buffer>& inDest, int64_t inDestOffset, const Ref<Buffer>& inSrc, int64_t inSrcOffset, int64_t inNumBytes)
    : dest(inDest), destOffset(inDestOffset), src(inSrc), srcOffset(inSrcOffset), numBytes(inNumBytes)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->CopyBufferRegion(dest, destOffset, src, srcOffset, numBytes);
	}

	Ref<Buffer> dest;
	int64_t destOffset = -1;
	Ref<Buffer> src;
	int64_t srcOffset = -1;
	int64_t numBytes = -1;
};

DEFINE_RENDER_COMMAND(CopyBufferRegionToTextureRenderCommand)
{
CopyBufferRegionToTextureRenderCommand(const Ref<Texture>& inDest, glm::int3 inDestLoc, int32_t inSubresourceIndex, const Ref<Buffer>& inSrc, int64_t inSrcOffset)
    : dest(inDest), destLoc(inDestLoc), subresourceIndex(inSubresourceIndex), src(inSrc), srcOffset(inSrcOffset)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->CopyBufferRegion(dest, destLoc, subresourceIndex, src, srcOffset);
	}

	Ref<Texture> dest;
	glm::int3 destLoc;
	int32_t subresourceIndex = -1;
	Ref<Buffer> src;
	int64_t srcOffset = -1;
};

DEFINE_RENDER_COMMAND(CopyTextureRegionToBufferRenderCommand)
{
CopyTextureRegionToBufferRenderCommand(const Ref<Buffer>& inDest, int64_t inDestOffset, const Ref<Texture>& inSrc, int32_t inSrcSubresourceIndex, Box3I inSrcBox)
    : dest(inDest), dsetOffset(inDestOffset),
      src(inSrc), srcSubresourceIndex(inSrcSubresourceIndex), srcBox(inSrcBox)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->CopyTextureRegion(dest, dsetOffset, src, srcSubresourceIndex, srcBox);
	}

	Ref<Buffer> dest;
	int64_t dsetOffset;
	Ref<Texture> src;
	int32_t srcSubresourceIndex = -1;
	Box3I srcBox;
};

DEFINE_RENDER_COMMAND(CopyTextureRegionToTextureRenderCommand)
{
CopyTextureRegionToTextureRenderCommand(const Ref<Texture>& inDest, glm::int3 inDestLoc, int32_t inSubresourceIndex,
                                       const Ref<Texture>& inSrc, int32_t inSrcSubresourceIndex, Box3I inSrcBox)
    : dest(inDest), destLoc(inDestLoc), destSubresourceIndex(inSubresourceIndex),
      src(inSrc), srcSubresourceIndex(inSrcSubresourceIndex), srcBox(inSrcBox)
	{
	}

	void Execute(RenderCommandList* cmdList)
	{
		cmdList->CopyTextureRegion(dest, destLoc, destSubresourceIndex, src, srcSubresourceIndex, srcBox);
	}

	Ref<Texture> dest;
	glm::int3 destLoc;
	int32_t destSubresourceIndex = -1;
	Ref<Texture> src;
	int32_t srcSubresourceIndex = -1;
	Box3I srcBox;
};
}
