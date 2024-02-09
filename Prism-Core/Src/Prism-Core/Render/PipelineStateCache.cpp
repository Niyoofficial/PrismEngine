#include "pcpch.h"
#include "PipelineStateCache.h"

namespace Prism::Render
{
GraphicsPipelineState* PipelineStateCache::GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc)
{
	PipelineStateHash hash(desc);

	auto findIt = m_pipelineStates.find(hash);
	if (findIt != m_pipelineStates.end())
		return findIt->second.get();

	auto [it, success] = m_pipelineStates.emplace(hash, CreatePipelineState(desc));
	return it->second.get();
}
}
