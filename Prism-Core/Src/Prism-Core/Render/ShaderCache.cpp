#include "pcpch.h"
#include "ShaderCache.h"

#include "Prism-Core/Render/RenderResourceCreation.h"

namespace Prism::Render
{
Shader* ShaderCache::GetOrCreateShader(const ShaderCreateInfo& createInfo)
{
	auto findIt = m_shaders.find(createInfo);
	if (findIt != m_shaders.end())
		return findIt->second;

	auto [it, success] = m_shaders.emplace(createInfo, Private::CreateShader(createInfo));
	return it->second;
}
}
