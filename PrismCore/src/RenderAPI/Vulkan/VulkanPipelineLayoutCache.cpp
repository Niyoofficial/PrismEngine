#include "VulkanPipelineLayoutCache.h"
#include "VulkanRenderDevice.h"
#include "VulkanShaderReflection.h"
#include "VulkanTypeConversions.h"

Prism::Render::Vulkan::VulkanPipelineLayoutCache::~VulkanPipelineLayoutCache()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	for (const auto& layout : m_cache | std::views::values)
	{
		vkDestroyPipelineLayout(device, layout, nullptr);
	}
}

VkPipelineLayout Prism::Render::Vulkan::VulkanPipelineLayoutCache::GetOrCreatePipelineLayout(
    const std::span<const VulkanShaderReflection* const> shaderReflections)
{
	PipelineLayoutKey pipelineKey;

	std::set<uint32_t> usedSets;

	for (const auto& reflection : shaderReflections)
	{
		for (uint32_t i = 0; i < reflection.GetDescriptorSetCount(); ++i)
		{
			usedSets.insert(reflection.GetDescriptorSet(i).set);
		}
	}

	std::vector<VkDescriptorSetLayout> setLayouts;

	setLayouts.reserve(usedSets.size());

	for (const uint32_t set : usedSets)
	{
		auto layoutKey = VulkanDescriptorSetLayoutCache::BuildLayoutKey(shaderReflections, set);

		pipelineKey.descriptorSets.push_back(layoutKey);

		setLayouts.push_back(m_descriptorSetLayoutCache.GetOrCreateDescriptorSetLayout(layoutKey));
	}

	for (const auto& reflection : shaderReflections)
	{
		for (uint32_t i = 0; i < reflection->GetPushConstantBlockCount(); ++i)
		{
			const auto& block = reflection->GetPushConstantBlock(i);

			auto it = std::ranges::find_if(pipelineKey.pushConstants, [&](const PushConstantKey& key)
			                               { return key.offset == block.offset && key.size == block.size; });

			if (it == pipelineKey.pushConstants.end())
			{
				pipelineKey.pushConstants.push_back({
				    .offset = block.offset,
				    .size = block.size,
				    .stageFlags = GetVkShaderStageFlags(reflection->GetShaderType()),
				});
			}
			else
			{
				it->stageFlags |= GetVkShaderStageFlags(reflection->GetShaderType());
			}
		}
	}

	if (const auto it = m_cache.find(pipelineKey); it != m_cache.end())
	{
		return it->second;
	}

	std::vector<VkPushConstantRange> pushConstantRanges;

	pushConstantRanges.reserve(pipelineKey.pushConstants.size());

	for (const auto& [offset, size, stageFlags] : pipelineKey.pushConstants)
	{
		pushConstantRanges.push_back({
		    .stageFlags = stageFlags,
		    .offset = offset,
		    .size = size,
		});
	}

	const VkPipelineLayoutCreateInfo createInfo{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	    .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
	    .pSetLayouts = setLayouts.data(),
	    .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
	    .pPushConstantRanges = pushConstantRanges.data(),
	};

	VkPipelineLayout pipelineLayout;

	const auto device = VulkanRenderDevice::Get().GetDevice();

	const VkResult result = vkCreatePipelineLayout(device, &createInfo, nullptr, &pipelineLayout);

	PE_ASSERT(result == VK_SUCCESS);

	m_cache.emplace(std::move(pipelineKey), pipelineLayout);

	return pipelineLayout;
}
