#pragma once

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include <spirv_reflect.h>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/Shader.h"

namespace Prism::Render
{
enum class VertexAttribute;
}

namespace Prism::Render::Vulkan
{
struct VulkanShaderVertexInput
{
	VertexAttribute attribute{};
	uint32_t location = 0;
	VkFormat format = VK_FORMAT_UNDEFINED;
};

struct VulkanShaderReflection
{
	VulkanShaderReflection() = default;

	~VulkanShaderReflection();

	VulkanShaderReflection(const VulkanShaderReflection&) = delete;
	VulkanShaderReflection& operator=(const VulkanShaderReflection&) = delete;

	VulkanShaderReflection(VulkanShaderReflection&& other) noexcept;
	VulkanShaderReflection& operator=(VulkanShaderReflection&& other) noexcept;

	[[nodiscard]] SpvReflectResult Create(size_t size, const void* data);

	[[nodiscard]] ShaderType GetShaderType() const;

	[[nodiscard]] uint32_t GetDescriptorSetCount() const;

	[[nodiscard]] const SpvReflectDescriptorSet& GetDescriptorSet(uint32_t index) const;

	[[nodiscard]] uint32_t GetPushConstantBlockCount() const;

	[[nodiscard]] const SpvReflectBlockVariable& GetPushConstantBlock(uint32_t index) const;

	[[nodiscard]] uint32_t GetInputVariableCount() const;

	[[nodiscard]] const SpvReflectInterfaceVariable& GetInputVariable(uint32_t index) const;

	[[nodiscard]] const std::vector<SpvReflectInterfaceVariable*>& GetInputVariables() const;

	[[nodiscard]] const std::vector<SpvReflectInterfaceVariable*>& GetOutputVariables() const;

	[[nodiscard]] const SpvReflectDescriptorSet* FindDescriptorSet(uint32_t set) const;

	[[nodiscard]] const std::vector<VulkanShaderVertexInput>& GetVertexInputs() const;

private:
	static VertexAttribute GetVertexAttribute(const SpvReflectInterfaceVariable& variable);

	SpvReflectShaderModule m_module{};
	std::vector<SpvReflectInterfaceVariable*> m_inputVariables;
	std::vector<SpvReflectInterfaceVariable*> m_outputVariables;
	std::vector<VulkanShaderVertexInput> m_vertexInputs;
};
} // namespace Prism::Render::Vulkan
