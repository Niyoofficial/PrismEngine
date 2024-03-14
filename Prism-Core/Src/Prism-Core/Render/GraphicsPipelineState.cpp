#include "pcpch.h"
#include "GraphicsPipelineState.h"

#include "Prism-Core/Render/RenderDevice.h"
#include "Prism-Core/Render/PipelineStateCache.h"

namespace Prism::Render
{
GraphicsPipelineState* GraphicsPipelineState::Create(const GraphicsPipelineStateDesc& desc)
{
	return RenderDevice::Get().GetPipelineStateCache().GetOrCreatePipelineState(desc);
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
