#include "pcpch.h"
#include "PipelineState.h"

#include "Prism/Render/RenderDevice.h"

namespace Prism::Render
{
bool GraphicsPipelineStateDesc::IsValid() const
{
	return vs.IsValid() && ps.IsValid();
}

bool GraphicsPipelineStateDesc::operator==(const GraphicsPipelineStateDesc& other) const
{
	return
		other.vs == vs &&
		other.ps == ps &&
		other.blendState == blendState &&
		other.rasterizerState == rasterizerState &&
		other.depthStencilState == depthStencilState &&
		other.primitiveTopologyType == primitiveTopologyType &&
		other.sampleMask == sampleMask &&
		other.sampleDesc == sampleDesc;
}

bool ComputePipelineStateDesc::IsValid() const
{
	return cs.IsValid();
}

bool ComputePipelineStateDesc::operator==(const ComputePipelineStateDesc& other) const
{
	return other.cs == cs;
}
}
