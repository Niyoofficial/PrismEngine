#include "pcpch.h"
#include "ShaderCache.h"

namespace Prism::Render
{
Shader* ShaderCache::GetOrCreateShader(const ShaderCreateInfo& createInfo)
{
	ShaderHash hash(createInfo);

	auto findIt = m_shaders.find(hash);
	if (findIt != m_shaders.end())
		return findIt->second.get();

	auto [it, success] = m_shaders.emplace(hash, CreateShader(createInfo));
	return it->second.get();
}
}
