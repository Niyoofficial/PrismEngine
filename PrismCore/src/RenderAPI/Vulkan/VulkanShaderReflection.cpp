#pragma once

#include "VulkanShaderReflection.h"

#include "Prism/Base/Assert.h"
#include "Prism/Render/Shader.h"

Prism::Render::Vulkan::VulkanShaderReflection::~VulkanShaderReflection() { spvReflectDestroyShaderModule(&m_module); }

Prism::Render::Vulkan::VulkanShaderReflection::VulkanShaderReflection(VulkanShaderReflection&& other) noexcept
{
	m_module = other.m_module;
	std::memset(&other.m_module, 0, sizeof(other.m_module));
}

Prism::Render::Vulkan::VulkanShaderReflection&
Prism::Render::Vulkan::VulkanShaderReflection::operator=(VulkanShaderReflection&& other) noexcept
{
	if (this != &other)
	{
		spvReflectDestroyShaderModule(&m_module);

		m_module = other.m_module;
		std::memset(&other.m_module, 0, sizeof(other.m_module));
	}

	return *this;
}

SpvReflectResult Prism::Render::Vulkan::VulkanShaderReflection::Create(size_t size, const void* data)
{
	return spvReflectCreateShaderModule(size, data, &m_module);
}

Prism::Render::ShaderType Prism::Render::Vulkan::VulkanShaderReflection::GetStage() const
{
	switch (m_module.shader_stage)
	{
	case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
		return ShaderType::VS;
	case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
		return ShaderType::PS;
	case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
		return ShaderType::CS;
	case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
		return ShaderType::GS;
	case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return ShaderType::HS;
	case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return ShaderType::DS;
	default:
		PE_ASSERT_NO_ENTRY();
		return ShaderType::VS;
	}
}

uint32_t Prism::Render::Vulkan::VulkanShaderReflection::GetDescriptorSetCount() const { return m_module.descriptor_set_count; }

const SpvReflectDescriptorSet& Prism::Render::Vulkan::VulkanShaderReflection::GetDescriptorSet(const uint32_t index) const
{
	PE_ASSERT(index < m_module.descriptor_set_count);

	return m_module.descriptor_sets[index];
}

uint32_t Prism::Render::Vulkan::VulkanShaderReflection::GetPushConstantBlockCount() const
{
	return m_module.push_constant_block_count;
}

const SpvReflectBlockVariable& Prism::Render::Vulkan::VulkanShaderReflection::GetPushConstantBlock(uint32_t index) const
{
	PE_ASSERT(index < m_module.push_constant_block_count);

	return m_module.push_constant_blocks[index];
}
