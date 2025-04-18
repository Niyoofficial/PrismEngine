#pragma once

#include "yaml-cpp/yaml.h"

namespace Prism::Render
{
enum class ShaderType : uint8_t
{
	VertexShader,
	PixelShader,
	DomainShader,
	HullShader,
	GeometryShader,
	MeshShader,
	AmplificationShader,
	ComputeShader,

	VS = VertexShader,
	PS = PixelShader,
	DS = DomainShader,
	HS = HullShader,
	GS = GeometryShader,
	MS = MeshShader,
	AS = AmplificationShader,
	CS = ComputeShader
};

struct ShaderDesc
{
	bool IsValid() const;
	bool operator==(const ShaderDesc& other) const;

	std::wstring filepath;
	std::wstring entryName;
	ShaderType shaderType;
};

YAML::Emitter& operator<<(YAML::Emitter& out, const ShaderDesc& shaderCreateInfo);
}

template<>
struct std::hash<Prism::Render::ShaderDesc>
{
	size_t operator()(const Prism::Render::ShaderDesc& shaderCreateInfo) const noexcept
	{
		return
			std::hash<std::wstring>()(shaderCreateInfo.filepath) ^
			std::hash<std::wstring>()(shaderCreateInfo.entryName) ^
			std::hash<Prism::Render::ShaderType>()(shaderCreateInfo.shaderType);
	}
};
