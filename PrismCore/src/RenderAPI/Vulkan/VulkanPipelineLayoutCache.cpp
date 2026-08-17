#include "VulkanPipelineLayoutCache.h"
#include "VulkanRenderDevice.h"
#include "VulkanShaderReflection.h"
#include "VulkanTypeConversions.h"

Prism::Render::Vulkan::VulkanPipelineLayoutCache::~VulkanPipelineLayoutCache()
{
	const VkDevice device = VulkanRenderDevice::Get().GetDevice();

	for (const auto& pipelineLayout : m_cache | std::views::values)
	{
		if (pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		}
	}
}

VkPipelineLayout Prism::Render::Vulkan::VulkanPipelineLayoutCache::GetOrCreatePipelineLayout(
    const std::span<const VulkanShaderReflection* const> shaderReflections)
{
	PE_ASSERT(!shaderReflections.empty());

	PipelineLayoutKey pipelineKey;

	// set 0 is reserved for bindless
	pipelineKey.descriptorSets.emplace_back();

	std::vector<VkDescriptorSetLayout> setLayouts;

	setLayouts.push_back(VulkanRenderDevice::Get().GetBindlessManager().GetLayout());

	uint32_t maxSet = 0;

	for (const VulkanShaderReflection* reflection : shaderReflections)
	{
		PE_ASSERT(reflection != nullptr);

		const uint32_t descriptorSetCount = reflection->GetDescriptorSetCount();

		for (uint32_t i = 0; i < descriptorSetCount; ++i)
		{
			const SpvReflectDescriptorSet& descriptorSet = reflection->GetDescriptorSet(i);

			const uint32_t set = descriptorSet.set;

			if (set == 0)
			{
				continue;
			}

			maxSet = std::max(maxSet, set);
		}
	}

	for (uint32_t set = 1; set <= maxSet; ++set)
	{
		auto layoutKey = VulkanDescriptorSetLayoutCache::BuildLayoutKey(shaderReflections, set);

		pipelineKey.descriptorSets.push_back(layoutKey);

		const VkDescriptorSetLayout layout = m_descriptorSetLayoutCache.GetOrCreateDescriptorSetLayout(layoutKey);

		setLayouts.push_back(layout);
	}

	struct RawPushConstantRange
	{
		uint32_t offset;
		uint32_t size;
		VkShaderStageFlags stageFlags;
	};

	std::vector<RawPushConstantRange> rawPushConstants;

	for (const VulkanShaderReflection* reflection : shaderReflections)
	{
		const VkShaderStageFlags stageFlags = GetVkShaderStageFlags(reflection->GetShaderType());

		const uint32_t pushConstantCount = reflection->GetPushConstantBlockCount();

		for (uint32_t i = 0; i < pushConstantCount; ++i)
		{
			const SpvReflectBlockVariable& block = reflection->GetPushConstantBlock(i);

			if (block.size == 0)
			{
				continue;
			}

			rawPushConstants.push_back({
			    .offset = block.offset,
			    .size = block.size,
			    .stageFlags = stageFlags,
			});
		}
	}

	if (!rawPushConstants.empty())
	{

		std::vector<uint32_t> boundaries;
		boundaries.reserve(rawPushConstants.size() * 2);

		for (const auto& range : rawPushConstants)
		{
			boundaries.push_back(range.offset);
			boundaries.push_back(range.offset + range.size);
		}

		std::ranges::sort(boundaries);

		boundaries.erase(std::ranges::unique(boundaries).begin(), boundaries.end());

		for (size_t i = 0; i + 1 < boundaries.size(); ++i)
		{
			const uint32_t begin = boundaries[i];
			const uint32_t end = boundaries[i + 1];

			if (begin == end)
			{
				continue;
			}

			VkShaderStageFlags stageFlags = 0;

			for (const auto& range : rawPushConstants)
			{
				const uint32_t rangeBegin = range.offset;
				const uint32_t rangeEnd = range.offset + range.size;

				if (begin >= rangeBegin && end <= rangeEnd)
				{
					stageFlags |= range.stageFlags;
				}
			}

			if (stageFlags == 0)
			{
				continue;
			}

			pipelineKey.pushConstants.push_back({
			    .offset = begin,
			    .size = end - begin,
			    .stageFlags = stageFlags,
			});
		}
	}

	if (const auto it = m_cache.find(pipelineKey); it != m_cache.end())
	{
		return it->second;
	}


	std::vector<VkPushConstantRange> pushConstantRanges;

	pushConstantRanges.reserve(pipelineKey.pushConstants.size());

	for (const PushConstantKey& key : pipelineKey.pushConstants)
	{
		pushConstantRanges.push_back({
		    .stageFlags = key.stageFlags,
		    .offset = key.offset,
		    .size = key.size,
		});
	}

	VkPipelineLayoutCreateInfo createInfo{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	    .pNext = nullptr,
	    .flags = 0,
	    .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
	    .pSetLayouts = setLayouts.data(),
	    .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
	    .pPushConstantRanges = pushConstantRanges.data(),
	};

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	PE_ASSERT(vkCreatePipelineLayout(VulkanRenderDevice::Get().GetDevice(), &createInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

	m_cache.emplace(std::move(pipelineKey), pipelineLayout);

	return pipelineLayout;
}
