#include "pcpch.h"
#include "PipelineStateCache.h"

#include "Prism-Core/Render/RenderResourceCreation.h"

namespace Prism::Render
{
GraphicsPipelineState* PipelineStateCache::GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc)
{
	auto findIt = m_pipelineStates.find(desc);
	if (findIt != m_pipelineStates.end())
		return findIt->second;

	auto [it, success] = m_pipelineStates.emplace(desc, Private::CreatePipelineState(desc));
	return it->second;
}
}
