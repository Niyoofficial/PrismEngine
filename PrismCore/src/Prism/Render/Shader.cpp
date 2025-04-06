#include "pcpch.h"
#include "Shader.h"

#include "Prism/Render/RenderDevice.h"

namespace Prism::Render
{
bool ShaderCreateInfo::operator==(const ShaderCreateInfo& other) const
{
	return
		other.filepath == filepath &&
		other.entryName == entryName &&
		other.shaderType == shaderType;
}

Shader* Shader::Create(const ShaderCreateInfo& createInfo)
{
	return RenderDevice::Get().GetShaderCache().GetOrCreateShader(createInfo);
}

Shader::Shader(const ShaderCreateInfo& createInfo)
	: m_createInfo(createInfo)
{
}
}
