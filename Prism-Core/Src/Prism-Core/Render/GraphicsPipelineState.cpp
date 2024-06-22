#include "pcpch.h"
#include "GraphicsPipelineState.h"

#include "Prism-Core/Render/RenderDevice.h"
#include "Prism-Core/Render/PipelineStateCache.h"

namespace Prism::Render
{
bool GraphicsPipelineStateDesc::operator==(const GraphicsPipelineStateDesc& other) const
{
	return
		other.vs == vs &&
		other.ps == ps &&
		other.blendState == blendState &&
		other.rasterizerState == rasterizerState &&
		other.depthStencilState == depthStencilState &&
		other.primitiveTopologyType == primitiveTopologyType &&
		other.numRenderTargets == numRenderTargets &&
		other.renderTargetFormats == renderTargetFormats &&
		other.depthStencilFormat == depthStencilFormat &&
		other.sampleMask == sampleMask &&
		other.sampleDesc == sampleDesc;
}

GraphicsPipelineState* GraphicsPipelineState::Create(const GraphicsPipelineStateDesc& desc)
{
	return RenderDevice::Get().GetPipelineStateCache().GetOrCreatePipelineState(desc);
}

GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
	: m_desc(desc)
{
}
}
