#include "pcpch.h"
#include "PipelineStateCache.h"

#include "Prism/Render/RenderResourceCreation.h"

namespace Prism::Render
{
GraphicsPipelineState* PipelineStateCache::GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc)
{
	auto findIt = m_graphicsPipelineStates.find(desc);
	if (findIt != m_graphicsPipelineStates.end())
		return findIt->second;

	auto [it, success] = m_graphicsPipelineStates.emplace(desc, Private::CreatePipelineState(desc));
	PE_ASSERT(success, "Failed to create PSO");
	PE_RENDER_LOG(Info, "New PSO created {}", std::hash<GraphicsPipelineStateDesc>()(desc));
	return it->second;
}

ComputePipelineState* PipelineStateCache::GetOrCreatePipelineState(const ComputePipelineStateDesc& desc)
{
	auto findIt = m_computePipelineStates.find(desc);
	if (findIt != m_computePipelineStates.end())
		return findIt->second;

	auto [it, success] = m_computePipelineStates.emplace(desc, Private::CreatePipelineState(desc));
	PE_ASSERT(success, "Failed to create PSO");
	PE_RENDER_LOG(Info, "New PSO created {}", std::hash<ComputePipelineStateDesc>()(desc));
	return it->second;
}
}
