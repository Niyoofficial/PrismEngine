#pragma once
#include "Prism-Core/Render/PipelineState.h"
#include "Prism-Core/Utilities/PreservingObjectContainer.h"

namespace Prism::Render
{
class Buffer;
class BufferView;
class Texture;
class TextureView;

struct DrawCommandDesc
{
	int64_t numVertices;
	int64_t numInstances = 1;
	int64_t startVertexLocation;
};

struct DrawIndexedCommandDesc
{
	int64_t numIndices;
	int64_t numInstances = 1;
	int64_t startIndexLocation;
	int64_t baseVertexLocation;
};

enum class IndexBufferFormat
{
	Uint16,
	Uint32
};

class RenderCommandList : public RefCounted
{
	friend class RenderCommandQueue;

public:
	virtual void Draw(DrawCommandDesc desc) = 0;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) = 0;

	virtual void Dispatch(int32_t threadGroupCountX, int32_t threadGroupCountY, int32_t threadGroupCountZ) = 0;

	virtual void SetPSO(GraphicsPipelineState* pso) = 0;
	virtual void SetPSO(ComputePipelineState* pso) = 0;

	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) = 0;
	virtual void SetViewports(std::vector<Viewport> viewports) = 0;
	virtual void SetScissors(std::vector<Scissor> scissors) = 0;

	virtual void SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes) = 0;
	virtual void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format) = 0;

	virtual void SetTexture(TextureView* textureView, const std::wstring& paramName, int32_t index) = 0;
	virtual void SetBuffer(BufferView* bufferView, const std::wstring& paramName) = 0;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) = 0;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) = 0;

	virtual void Barrier(BufferBarrier barrier) = 0;
	virtual void Barrier(TextureBarrier barrier) = 0;

	virtual void UpdateBuffer(Buffer* buffer, RawData data) = 0;
	virtual void UpdateTexture(Texture* texture, RawData data, int32_t subresourceIndex) = 0;

	virtual void CopyBufferRegion(Buffer* dest, int64_t destOffset, Buffer* src, int64_t srcOffset, int64_t numBytes) = 0;
	virtual void CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex, Buffer* src, int64_t srcOffset) = 0;
	virtual void CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex,
								   Texture* src, int32_t srcSubresourceIndex, glm::int3 srcLoc, glm::int3 srcSize) = 0;

	uint64_t GetFenceValue() const { return m_fenceValue; }

	template<typename T>
	void SafeReleaseResource(T&& resource)
	{
		m_preservedResources.AddObject(std::move(resource));
	}

protected:
	explicit RenderCommandList(uint64_t fenceValue);

	static Ref<RenderCommandList> Create(uint64_t fenceValue);

	virtual void Close();
	bool IsClosed() const;

private:
	PreservingObjectContainer m_preservedResources;
	std::atomic<bool> m_closed = false;
	uint64_t m_fenceValue = 0;
};
}
