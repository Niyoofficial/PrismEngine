#pragma once
#include "Prism/Render/PipelineState.h"

namespace Prism::Render
{
struct PipelineStateCache final
{
public:
	PipelineStateCache() = default;

	GraphicsPipelineState* GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc);
	ComputePipelineState* GetOrCreatePipelineState(const ComputePipelineStateDesc& desc);

private:
	std::unordered_map<GraphicsPipelineStateDesc, Ref<GraphicsPipelineState>> m_graphicsPipelineStates;
	std::unordered_map<ComputePipelineStateDesc, Ref<ComputePipelineState>> m_computePipelineStates;
};
}
