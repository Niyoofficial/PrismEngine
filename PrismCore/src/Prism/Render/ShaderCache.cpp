#include "pcpch.h"
#include "ShaderCache.h"

#include "Prism/Render/RenderResourceCreation.h"

namespace Prism::Render
{
Shader* ShaderCache::GetOrCreateShader(const ShaderCreateInfo& createInfo)
{
	auto findIt = m_shaders.find(createInfo);
	if (findIt != m_shaders.end())
		return findIt->second;

	auto [it, success] = m_shaders.emplace(createInfo, Private::CreateShader(createInfo));
	PE_ASSERT(success, "Failed to create shader! Filename: {}, Entryname: {}", createInfo.filepath, createInfo.entryName);
	PE_RENDER_LOG(Info, "New shader created. Hash: {}, Filename: {}, Entryname: {}", std::hash<ShaderCreateInfo>()(createInfo),
				  createInfo.filepath, createInfo.entryName);
	return it->second;
}
}
