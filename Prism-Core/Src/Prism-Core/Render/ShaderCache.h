#pragma once
#include "Prism-Core/Render/Shader.h"

namespace Prism::Render
{
struct ShaderCache final
{
public:
	Shader* GetOrCreateShader(const ShaderCreateInfo& createInfo);

private:
	std::unordered_map<ShaderCreateInfo, Ref<Shader>> m_shaders;
};
}
