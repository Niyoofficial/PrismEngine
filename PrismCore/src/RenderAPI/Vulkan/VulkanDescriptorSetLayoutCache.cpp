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
    const std::span<const VulkanShaderReflection* const> shaderReflections, const uint32_t set)
{
	return GetOrCreateDescriptorSetLayout(BuildLayoutKey(shaderReflections, set));
}

VkDescriptorSetLayout
Prism::Render::Vulkan::VulkanDescriptorSetLayoutCache::GetOrCreateDescriptorSetLayout(const DescriptorSetLayoutKey& layoutKey)
{
	if (const auto it = m_cache.find(layoutKey); it != m_cache.end())
	{
		return it->second;
	}

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.reserve(layoutKey.bindings.size());

	for (const auto& [binding, type, count, stageFlags] : layoutKey.bindings)
	{
		PE_ASSERT(count > 0);
		PE_ASSERT(type != VK_DESCRIPTOR_TYPE_MAX_ENUM);
		PE_ASSERT(stageFlags != 0);

		bindings.push_back({
		    .binding = binding,
		    .descriptorType = type,
		    .descriptorCount = count,
		    .stageFlags = stageFlags,
		    .pImmutableSamplers = nullptr,
		});
	}

	const VkDescriptorSetLayoutCreateInfo createInfo{
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

Prism::Render::Vulkan::DescriptorSetLayoutKey Prism::Render::Vulkan::VulkanDescriptorSetLayoutCache::BuildLayoutKey(
    const std::span<const VulkanShaderReflection* const> shaderReflections, const uint32_t set)
{
	DescriptorSetLayoutKey layoutKey;

	// set 0 is reserved for bindless
	PE_ASSERT(set != 0);

	for (const auto& reflection : shaderReflections)
	{
		const auto* descriptorSet = reflection->FindDescriptorSet(set);

		if (!descriptorSet)
		{
			continue;
		}

		const VkShaderStageFlags stageFlags = GetVkShaderStageFlags(reflection->GetShaderType());

		for (uint32_t i = 0; i < descriptorSet->binding_count; ++i)
		{
			PE_ASSERT(descriptorSet->bindings[i] != nullptr);

			const auto& binding = *descriptorSet->bindings[i];

			const VkDescriptorType descriptorType = GetVkDescriptorType(binding.descriptor_type);

			const uint32_t descriptorCount = GetDescriptorCount(binding);

			PE_ASSERT(descriptorCount > 0);

			auto it = std::ranges::find_if(layoutKey.bindings,
			                               [&](const DescriptorBindingKey& key) { return key.binding == binding.binding; });

			if (it == layoutKey.bindings.end())
			{
				layoutKey.bindings.push_back({
				    .binding = binding.binding,
				    .type = descriptorType,
				    .count = descriptorCount,
				    .stageFlags = stageFlags,
				});

				continue;
			}

			PE_ASSERT(it->type == descriptorType, "Descriptor type mismatch between shader stages");

			PE_ASSERT(it->count == descriptorCount, "Descriptor count mismatch between shader stages");

			it->stageFlags |= stageFlags;
		}
	}

	std::ranges::sort(layoutKey.bindings, {}, &DescriptorBindingKey::binding);

	return layoutKey;
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
