#pragma once
#include "Prism-Core/Render/Shader.h"

namespace Prism::Render
{
class ShaderCache final
{
public:
	Shader* GetOrCreateShader(const ShaderCreateInfo& createInfo);

private:
	std::unordered_map<ShaderHash, std::unique_ptr<Shader>> m_shaders;
};
}
