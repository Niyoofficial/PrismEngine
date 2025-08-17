#pragma once
#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/Shader.h"
#include "Prism/Base/Base.h"

namespace Prism::Render
{
struct GraphicsPipelineStateDesc
{
	bool IsValid() const;
	bool operator==(const GraphicsPipelineStateDesc& other) const;

	ShaderDesc vs;
	ShaderDesc ps;
	BlendStateDesc blendState;
	RasterizerStateDesc rasterizerState;
	DepthStencilStateDesc depthStencilState;
	TopologyType primitiveTopologyType = TopologyType::TriangleList;
	/// 32-bit sample mask that determines which samples get updated
	/// in all the active render targets. A sample mask is always applied;
	/// it is independent of whether multisampling is enabled, and does not
	/// depend on whether an application uses multisample render targets.
	uint32_t sampleMask = 0xFFFFFFFF;
	SampleDesc sampleDesc;
};

struct ComputePipelineStateDesc
{
	bool IsValid() const;
	bool operator==(const ComputePipelineStateDesc& other) const;

	ShaderDesc cs;
};
}

template<>
struct std::hash<Prism::Render::GraphicsPipelineStateDesc>
{
	size_t operator()(const Prism::Render::GraphicsPipelineStateDesc& desc) const noexcept
	{
		using namespace Prism::Render;

#ifdef PE_BUILD_DEBUG
		static_assert(sizeof(desc) == 560,
					  "If new field was added, add it to the hash function and update this assert");
#endif

		return
			std::hash<ShaderDesc>()(desc.vs) ^
			std::hash<ShaderDesc>()(desc.ps) ^
			std::hash<BlendStateDesc>()(desc.blendState) ^
			std::hash<RasterizerStateDesc>()(desc.rasterizerState) ^
			std::hash<DepthStencilStateDesc>()(desc.depthStencilState) ^
			std::hash<uint32_t>()(desc.sampleMask);
	}
};

template<>
struct std::hash<Prism::Render::ComputePipelineStateDesc>
{
	size_t operator()(const Prism::Render::ComputePipelineStateDesc& desc) const noexcept
	{
		using namespace Prism::Render;

#ifdef PE_BUILD_DEBUG
		static_assert(sizeof(desc) == 88,
					  "If new field was added, add it to the hash function and update this assert");
#endif

		return std::hash<ShaderDesc>()(desc.cs);
	}
};
