#include "pcpch.h"
#include "PipelineState.h"

#include "Prism/Render/RenderDevice.h"
#include "Prism/Render/PipelineStateCache.h"

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

bool ComputePipelineStateDesc::operator==(const ComputePipelineStateDesc& other) const
{
	return other.cs == cs;
}

ComputePipelineState* ComputePipelineState::Create(const ComputePipelineStateDesc& desc)
{
	return RenderDevice::Get().GetPipelineStateCache().GetOrCreatePipelineState(desc);
}

ComputePipelineState::ComputePipelineState(const ComputePipelineStateDesc& desc)
	: m_desc(desc)
{
}
}
