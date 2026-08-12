#include "VulkanBindlessManager.h"
#include "Prism/Base/Assert.h"

void Prism::Render::Vulkan::VulkanBindlessManager::Initialize(VkDevice device)
{
	constexpr VkDescriptorType bindingTypes[] = {
	    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, // Texture2D
	    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, // TextureCube
	    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // RawBuffer
	};

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	std::vector<VkDescriptorBindingFlags> bindingFlags;

	for (uint32_t i = 0; i < static_cast<uint32_t>(BindlessBinding::Count); ++i)
	{
		bindings.push_back({
		    .binding = i,
		    .descriptorType = bindingTypes[i],
		    .descriptorCount = MaxBindlessDescriptorsPerBinding,
		    .stageFlags = VK_SHADER_STAGE_ALL,
		});

		bindingFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
	}

	const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
	    .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
	    .pBindingFlags = bindingFlags.data(),
	};

	const VkDescriptorSetLayoutCreateInfo layoutCreateInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	    .pNext = &bindingFlagsInfo,
	    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
	    .bindingCount = static_cast<uint32_t>(bindings.size()),
	    .pBindings = bindings.data(),
	};

	PE_ASSERT(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &m_layout) == VK_SUCCESS);

	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto type : bindingTypes)
	{
		poolSizes.push_back({.type = type, .descriptorCount = MaxBindlessDescriptorsPerBinding});
	}

	const VkDescriptorPoolCreateInfo poolCreateInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	    .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
	    .maxSets = 1,
	    .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
	    .pPoolSizes = poolSizes.data(),
	};

	PE_ASSERT(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &m_pool) == VK_SUCCESS);

	constexpr uint32_t variableCount = MaxBindlessDescriptorsPerBinding;

	const VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
	    .descriptorSetCount = 1,
	    .pDescriptorCounts = &variableCount,
	};

	const VkDescriptorSetAllocateInfo allocInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	    .pNext = &variableCountInfo,
	    .descriptorPool = m_pool,
	    .descriptorSetCount = 1,
	    .pSetLayouts = &m_layout,
	};

	PE_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, &m_set) == VK_SUCCESS);
}

void Prism::Render::Vulkan::VulkanBindlessManager::Shutdown(VkDevice device)
{
	if (m_pool)
	{
		vkDestroyDescriptorPool(device, m_pool, nullptr);
	}
	if (m_layout)
	{
		vkDestroyDescriptorSetLayout(device, m_layout, nullptr);
	}
}

uint32_t Prism::Render::Vulkan::VulkanBindlessManager::AllocateSlot(BindlessBinding binding)
{
	auto& list = m_freeLists[static_cast<size_t>(binding)];
	std::lock_guard lock(list.mutex);

	if (!list.freeIndices.empty())
	{
		const uint32_t slot = list.freeIndices.back();
		list.freeIndices.pop_back();
		return slot;
	}

	PE_ASSERT(list.nextIndex < MaxBindlessDescriptorsPerBinding, "Bindless heap exhausted");
	return list.nextIndex++;
}

void Prism::Render::Vulkan::VulkanBindlessManager::FreeSlot(BindlessBinding binding, uint32_t slot)
{
	auto& list = m_freeLists[static_cast<size_t>(binding)];
	std::lock_guard lock(list.mutex);
	list.freeIndices.push_back(slot);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteTexture(VkDevice device, uint32_t slot, VkImageView view,
                                                                VkImageLayout layout)
{
	const VkDescriptorImageInfo imageInfo{.imageView = view, .imageLayout = layout};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = static_cast<uint32_t>(BindlessBinding::Texture2D),
	    .dstArrayElement = slot,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	    .pImageInfo = &imageInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteRawBuffer(VkDevice device, const uint32_t slot, VkBuffer buffer,
                                                                      VkDeviceSize offset, VkDeviceSize range)
{
	const VkDescriptorBufferInfo bufferInfo{.buffer = buffer, .offset = offset, .range = range};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = static_cast<uint32_t>(BindlessBinding::RawBuffer),
	    .dstArrayElement = slot,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	    .pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}
