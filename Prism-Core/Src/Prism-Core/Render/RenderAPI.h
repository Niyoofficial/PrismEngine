#pragma once
#include "Prism-Core/Render/PipelineStateCache.h"
#include "Prism-Core/Render/ShaderCache.h"

DECLARE_LOG_CATEGORY(PERender, "Prism-Render");
#define PE_RENDER_LOG(verbosity, ...) PE_LOG(PERender, verbosity, __VA_ARGS__)

namespace Prism::Render
{
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

class TextureView;

class RenderAPI
{
public:
	virtual ~RenderAPI() = default;

	static RenderAPI* Create();

	ShaderCache& GetShaderCache() { return m_shaderCache; }
	const ShaderCache& GetShaderCache() const { return m_shaderCache; }

	PipelineStateCache& GetPipelineStateCache() { return m_pipelineStateCache; }
	const PipelineStateCache& GetPipelineStateCache() const { return m_pipelineStateCache; }


	virtual void Begin() = 0;
	virtual void End() = 0;

	virtual void FlushCommandQueue() = 0;

	virtual void Draw(DrawCommandDesc desc) = 0;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) = 0;

	virtual void SetPSO(GraphicsPipelineState* pso) = 0;

	void SetRenderTarget(TextureView* rtv, TextureView* dsv);
	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) = 0;
	void SetViewport(Viewport viewport);
	virtual void SetViewports(std::vector<Viewport> viewports) = 0;
	void SetScissor(Scissor scissor);
	virtual void SetScissors(std::vector<Scissor> scissors) = 0;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) = 0;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) = 0;

	virtual void Transition(StateTransitionDesc desc) = 0;

protected:
	ShaderCache m_shaderCache;
	PipelineStateCache m_pipelineStateCache;
};
}
