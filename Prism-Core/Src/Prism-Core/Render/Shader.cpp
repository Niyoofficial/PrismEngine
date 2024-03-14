#include "pcpch.h"
#include "Shader.h"

#include "Prism-Core/Render/RenderDevice.h"

namespace Prism::Render
{
Shader* Shader::Create(const ShaderCreateInfo& createInfo)
{
	return RenderDevice::Get().GetShaderCache().GetOrCreateShader(createInfo);
}

ShaderHash::ShaderHash(const ShaderCreateInfo& createInfo)
	: Hash(createInfo)
{
}
}
