#pragma once

#include "RenderAPI/D3D12/D3D12Base.h"

#include "Prism/Render/Shader.h"

#include "dxcapi.h"
#include <d3d12shader.h>

namespace Prism::Render::D3D12
{
struct D3D12ShaderCompilerOutput
{
	ComPtr<IDxcBlob> bytecode;
	ComPtr<ID3D12ShaderReflection> reflection;
};

class D3D12ShaderCompiler
{
public:
	D3D12ShaderCompiler();

	D3D12ShaderCompilerOutput CompileShader(const ShaderCreateInfo& createInfo);

private:
	std::wstring GetStringForShader(ShaderType shaderType);
	std::wstring GetTargetStringForShader(ShaderType shaderType, int32_t major, int32_t minor);

private:
	ComPtr<IDxcUtils> m_dxcUtils;
	ComPtr<IDxcCompiler3> m_dxcCompiler;
	ComPtr<IDxcIncludeHandler> m_dxcIncludeHandler;
};
}
