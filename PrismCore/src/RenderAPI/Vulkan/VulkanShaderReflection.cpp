#pragma once

#include "VulkanShaderReflection.h"

Prism::Render::Vulkan::VulkanShaderReflection::~VulkanShaderReflection() { spvReflectDestroyShaderModule(&module); }

Prism::Render::Vulkan::VulkanShaderReflection::VulkanShaderReflection(VulkanShaderReflection&& other) noexcept
{
	module = other.module;
	std::memset(&other.module, 0, sizeof(other.module));
}

Prism::Render::Vulkan::VulkanShaderReflection&
Prism::Render::Vulkan::VulkanShaderReflection::operator=(VulkanShaderReflection&& other) noexcept
{
	if (this != &other)
	{
		spvReflectDestroyShaderModule(&module);

		module = other.module;
		std::memset(&other.module, 0, sizeof(other.module));
	}

	return *this;
}
