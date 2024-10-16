#pragma once
#include "Prism-Core/Render/Buffer.h"
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
	int64_t numVertices = 0;
	int64_t numInstances = 1;
	int64_t startVertexLocation = 0;
};

struct DrawIndexedCommandDesc
{
	int64_t numIndices = 0;
	int64_t numInstances = 1;
	int64_t startIndexLocation = 0;
	int64_t baseVertexLocation = 0;
};

enum class IndexBufferFormat
{
	Uint16,
	Uint32
};

class RenderContext : public RefCounted
{
public:
	static RenderContext* Create();


	virtual void Draw(DrawCommandDesc desc) = 0;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) = 0;

	virtual void Dispatch(int32_t threadGroupCountX, int32_t threadGroupCountY, int32_t threadGroupCountZ) = 0;

	virtual void SetPSO(GraphicsPipelineState* pso) = 0;
	virtual void SetPSO(ComputePipelineState* pso) = 0;

	void SetRenderTarget(TextureView* rtv, TextureView* dsv);
	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) = 0;
	void SetViewport(Viewport viewport);
	virtual void SetViewports(std::vector<Viewport> viewports) = 0;
	void SetScissor(Scissor scissor);
	virtual void SetScissors(std::vector<Scissor> scissors) = 0;

	virtual void SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes) = 0;
	virtual void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format) = 0;

	virtual void SetTexture(TextureView* textureView, const std::wstring& paramName, int32_t index = 0) = 0;
	virtual void SetCBuffer(BufferView* bufferView, const std::wstring& paramName) = 0;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) = 0;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) = 0;

	virtual void Transition(StateTransitionDesc desc) = 0;

	virtual void UpdateBuffer(Buffer* buffer, RawData data) = 0;
	virtual void UpdateTexture(Texture* texture, RawData data, int32_t subresourceIndex) = 0;

	virtual void CopyBufferRegion(Buffer* dest, int32_t destOffset, Buffer* src, int32_t srcOffset, int32_t numBytes) = 0;
	virtual void CopyTextureRegion(Texture* dest, int32_t x, int32_t y, int32_t z, int32_t subresourceIndex, Buffer* src, int64_t srcOffset = 0) = 0;


	// Objects MUST be std::move'd into this function
	template<typename T>
	void SafeReleaseResource(T&& resource) requires !std::is_lvalue_reference_v<T>
	{
		m_preservedResources.AddObject(std::move(resource));
	}


	// Called by RenderDevice before executing
	virtual void CloseContext() = 0;

protected:
	PreservingObjectContainer m_preservedResources;
};
}
