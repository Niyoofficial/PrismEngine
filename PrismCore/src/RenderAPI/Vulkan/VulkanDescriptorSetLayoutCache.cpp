#include "VulkanDescriptorSetLayoutCache.h"

#include "VulkanRenderDevice.h"
#include "VulkanTypeConversions.h"

Prism::Render::Vulkan::VulkanDescriptorSetLayoutCache::~VulkanDescriptorSetLayoutCache()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	for (const auto& layout : m_cache | std::views::values)
	{
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
	}
}

VkDescriptorSetLayout Prism::Render::Vulkan::VulkanDescriptorSetLayoutCache::GetOrCreateDescriptorSetLayout(
    std::span<const VulkanShaderReflection> shaderReflections, uint32_t set)
{
	DescriptorSetLayoutKey layoutKey;

	for (const auto& reflection : shaderReflections)
	{
		const auto* descriptorSet = reflection.FindDescriptorSet(set);

		if (!descriptorSet)
		{
			continue;
		}

		for (uint32_t i = 0; i < descriptorSet->binding_count; ++i)
		{
			const auto& binding = *descriptorSet->bindings[i];

			const uint32_t descriptorCount = GetDescriptorCount(binding);

			auto it = std::ranges::find_if(layoutKey.bindings,
			                               [&](const DescriptorBindingKey& key) { return key.binding == binding.binding; });

			if (it == layoutKey.bindings.end())
			{
				layoutKey.bindings.push_back({
				    .binding = binding.binding,
				    .type = static_cast<VkDescriptorType>(binding.descriptor_type),
				    .count = descriptorCount,
				    .stageFlags = GetVkShaderStageFlags(reflection.GetShaderType()),
				});
			}
			else
			{
				PE_ASSERT(it->type == static_cast<VkDescriptorType>(binding.descriptor_type));

				PE_ASSERT(it->count == descriptorCount);

				it->stageFlags |= GetVkShaderStageFlags(reflection.GetShaderType());
			}
		}
	}

	std::ranges::sort(layoutKey.bindings, {}, &DescriptorBindingKey::binding);

	if (const auto it = m_cache.find(layoutKey); it != m_cache.end())
	{
		return it->second;
	}

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	bindings.reserve(layoutKey.bindings.size());

	for (const auto& [binding, type, count, stageFlags] : layoutKey.bindings)
	{
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{
		    .binding = binding,
		    .descriptorType = type,
		    .descriptorCount = count,
		    .stageFlags = stageFlags,
		    .pImmutableSamplers = nullptr,
		};
		bindings.push_back(descriptorSetLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo createInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	    .bindingCount = static_cast<uint32_t>(bindings.size()),
	    .pBindings = bindings.data(),
	};

	VkDescriptorSetLayout layout;

	const auto device = VulkanRenderDevice::Get().GetDevice();

	const VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout);

	PE_ASSERT(result == VK_SUCCESS);

	m_cache.emplace(layoutKey, layout);

	return layout;
}

uint32_t Prism::Render::Vulkan::VulkanDescriptorSetLayoutCache::GetDescriptorCount(const SpvReflectDescriptorBinding& binding)
{
	uint32_t count = 1;

	for (uint32_t i = 0; i < binding.array.dims_count; ++i)
	{
		count *= binding.array.dims[i];
	}

	return count;
}
