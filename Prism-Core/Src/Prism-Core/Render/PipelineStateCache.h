#pragma once
#include "Prism-Core/Render/GraphicsPipelineState.h"

namespace Prism::Render
{
struct PipelineStateCache final
{
public:
	PipelineStateCache() = default;

	GraphicsPipelineState* GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc);

private:
	std::unordered_map<GraphicsPipelineStateDesc, Ref<GraphicsPipelineState>> m_pipelineStates;
};
}
