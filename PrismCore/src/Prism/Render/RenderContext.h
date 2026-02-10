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

	void SetRenderTarget(const Ref<TextureView>& rtv, const Ref<TextureView>& dsv);
	void SetRenderTargets(std::vector<Ref<TextureView>> rtvs, const Ref<TextureView>& dsv);
	void SetViewport(Viewport viewport);
	void SetViewports(std::vector<Viewport> viewports);
	void SetScissor(Scissor scissor);
	void SetScissors(std::vector<Scissor> scissors);

	void SetVertexBuffer(const Ref<Buffer>& buffer, int64_t vertexSizeInBytes);
	void SetIndexBuffer(const Ref<Buffer>& buffer, IndexBufferFormat format);

	void SetTexture(const std::wstring& paramName, const Ref<TextureView>& textureView);
	void SetTextures(const std::wstring& paramName, const std::vector<Ref<TextureView>>& textureViews);
	void SetBuffer(const std::wstring& paramName, const Ref<BufferView>& bufferView);
	void SetBuffers(const std::wstring& paramName, const std::vector<Ref<BufferView>>& bufferViews);
	void SetUniformBuffer(const std::wstring& paramName, void* data, int64_t size);
	template<typename T>
	void SetUniformBuffer(const std::wstring& paramName, T&& data)
	{
		SetUniformBuffer(paramName, &std::forward<T>(data), sizeof(T));
	}

	void ClearRenderTargetView(const Ref<TextureView>& rtv, glm::float4* clearColor = nullptr);
	void ClearDepthStencilView(const Ref<TextureView>& dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr);
	void ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::float4 values);
	void ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::uint4 values);

	void Barrier(BufferBarrier barrier);
	void Barrier(TextureBarrier barrier);

	void UpdateBuffer(const Ref<Buffer>& buffer, RawData data);
	void UpdateTexture(const Ref<Texture>& texture, RawData data, int32_t subresourceIndex);

	void CopyBufferRegion(const Ref<Buffer>& dest, int64_t destOffset, const Ref<Buffer>& src, int64_t srcOffset,
						  int64_t numBytes);
	void CopyBufferRegion(const Ref<Texture>& dest, glm::int3 destLoc, int32_t destSubresourceIndex, const Ref<Buffer>& src,
						  int64_t srcOffset);
	void CopyTextureRegion(const Ref<Buffer>& dest, int64_t destOffset, const Ref<Texture>& src, int32_t srcSubresourceIndex = 0,
						   Box3I srcBox = {});
	void CopyTextureRegion(const Ref<Texture>& dest, glm::int3 destLoc, int32_t destSubresourceIndex,
						   const Ref<Texture>& src, int32_t srcSubresourceIndex = 0, Box3I srcBox = {});

	Buffer* ReadbackBuffer(const Ref<Buffer>& bufferToReadback);
	void ReadbackTexture(const Ref<Texture>& textureToReadback, int32_t subresource,
						 std::function<void(std::vector<glm::float4>)> callback);

	void RenderImGui();

	void SetMarker(std::wstring string, glm::float3 color = {});
	void BeginEvent(std::wstring string, glm::float3 color = {});
	void EndEvent();


	/* Add a callback that will be executed when this context is finished by the GPU */
	void AddGPUCompletionCallback(std::function<void()> callback);

	template<typename T>
	void SafeReleaseResource(const Ref<T>& resource)
	{
		m_preservedResources.AddObject(resource);
	}

	void SafeReleaseResource(RefCounted* resource)
	{
		m_preservedResources.AddObject(Ref(resource));
	}

	explicit RenderContext(std::wstring debugName);

private:
	void CloseContext();
	void ExecuteGPUCompletionCallbacks();

private:
	std::wstring m_debugName;

	DeferredCommandRecorder m_commandRecorder;

	PreservingObjectContainer m_preservedResources;

	std::vector<std::function<void()>> m_gpuCompletionCallbacks;

	std::unordered_map<std::wstring, Ref<Buffer>> m_uniformBuffers;
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

#define SCOPED_RENDER_EVENT(context, string, ...) const auto PREPROCESSOR_JOIN(scopedRenderEvent_, __LINE__) = ::Prism::Render::Private::ScopedRenderEvent(context, string __VA_OPT__(,) __VA_ARGS__)
