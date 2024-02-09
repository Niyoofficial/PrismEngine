#include "pcpch.h"
#include "GraphicsPipelineState.h"

#include "Prism-Core/Render/RenderAPI.h"
#include "Prism-Core/Render/Renderer.h"
#include "Prism-Core/Render/PipelineStateCache.h"

namespace Prism::Render
{
GraphicsPipelineState* GraphicsPipelineState::Create(const GraphicsPipelineStateDesc& desc)
{
	return Renderer::Get().GetRenderAPI()->GetPipelineStateCache().GetOrCreatePipelineState(desc);
}

GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
	: m_desc(desc)
{
}

PipelineStateHash::PipelineStateHash(const GraphicsPipelineStateDesc& desc)
	: Hash(desc)
{
}
}
