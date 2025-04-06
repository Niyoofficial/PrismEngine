#pragma once
#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/Shader.h"
#include "Prism/Base/Base.h"

namespace Prism::Render
{
struct GraphicsPipelineStateDesc
{
	bool operator==(const GraphicsPipelineStateDesc& other) const;

	Shader* vs = nullptr;
	Shader* ps = nullptr;
	BlendStateDesc blendState;
	RasterizerStateDesc rasterizerState;
	DepthStencilStateDesc depthStencilState;
	TopologyType primitiveTopologyType = TopologyType::TriangleList;
	int32_t numRenderTargets = 1;
	std::array<TextureFormat, 8> renderTargetFormats = {TextureFormat::Unknown};
	TextureFormat depthStencilFormat;
	/// 32-bit sample mask that determines which samples get updated
	/// in all the active render targets. A sample mask is always applied;
	/// it is independent of whether multisampling is enabled, and does not
	/// depend on whether an application uses multisample render targets.
	uint32_t sampleMask = 0xFFFFFFFF;
	SampleDesc sampleDesc;
};

class GraphicsPipelineState : public RefCounted
{
public:
	static GraphicsPipelineState* Create(const GraphicsPipelineStateDesc& desc);

	const GraphicsPipelineStateDesc& GetDesc() const { return m_desc; }

protected:
	explicit GraphicsPipelineState(const GraphicsPipelineStateDesc& desc);

protected:
	GraphicsPipelineStateDesc m_desc = {};
};

struct ComputePipelineStateDesc
{
	bool operator==(const ComputePipelineStateDesc& other) const;

	Shader* cs = nullptr;
};

class ComputePipelineState : public RefCounted
{
public:
	static ComputePipelineState* Create(const ComputePipelineStateDesc& desc);

	const ComputePipelineStateDesc& GetDesc() const { return m_desc; }

protected:
	explicit ComputePipelineState(const ComputePipelineStateDesc& desc);

protected:
	ComputePipelineStateDesc m_desc = {};
};
}

template<>
struct std::hash<Prism::Render::GraphicsPipelineStateDesc>
{
	size_t operator()(const Prism::Render::GraphicsPipelineStateDesc& desc) const noexcept
	{
		using namespace Prism::Render;

		static_assert(sizeof(desc) == 440,
					  "If new field was added, add it to the hash function and update this assert");

		return
			std::hash<Shader*>()(desc.vs) ^
			std::hash<Shader*>()(desc.ps) ^
			std::hash<BlendStateDesc>()(desc.blendState) ^
			std::hash<RasterizerStateDesc>()(desc.rasterizerState) ^
			std::hash<DepthStencilStateDesc>()(desc.depthStencilState) ^
			std::hash<TopologyType>()(desc.primitiveTopologyType) ^
			std::hash<int32_t>()(desc.numRenderTargets) ^
			std::hash<std::array<TextureFormat, 8>>()(desc.renderTargetFormats) ^
			std::hash<TextureFormat>()(desc.depthStencilFormat);
	}
};

template<>
struct std::hash<Prism::Render::ComputePipelineStateDesc>
{
	size_t operator()(const Prism::Render::ComputePipelineStateDesc& desc) const noexcept
	{
		using namespace Prism::Render;

		static_assert(sizeof(desc) == 8,
					  "If new field was added, add it to the hash function and update this assert");

		return std::hash<Shader*>()(desc.cs);
	}
};
