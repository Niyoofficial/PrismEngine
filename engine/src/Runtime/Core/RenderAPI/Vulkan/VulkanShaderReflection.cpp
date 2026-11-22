#include "VulkanShaderReflection.h"

#include "Prism/Base/Assert.h"
#include "Prism/Render/Shader.h"
#include "Prism/Render/VertexBufferCache.h"
#include "VulkanTypeConversions.h"

namespace Prism::Render::Vulkan
{
VulkanShaderReflection::~VulkanShaderReflection() { spvReflectDestroyShaderModule(&m_module); }

VulkanShaderReflection::VulkanShaderReflection(VulkanShaderReflection&& other) noexcept
{
	m_module = other.m_module;
	std::memset(&other.m_module, 0, sizeof(other.m_module));

	m_inputVariables = std::move(other.m_inputVariables);
	m_outputVariables = std::move(other.m_outputVariables);
	m_vertexInputs = std::move(other.m_vertexInputs);
}

VulkanShaderReflection& VulkanShaderReflection::operator=(VulkanShaderReflection&& other) noexcept
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

SpvReflectResult VulkanShaderReflection::Create(const size_t size, const void* data)
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

ShaderType VulkanShaderReflection::GetShaderType() const
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

uint32_t VulkanShaderReflection::GetDescriptorSetCount() const { return m_module.descriptor_set_count; }

const SpvReflectDescriptorSet& VulkanShaderReflection::GetDescriptorSet(const uint32_t index) const
{
	PE_ASSERT(index < m_module.descriptor_set_count);

	return m_module.descriptor_sets[index];
}

uint32_t VulkanShaderReflection::GetPushConstantBlockCount() const { return m_module.push_constant_block_count; }

const SpvReflectBlockVariable& VulkanShaderReflection::GetPushConstantBlock(const uint32_t index) const
{
	PE_ASSERT(index < m_module.push_constant_block_count);

	return m_module.push_constant_blocks[index];
}

uint32_t VulkanShaderReflection::GetInputVariableCount() const { return static_cast<uint32_t>(m_inputVariables.size()); }

const SpvReflectInterfaceVariable& VulkanShaderReflection::GetInputVariable(const uint32_t index) const
{
	PE_ASSERT(index < m_inputVariables.size());

	return *m_inputVariables[index];
}

const std::vector<SpvReflectInterfaceVariable*>& VulkanShaderReflection::GetInputVariables() const { return m_inputVariables; }

const std::vector<SpvReflectInterfaceVariable*>& VulkanShaderReflection::GetOutputVariables() const { return m_outputVariables; }

const SpvReflectDescriptorSet* VulkanShaderReflection::FindDescriptorSet(const uint32_t set) const
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

const std::vector<VulkanShaderVertexInput>& VulkanShaderReflection::GetVertexInputs() const { return m_vertexInputs; }

std::vector<VertexAttribute> VulkanShaderReflection::GetVertexAttributeLayout() const
{
	std::vector<VertexAttribute> layout;
	layout.reserve(m_vertexInputs.size());

	for (const auto& input : m_vertexInputs)
	{
		layout.push_back(input.attribute);
	}

	return layout;
}

std::vector<VulkanPushConstantBlock>
VulkanShaderReflection::BuildPushConstantLayout(const std::vector<const VulkanShaderReflection*>& reflections)
{
	std::vector<VulkanPushConstantBlock> result;

	for (const auto* reflection : reflections)
	{
		PE_ASSERT(reflection != nullptr, "Shader reflection is null");

		for (uint32_t i = 0; i < reflection->GetPushConstantBlockCount(); ++i)
		{
			const auto& block = reflection->GetPushConstantBlock(i);

			const uint32_t blockSize = block.padded_size > 0 ? block.padded_size : block.size;

			if (blockSize == 0)
			{
				continue;
			}

			auto it = std::ranges::find_if(result, [&block, blockSize](const VulkanPushConstantBlock& existing)
										   { return existing.offset == block.offset && existing.size == blockSize; });

			if (it == result.end())
			{
				result.push_back({
					.offset = block.offset,
					.size = blockSize,
				});

				it = std::prev(result.end());
			}

			for (uint32_t j = 0; j < block.member_count; ++j)
			{
				const auto& member = block.members[j];

				if (!member.name)
				{
					continue;
				}

				const uint32_t memberSize = member.size > 0 ? member.size : sizeof(uint32_t);

				PE_ASSERT(member.offset + memberSize <= blockSize, "Push constant member out of block bounds: {}", member.name);

				const auto memberIt = std::ranges::find_if(it->members, [&member](const VulkanPushConstantMember& existing)
														   { return existing.offset == member.offset; });

				if (memberIt != it->members.end())
				{
					PE_ASSERT(memberIt->name == StringToWString(member.name) && memberIt->size == memberSize,
							  "Incompatible push constant layout at offset {}", member.offset);

					continue;
				}

				it->members.push_back({
					.name = StringToWString(member.name),
					.offset = member.offset,
					.size = memberSize,
				});
			}
		}
	}

	return result;
}

VertexAttribute VulkanShaderReflection::GetVertexAttribute(const SpvReflectInterfaceVariable& variable)
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
} // namespace Prism::Render::Vulkan
