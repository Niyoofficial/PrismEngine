#pragma once
#include "Prism-Core/Render/RenderTypes.h"
#include "Prism-Core/Render/Shader.h"

namespace Prism::Render
{
struct GraphicsPipelineStateDesc
{
	Shader* vs = nullptr;
	Shader* ps = nullptr;
	BlendStateDesc blendState;
	RasterizerStateDesc rasterizerState;
	DepthStencilStateDesc depthStencilState;
	std::vector<LayoutElement> inputLayout;
	TopologyType primitiveTopologyType;
	int32_t numRenderTargets = 0;
	TextureFormat renderTargetFormats[8] = {};
	TextureFormat depthStencilFormat;
	/// 32-bit sample mask that determines which samples get updated
	/// in all the active render targets. A sample mask is always applied;
	/// it is independent of whether multisampling is enabled, and does not
	/// depend on whether an application uses multisample render targets.
	uint32_t sampleMask = 0xFFFFFFFF;
	SampleDesc sampleDesc;
};

class GraphicsPipelineState
{
public:
	static GraphicsPipelineState* Create(const GraphicsPipelineStateDesc& desc);

	const GraphicsPipelineStateDesc& GetDesc() const { return m_desc; }

protected:
	explicit GraphicsPipelineState(const GraphicsPipelineStateDesc& desc);

protected:
	GraphicsPipelineStateDesc m_desc = {};
};

struct PipelineStateHash : Hash<HashSize::Bit128>
{
	explicit PipelineStateHash(const GraphicsPipelineStateDesc& desc);
};
}

template<>
struct std::hash<Prism::Render::PipelineStateHash>
{
	size_t operator()(const Prism::Render::PipelineStateHash& hash) const noexcept
	{
		return hash.hashValue.low ^ hash.hashValue.high;
	}
};
