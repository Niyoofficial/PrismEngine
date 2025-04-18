#pragma once
#include "xxhash.h"
#include "Prism/Render/Shader.h"

namespace Prism::Render
{
class ShaderCompiler
{
public:
	virtual ~ShaderCompiler() = default;

	virtual void CompileShader(const ShaderDesc& desc) = 0;
	virtual XXH64_hash_t GetShaderCodeHash(const ShaderDesc& desc) = 0;
	virtual void RecompileCachedShaders() = 0;
};
}
