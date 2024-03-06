#include "pcpch.h"
#include "D3D12ShaderImpl.h"

#include "RenderAPI/D3D12/D3D12RenderAPI.h"

Prism::Render::D3D12::D3D12Shader::D3D12Shader(const ShaderCreateInfo& createInfo)
{
	m_compilerOutput = D3D12RenderAPI::Get()->GetShaderCompiler().CompileShader(createInfo);
}
