#include "pcpch.h"
#include "RenderTypes.h"

bool Prism::Render::RenderTargetBlendDesc::operator==(const RenderTargetBlendDesc& other) const
{
	return
		other.blendEnable == blendEnable &&
		other.logicOperationEnable == logicOperationEnable &&
		other.srcBlend == srcBlend &&
		other.destBlend == destBlend &&
		other.blendOperation == blendOperation &&
		other.srcBlendAlpha == srcBlendAlpha &&
		other.destBlendAlpha == destBlendAlpha &&
		other.blendOperationAlpha == blendOperationAlpha &&
		other.logicOperation == logicOperation &&
		other.renderTargetWriteMask == renderTargetWriteMask;
}

bool Prism::Render::BlendStateDesc::operator==(const BlendStateDesc& other) const
{
	return
		other.alphaToCoverageEnable == alphaToCoverageEnable &&
		other.independentBlendEnable == independentBlendEnable &&
		other.renderTargetBlendDesc == renderTargetBlendDesc;
}

bool Prism::Render::RasterizerStateDesc::operator==(const RasterizerStateDesc& other) const
{
	return
		other.fillMode == fillMode &&
		other.cullMode == cullMode &&
		other.frontCounterClockwise == frontCounterClockwise &&
		other.depthBias == depthBias &&
		std::abs(other.depthBiasClamp - depthBiasClamp) < std::numeric_limits<float>::epsilon() &&
		std::abs(other.slopeScaledDepthBias - slopeScaledDepthBias) < std::numeric_limits<float>::epsilon() &&
		other.depthClipEnable == depthClipEnable &&
		other.antialiasedLineEnable == antialiasedLineEnable &&
		other.forcedSampleCount == forcedSampleCount;
}

bool Prism::Render::DepthStencilOperationDesc::operator==(const DepthStencilOperationDesc& other) const
{
	return
		other.stencilFail == stencilFail &&
		other.stencilDepthFail == stencilDepthFail &&
		other.stencilPass == stencilPass &&
		other.stencilFunction == stencilFunction;
}

bool Prism::Render::DepthStencilStateDesc::operator==(const DepthStencilStateDesc& other) const
{
	return
		other.depthEnable == depthEnable &&
		other.depthWriteEnable == depthWriteEnable &&
		other.depthFunc == depthFunc &&
		other.stencilEnable == stencilEnable &&
		other.stencilReadMask == stencilReadMask &&
		other.stencilWriteMask == stencilWriteMask &&
		other.frontFace == frontFace &&
		other.backFace == backFace;
}

bool Prism::Render::SampleDesc::operator==(const SampleDesc& other) const
{
	return
		other.count == count &&
		other.quality == quality;
}
