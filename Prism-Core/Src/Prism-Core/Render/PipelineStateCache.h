#pragma once
#include "Prism-Core/Render/GraphicsPipelineState.h"

namespace Prism::Render
{
class PipelineStateCache final
{
public:
	PipelineStateCache() = default;

	GraphicsPipelineState* GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc);

private:
	std::unordered_map<PipelineStateHash, std::unique_ptr<GraphicsPipelineState>> m_pipelineStates;
};
}
