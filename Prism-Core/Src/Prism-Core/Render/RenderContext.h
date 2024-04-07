#pragma once
#include "Prism-Core/Render/GraphicsPipelineState.h"


namespace Prism::Render
{
class Buffer;
class Texture;
class TextureView;

struct DrawCommandDesc
{
	int32_t numVertices = 0;
	int32_t numInstances = 1;
	int32_t startVertexLocation = 0;
};

struct DrawIndexedCommandDesc
{
	int32_t numIndices = 0;
	int32_t numInstances = 1;
	int32_t startIndexLocation = 0;
	int32_t baseVertexLocation = 0;
};

enum class IndexBufferFormat
{
	Uint16,
	Uint32
};

class RenderContext
{
public:
	virtual ~RenderContext() = default;

	static RenderContext* Create();


	virtual void Draw(DrawCommandDesc desc) = 0;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) = 0;

	virtual void SetPSO(GraphicsPipelineState* pso) = 0;

	void SetRenderTarget(TextureView* rtv, TextureView* dsv);
	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) = 0;
	void SetViewport(Viewport viewport);
	virtual void SetViewports(std::vector<Viewport> viewports) = 0;
	void SetScissor(Scissor scissor);
	virtual void SetScissors(std::vector<Scissor> scissors) = 0;

	virtual void SetVertexBuffer(Buffer* buffer, int32_t vertexSizeInBytes) = 0;
	virtual void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format) = 0;

	virtual void SetUniformBuffer(Buffer* buffer, const std::wstring& paramName = 0) = 0;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) = 0;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) = 0;

	virtual void Transition(StateTransitionDesc desc) = 0;

	virtual void CopyBufferRegion(Buffer* dest, int32_t destOffset, Buffer* src, int32_t srcOffset, int32_t numBytes) = 0;


	// Called by RenderDevice before executing
	virtual void CloseContext() = 0;
};
}
