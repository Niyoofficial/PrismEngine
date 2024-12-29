#pragma once

#include "Prism-Core/Render/DeferredCommandRecorder.h"
#include "Prism-Core/Render/RenderCommandList.h"
#include "Prism-Core/Utilities/PreservingObjectContainer.h"


namespace Prism::Render
{
class RenderContext : public RefCounted
{
	friend class RenderCommandQueue;
	friend class RenderDevice;

public:
	void Draw(DrawCommandDesc desc);
	void DrawIndexed(DrawIndexedCommandDesc desc);

	void Dispatch(int32_t threadGroupCountX, int32_t threadGroupCountY, int32_t threadGroupCountZ);

	void SetPSO(GraphicsPipelineState* pso);
	void SetPSO(ComputePipelineState* pso);

	void SetRenderTarget(TextureView* rtv, TextureView* dsv);
	void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv);
	void SetViewport(Viewport viewport);
	void SetViewports(std::vector<Viewport> viewports);
	void SetScissor(Scissor scissor);
	void SetScissors(std::vector<Scissor> scissors);

	void SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes);
	void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format);

	void SetTexture(TextureView* textureView, const std::wstring& paramName, int32_t index = 0);
	void SetBuffer(BufferView* bufferView, const std::wstring& paramName);

	void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr);
	void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr);

	virtual void Barrier(BufferBarrier barrier);
	virtual void Barrier(TextureBarrier barrier);

	void UpdateBuffer(Buffer* buffer, RawData data);
	void UpdateTexture(Texture* texture, RawData data, int32_t subresourceIndex);

	void CopyBufferRegion(Buffer* dest, int64_t destOffset, Buffer* src, int64_t srcOffset, int64_t numBytes);
	void CopyBufferRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex, Buffer* src, int64_t srcOffset);
	void CopyTextureRegion(Buffer* dest, int64_t destOffset, Texture* src, int32_t srcSubresourceIndex = 0, Box srcBox = {});
	void CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex,
						   Texture* src, int32_t srcSubresourceIndex = 0, Box srcBox = {});

	Buffer* ReadbackBuffer(Buffer* bufferToReadback);
	void ReadbackTexture(Texture* textureToReadback, int32_t subresource,
						 std::function<void(std::vector<glm::float4>)> callback);


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
}
