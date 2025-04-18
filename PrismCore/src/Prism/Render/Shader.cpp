#include "pcpch.h"
#include "Shader.h"

#include "Prism/Render/RenderDevice.h"

namespace Prism::Render
{
bool ShaderDesc::IsValid() const
{
	return !filepath.empty() && !entryName.empty();
}

bool ShaderDesc::operator==(const ShaderDesc& other) const
{
	return
		other.filepath == filepath &&
		other.entryName == entryName &&
		other.shaderType == shaderType;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const ShaderDesc& shaderCreateInfo)
{
	out << YAML::BeginMap;
	out << YAML::Key << "filepath" << YAML::Value << WStringToString(shaderCreateInfo.filepath);
	out << YAML::Key << "entryName" << YAML::Value << WStringToString(shaderCreateInfo.entryName);
	out << YAML::Key << "shaderType" << YAML::Value << (int32_t)shaderCreateInfo.shaderType;
	out << YAML::EndMap;
	return out;
}
}
