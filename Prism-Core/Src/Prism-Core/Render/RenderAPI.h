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

class RenderAPI
{
public:
	static RenderAPI* Create();

	ShaderCache& GetShaderCache() { return m_shaderCache; }
	const ShaderCache& GetShaderCache() const { return m_shaderCache; }

	PipelineStateCache& GetPipelineStateCache() { return m_pipelineStateCache; }
	const PipelineStateCache& GetPipelineStateCache() const { return m_pipelineStateCache; }

	virtual void Begin() = 0;
	virtual void End() = 0;

	virtual void Draw(DrawCommandDesc desc) = 0;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) = 0;

	virtual void SetPSO(GraphicsPipelineState* pso) = 0;

protected:
	ShaderCache m_shaderCache;
	PipelineStateCache m_pipelineStateCache;
};
}
