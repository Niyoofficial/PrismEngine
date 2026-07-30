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

SpvReflectResult Prism::Render::Vulkan::VulkanShaderReflection::Create(const size_t size, const void* data)
{
	const auto result = spvReflectCreateShaderModule(size, data, &m_module);

	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		return result;
	}

	uint32_t inputCount = 0;
	spvReflectEnumerateInputVariables(&m_module, &inputCount, nullptr);

	m_inputVariables.resize(inputCount);
	spvReflectEnumerateInputVariables(&m_module, &inputCount, m_inputVariables.data());

	std::ranges::sort(m_inputVariables, [](const auto* a, const auto* b) { return a->location < b->location; });

	uint32_t outputCount = 0;
	spvReflectEnumerateOutputVariables(&m_module, &outputCount, nullptr);

	m_outputVariables.resize(outputCount);
	spvReflectEnumerateOutputVariables(&m_module, &outputCount, m_outputVariables.data());

	std::ranges::sort(m_outputVariables, [](const auto* a, const auto* b) { return a->location < b->location; });

	return result;
}

Prism::Render::ShaderType Prism::Render::Vulkan::VulkanShaderReflection::GetShaderType() const
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

const SpvReflectBlockVariable& Prism::Render::Vulkan::VulkanShaderReflection::GetPushConstantBlock(const uint32_t index) const
{
	PE_ASSERT(index < m_module.push_constant_block_count);

	return m_module.push_constant_blocks[index];
}

uint32_t Prism::Render::Vulkan::VulkanShaderReflection::GetInputVariableCount() const
{
	return static_cast<uint32_t>(m_inputVariables.size());
}

const SpvReflectInterfaceVariable& Prism::Render::Vulkan::VulkanShaderReflection::GetInputVariable(const uint32_t index) const
{
	PE_ASSERT(index < m_inputVariables.size());

	return *m_inputVariables[index];
}

const std::vector<SpvReflectInterfaceVariable*>& Prism::Render::Vulkan::VulkanShaderReflection::GetInputVariables() const
{
	return m_inputVariables;
}

const std::vector<SpvReflectInterfaceVariable*>& Prism::Render::Vulkan::VulkanShaderReflection::GetOutputVariables() const
{
	return m_outputVariables;
}

const SpvReflectDescriptorSet* Prism::Render::Vulkan::VulkanShaderReflection::FindDescriptorSet(const uint32_t set) const
{
	for (uint32_t i = 0; i < m_module.descriptor_set_count; ++i)
	{
		if (m_module.descriptor_sets[i].set == set)
		{
			return &m_module.descriptor_sets[i];
		}
	}

	return nullptr;
}
