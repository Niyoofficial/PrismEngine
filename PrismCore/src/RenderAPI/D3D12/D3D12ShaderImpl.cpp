#include "pcpch.h"
#include "D3D12ShaderImpl.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"

Prism::Render::D3D12::D3D12Shader::D3D12Shader(const ShaderCreateInfo& createInfo)
	: Shader(createInfo)
{
	m_compilerOutput = D3D12RenderDevice::Get().GetShaderCompiler().CompileShader(createInfo);
}
