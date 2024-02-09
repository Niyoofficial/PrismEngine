#pragma once
#include "Prism-Core/Render/Shader.h"
#include "RenderAPI/D3D12/D3D12ShaderCompiler.h"

namespace Prism::D3D12
{
class D3D12Shader : public Render::Shader
{
public:
	explicit D3D12Shader(const Render::ShaderCreateInfo& createInfo);

	const D3D12ShaderCompilerOutput& GetCompilerOutput() const { return m_compilerOutput; }

private:
	D3D12ShaderCompilerOutput m_compilerOutput;
};
}
