#pragma once

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include "spirv_reflect.h"

namespace Prism::Render::Vulkan
{
struct VulkanShaderReflection
{
	SpvReflectShaderModule module{};

	VulkanShaderReflection() = default;

	~VulkanShaderReflection();

	VulkanShaderReflection(const VulkanShaderReflection&) = delete;
	VulkanShaderReflection& operator=(const VulkanShaderReflection&) = delete;

	VulkanShaderReflection(VulkanShaderReflection&& other) noexcept;
	VulkanShaderReflection& operator=(VulkanShaderReflection&& other) noexcept;
};
} // namespace Prism::Render::Vulkan
