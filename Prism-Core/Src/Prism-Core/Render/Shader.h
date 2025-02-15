﻿#pragma once

namespace Prism::Render
{
enum class ShaderType
{
	VertexShader,
	PixelShader,
	DomainShader,
	HullShader,
	GeometryShader,
	ComputeShader,

	VS = VertexShader,
	PS = PixelShader,
	DS = DomainShader,
	HS = HullShader,
	GS = GeometryShader,
	CS = ComputeShader
};

struct ShaderCreateInfo
{
	bool operator==(const ShaderCreateInfo& other) const;

	std::wstring filepath;
	std::wstring entryName;
	ShaderType shaderType;
};

class Shader : public RefCounted
{
public:
	static Shader* Create(const ShaderCreateInfo& createInfo);
};
}

template<>
struct std::hash<Prism::Render::ShaderCreateInfo>
{
	size_t operator()(const Prism::Render::ShaderCreateInfo& shaderCreateInfo) const noexcept
	{
		return
			std::hash<std::wstring>()(shaderCreateInfo.filepath) ^
			std::hash<std::wstring>()(shaderCreateInfo.entryName) ^
			std::hash<Prism::Render::ShaderType>()(shaderCreateInfo.shaderType);
	}
};
