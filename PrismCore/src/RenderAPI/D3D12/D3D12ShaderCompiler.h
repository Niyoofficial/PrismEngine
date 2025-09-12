#pragma once

#include "RenderAPI/D3D12/D3D12Base.h"

#include "Prism/Render/Shader.h"

#include "dxcapi.h"
#include <d3d12shader.h>

#include "Prism/Render/ShaderCompiler.h"

namespace Prism::Render::D3D12
{
class D3D12Shader;

struct D3D12ShaderCompilerOutput
{
	ComPtr<IDxcBlob> bytecode;
	ComPtr<ID3D12ShaderReflection> reflection;
};

class D3D12ShaderCompiler : public ShaderCompiler
{
public:
	D3D12ShaderCompiler();

	D3D12ShaderCompilerOutput GetOrCreateShader(const ShaderDesc& desc);

	virtual void CompileShader(const ShaderDesc& desc) override;
	virtual XXH64_hash_t GetShaderCodeHash(const ShaderDesc& desc) override;

	virtual void RecompileCachedShaders() override;

private:
	std::wstring GetStringForShader(ShaderType shaderType) const;
	std::wstring GetTargetStringForShader(ShaderType shaderType, int32_t major, int32_t minor) const;

	void RemoveShaderCache(XXH64_hash_t shaderHash);

private:
	ComPtr<IDxcUtils> m_dxcUtils;
	ComPtr<IDxcCompiler3> m_dxcCompiler;
	ComPtr<IDxcIncludeHandler> m_dxcIncludeHandler;

	struct CompiledShaderBackend
	{
		XXH64_hash_t hash;
		D3D12ShaderCompilerOutput output;
	};

	std::unordered_map<ShaderDesc, CompiledShaderBackend> m_shaderCache;
};
}
