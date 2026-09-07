#pragma once

#include "VulkanShaderReflection.h"

#include "Prism/Base/Assert.h"
#include "Prism/Render/Shader.h"
#include "VulkanTypeConversions.h"
#include "Prism/Render/VertexBufferCache.h"

Prism::Render::Vulkan::VulkanShaderReflection::~VulkanShaderReflection() { spvReflectDestroyShaderModule(&m_module); }

Prism::Render::Vulkan::VulkanShaderReflection::VulkanShaderReflection(VulkanShaderReflection&& other) noexcept
{
	m_module = other.m_module;
	std::memset(&other.m_module, 0, sizeof(other.m_module));

	m_inputVariables = std::move(other.m_inputVariables);
	m_outputVariables = std::move(other.m_outputVariables);
	m_vertexInputs = std::move(other.m_vertexInputs);
}

Prism::Render::Vulkan::VulkanShaderReflection&
Prism::Render::Vulkan::VulkanShaderReflection::operator=(VulkanShaderReflection&& other) noexcept
{
	if (this != &other)
	{
		spvReflectDestroyShaderModule(&m_module);

		m_module = other.m_module;
		std::memset(&other.m_module, 0, sizeof(other.m_module));

		m_inputVariables = std::move(other.m_inputVariables);
		m_outputVariables = std::move(other.m_outputVariables);
		m_vertexInputs = std::move(other.m_vertexInputs);
	}

	return *this;
}

SpvReflectResult Prism::Render::Vulkan::VulkanShaderReflection::Create(const size_t size, const void* data)
{
	spvReflectDestroyShaderModule(&m_module);
	std::memset(&m_module, 0, sizeof(m_module));

	m_inputVariables.clear();
	m_outputVariables.clear();
	m_vertexInputs.clear();

	const auto result = spvReflectCreateShaderModule(size, data, &m_module);

	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		return result;
	}

	uint32_t inputCount = 0;
	PE_ASSERT(spvReflectEnumerateInputVariables(&m_module, &inputCount, nullptr) == SPV_REFLECT_RESULT_SUCCESS);

	m_inputVariables.resize(inputCount);
	PE_ASSERT(spvReflectEnumerateInputVariables(&m_module, &inputCount, m_inputVariables.data()) == SPV_REFLECT_RESULT_SUCCESS);

	std::ranges::sort(m_inputVariables, [](const auto* a, const auto* b) { return a->location < b->location; });

	uint32_t outputCount = 0;
	PE_ASSERT(spvReflectEnumerateOutputVariables(&m_module, &outputCount, nullptr) == SPV_REFLECT_RESULT_SUCCESS);

	m_outputVariables.resize(outputCount);
	PE_ASSERT(spvReflectEnumerateOutputVariables(&m_module, &outputCount, m_outputVariables.data()) ==
	          SPV_REFLECT_RESULT_SUCCESS);

	std::ranges::sort(m_outputVariables, [](const auto* a, const auto* b) { return a->location < b->location; });

	if (m_module.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
	{
		m_vertexInputs.reserve(m_inputVariables.size());

		for (const auto* variable : m_inputVariables)
		{
			if (!variable)
			{
				continue;
			}

			if (variable->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
			{
				continue;
			}

			if (variable->location == std::numeric_limits<uint32_t>::max())
			{
				continue;
			}

			const VertexAttribute attribute = GetVertexAttribute(*variable);

			m_vertexInputs.push_back({
			    .attribute = attribute,
			    .location = variable->location,
			    .format = GetVkFormat(variable->format),
			});
		}

		std::ranges::sort(m_vertexInputs, [](const auto& a, const auto& b) { return a.location < b.location; });
	}


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

const std::vector<Prism::Render::Vulkan::VulkanShaderVertexInput>&
Prism::Render::Vulkan::VulkanShaderReflection::GetVertexInputs() const
{
	return m_vertexInputs;
}

Prism::Render::VertexAttribute
Prism::Render::Vulkan::VulkanShaderReflection::GetVertexAttribute(const SpvReflectInterfaceVariable& variable)
{
	PE_ASSERT(variable.semantic != nullptr);

	const std::string_view semantic = variable.semantic;

	if (semantic == "POSITION")
	{
		return VertexAttribute::Position;
	}

	if (semantic == "NORMAL")
	{
		return VertexAttribute::Normal;
	}

	if (semantic == "TEXCOORD")
	{
		return VertexAttribute::TexCoord;
	}

	if (semantic == "TANGENT")
	{
		return VertexAttribute::Tangent;
	}

	if (semantic == "BITANGENT")
	{
		return VertexAttribute::Bitangent;
	}

	if (semantic == "COLOR")
	{
		return VertexAttribute::Color;
	}

	PE_ASSERT_NO_ENTRY();
	return VertexAttribute::Position;
}
