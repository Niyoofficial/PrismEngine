#pragma once

#include "Prism/Render/DeferredCommandRecorder.h"
#include "Prism/Render/RenderCommandList.h"
#include "Prism/Utilities/PreservingObjectContainer.h"


namespace Prism::Render
{
class RenderContext : public RefCounted
{
	friend class RenderCommandQueue;
	friend class RenderDevice;

public:
	// TODO: Maybe instead of copying the render command functions from RenderCommandList
	// we should have single SubmitCommand function taking a lambda with RenderCommandList parameter?
	// It would allow for some render command related calculations to be automatically offloaded to a different thread

	void Draw(DrawCommandDesc desc);
	void DrawIndexed(DrawIndexedCommandDesc desc);

	void Dispatch(glm::int3 threadGroupCount);

	void SetPSO(const GraphicsPipelineStateDesc& pso);
	void SetPSO(const ComputePipelineStateDesc& pso);

	void SetStencilRef(uint32_t ref);

	void SetRenderTarget(TextureView* rtv, TextureView* dsv);
	void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv);
	void SetViewport(Viewport viewport);
	void SetViewports(std::vector<Viewport> viewports);
	void SetScissor(Scissor scissor);
	void SetScissors(std::vector<Scissor> scissors);

	void SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes);
	void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format);

	void SetTexture(TextureView* textureView, const std::wstring& paramName);
	void SetTextures(const std::vector<Ref<TextureView>>& textureViews, const std::wstring& paramName);
	void SetBuffer(BufferView* bufferView, const std::wstring& paramName);
	void SetBuffers(const std::vector<Ref<BufferView>>& bufferViews, const std::wstring& paramName);

	void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr);
	void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr);
	void ClearUnorderedAccessView(TextureView* uav, glm::float4 values);
	void ClearUnorderedAccessView(TextureView* uav, glm::uint4 values);

	void Barrier(BufferBarrier barrier);
	void Barrier(TextureBarrier barrier);

	void UpdateBuffer(Buffer* buffer, RawData data);
	void UpdateTexture(Texture* texture, RawData data, int32_t subresourceIndex);

	void CopyBufferRegion(Buffer* dest, int64_t destOffset, Buffer* src, int64_t srcOffset, int64_t numBytes);
	void CopyBufferRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex, Buffer* src, int64_t srcOffset);
	void CopyTextureRegion(Buffer* dest, int64_t destOffset, Texture* src, int32_t srcSubresourceIndex = 0, Box3I srcBox = {});
	void CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex,
						   Texture* src, int32_t srcSubresourceIndex = 0, Box3I srcBox = {});

	Buffer* ReadbackBuffer(Buffer* bufferToReadback);
	void ReadbackTexture(Texture* textureToReadback, int32_t subresource,
						 std::function<void(std::vector<glm::float4>)> callback);

	void RenderImGui();

	void SetMarker(std::wstring string, glm::float3 color = {});
	void BeginEvent(std::wstring string, glm::float3 color = {});
	void EndEvent();


	/* Add a callback that will be executed when this context is finished by the GPU */
	void AddGPUCompletionCallback(std::function<void()> callback);

	template<typename T>
	void SafeReleaseResource(T&& resource)
	{
		m_preservedResources.AddObject(std::move(resource));
	}

private:
	RenderContext() = default;

	void CloseContext();
	void ExecuteGPUCompletionCallbacks();

private:
	DeferredCommandRecorder m_commandRecorder;

	PreservingObjectContainer m_preservedResources;

	std::vector<std::function<void()>> m_gpuCompletionCallbacks;
};

namespace Private
{
	struct ScopedRenderEvent
	{
		ScopedRenderEvent(RenderContext* context, std::wstring string, glm::float3 color = {})
			: m_context(context)
		{
			PE_ASSERT(context);
			m_context->BeginEvent(string, color);
		}
		~ScopedRenderEvent()
		{
			m_context->EndEvent();
		}

		ScopedRenderEvent(const ScopedRenderEvent&) = delete;
		ScopedRenderEvent(ScopedRenderEvent&&) = delete;
		bool operator=(const ScopedRenderEvent&) const = delete;
		bool operator=(ScopedRenderEvent&&) const = delete;

	private:
		Ref<RenderContext> m_context = nullptr;
	};
}
}

#define SCOPED_RENDER_EVENT(context, string, ...) do {::Prism::Render::Private::ScopedRenderEvent(context, string __VA_OPT__(,) __VA_ARGS__);} while (0)