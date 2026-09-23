#pragma once

#include "VulkanWindowsCOM.h"
#include <dxcapi.h>
#include "Prism/Render/ShaderCompiler.h"
#include "VulkanShaderReflection.h"

namespace Prism::Render::Vulkan
{
struct VulkanShaderCompilerOutput
{
	std::vector<uint32_t> spirv{};

	VulkanShaderReflection reflection{};
};

class VulkanShaderCompiler : public ShaderCompiler
{
public:
	VulkanShaderCompiler();

	const VulkanShaderCompilerOutput& GetOrCreateShader(const ShaderDesc& desc);

	void CompileShader(const ShaderDesc& desc) override;
	[[nodiscard]] uint64_t GetShaderCodeHash(const ShaderDesc& desc) override;
	void RecompileCachedShaders() override;

private:
	[[nodiscard]] std::wstring GetStringForShader(ShaderType shaderType) const;
	[[nodiscard]] std::wstring GetTargetStringForShader(ShaderType shaderType, int32_t major, int32_t minor) const;

	void RemoveShaderCache(uint64_t shaderHash);

	ComPtr<IDxcUtils> m_dxcUtils;
	ComPtr<IDxcCompiler3> m_dxcCompiler;
	ComPtr<IDxcIncludeHandler> m_dxcIncludeHandler;

	struct CompiledShaderBackend
	{
		uint64_t hash{};
		VulkanShaderCompilerOutput output{};
	};

	std::unordered_map<ShaderDesc, CompiledShaderBackend> m_shaderCache;
};
} // namespace Prism::Render::Vulkan
