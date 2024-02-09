#pragma once
#include "Prism-Core/Utilities/HashUtils.h"

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
	std::wstring filepath;
	std::wstring entryName;
	ShaderType shaderType;
};

class Shader
{
public:
	static Shader* Create(const ShaderCreateInfo& createInfo);
};

struct ShaderHash : Hash<HashSize::Bit128>
{
	explicit ShaderHash(const ShaderCreateInfo& createInfo);
};
}

template<>
struct std::hash<Prism::Render::ShaderHash>
{
	size_t operator()(const Prism::Render::ShaderHash& hash) const noexcept
	{
		return hash.hashValue.low ^ hash.hashValue.high;
	}
};
