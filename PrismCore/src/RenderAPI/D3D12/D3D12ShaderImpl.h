#pragma once
#include "Prism/Render/Shader.h"
#include "RenderAPI/D3D12/D3D12ShaderCompiler.h"

namespace Prism::Render::D3D12
{
class D3D12Shader : public Shader
{
public:
	explicit D3D12Shader(const ShaderCreateInfo& createInfo);

	const D3D12ShaderCompilerOutput& GetCompilerOutput() const { return m_compilerOutput; }

private:
	D3D12ShaderCompilerOutput m_compilerOutput;
};
}
