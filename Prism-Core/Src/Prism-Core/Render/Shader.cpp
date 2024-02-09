#include "pcpch.h"
#include "Shader.h"

#include "Prism-Core/Render/RenderAPI.h"
#include "Prism-Core/Render/Renderer.h"

namespace Prism::Render
{
Shader* Shader::Create(const ShaderCreateInfo& createInfo)
{
	return Renderer::Get().GetRenderAPI()->GetShaderCache().GetOrCreateShader(createInfo);
}

ShaderHash::ShaderHash(const ShaderCreateInfo& createInfo)
	: Hash(createInfo)
{
}
}
