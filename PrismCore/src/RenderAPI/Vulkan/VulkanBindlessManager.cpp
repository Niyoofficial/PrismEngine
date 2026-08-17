#include "VulkanBindlessManager.h"

#include <array>
#include "Prism/Base/Assert.h"

namespace
{
constexpr std::array MutableResourceTypes = {
    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
};
} // namespace

void Prism::Render::Vulkan::VulkanBindlessManager::Initialize(VkDevice device)
{
	std::array<VkDescriptorSetLayoutBinding, 10> bindings{};

	for (uint32_t i = 0; i < SamplerCount; ++i)
	{
		bindings[i] = {
		    .binding = i,
		    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
		    .descriptorCount = 1,
		    .stageFlags = VK_SHADER_STAGE_ALL,
		    .pImmutableSamplers = nullptr,
		};
	}

	bindings[ResourcesBinding] = {
	    .binding = ResourcesBinding,
	    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    .descriptorCount = 1,
	    .stageFlags = VK_SHADER_STAGE_ALL,
	    .pImmutableSamplers = nullptr,
	};
	bindings[ResourceHeapBinding] = {
	    .binding = ResourceHeapBinding,
	    .descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
	    .descriptorCount = MaxBindlessDescriptors,
	    .stageFlags = VK_SHADER_STAGE_ALL,
	    .pImmutableSamplers = nullptr,
	};

	bindings[LegacyBufferHeapBinding] = {
	    .binding = LegacyBufferHeapBinding,
	    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	    .descriptorCount = MaxBindlessDescriptors,
	    .stageFlags = VK_SHADER_STAGE_ALL,
	    .pImmutableSamplers = nullptr,
	};

	std::array<VkMutableDescriptorTypeListEXT, 10> mutableLists{};

	mutableLists[ResourceHeapBinding] = VkMutableDescriptorTypeListEXT{
	    .descriptorTypeCount = static_cast<uint32_t>(MutableResourceTypes.size()),
	    .pDescriptorTypes = MutableResourceTypes.data(),
	};

	const VkMutableDescriptorTypeCreateInfoEXT mutableDescriptorInfo{
	    .sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT,
	    .pNext = nullptr,
	    .mutableDescriptorTypeListCount = static_cast<uint32_t>(mutableLists.size()),
	    .pMutableDescriptorTypeLists = mutableLists.data(),
	};

	std::array<VkDescriptorBindingFlags, 10> bindingFlags{};

	bindingFlags[ResourceHeapBinding] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

	bindingFlags[LegacyBufferHeapBinding] =
	    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

	const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
	    .pNext = &mutableDescriptorInfo,
	    .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
	    .pBindingFlags = bindingFlags.data(),
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	    .pNext = &bindingFlagsInfo,
	    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
	    .bindingCount = static_cast<uint32_t>(bindings.size()),
	    .pBindings = bindings.data(),
	};

	PE_ASSERT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_layout) == VK_SUCCESS);

	std::array<VkDescriptorPoolSize, 4> poolSizes{
	    VkDescriptorPoolSize{
	        .type = VK_DESCRIPTOR_TYPE_SAMPLER,
	        .descriptorCount = SamplerCount,
	    },
	    VkDescriptorPoolSize{
	        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	        .descriptorCount = 1,
	    },
	    VkDescriptorPoolSize{
	        .type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
	        .descriptorCount = MaxBindlessDescriptors,
	    },
	    VkDescriptorPoolSize{
	        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	        .descriptorCount = MaxBindlessDescriptors,
	    },
	};

	constexpr VkMutableDescriptorTypeListEXT poolMutableList{
	    .descriptorTypeCount = static_cast<uint32_t>(MutableResourceTypes.size()),
	    .pDescriptorTypes = MutableResourceTypes.data(),
	};

	std::array<VkMutableDescriptorTypeListEXT, 4> poolMutableLists{};

	// [0] = SAMPLER
	// [1] = UBO
	// [2] = MUTABLE
	poolMutableLists[2] = poolMutableList;

	VkMutableDescriptorTypeCreateInfoEXT poolMutableInfo{
	    .sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT,
	    .pNext = nullptr,
	    .mutableDescriptorTypeListCount = static_cast<uint32_t>(poolMutableLists.size()),
	    .pMutableDescriptorTypeLists = poolMutableLists.data(),
	};

	VkDescriptorPoolCreateInfo poolInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	    .pNext = &poolMutableInfo,
	    .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
	    .maxSets = 1,
	    .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
	    .pPoolSizes = poolSizes.data(),
	};

	PE_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool) == VK_SUCCESS);

	const VkDescriptorSetAllocateInfo allocateInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	    .descriptorPool = m_pool,
	    .descriptorSetCount = 1,
	    .pSetLayouts = &m_layout,
	};

	PE_ASSERT(vkAllocateDescriptorSets(device, &allocateInfo, &m_set) == VK_SUCCESS);
}

void Prism::Render::Vulkan::VulkanBindlessManager::Shutdown(VkDevice device)
{
	if (m_pool)
	{
		vkDestroyDescriptorPool(device, m_pool, nullptr);
		m_pool = VK_NULL_HANDLE;
		m_set = VK_NULL_HANDLE;
	}

	if (m_layout)
	{
		vkDestroyDescriptorSetLayout(device, m_layout, nullptr);
		m_layout = VK_NULL_HANDLE;
	}
}

uint32_t Prism::Render::Vulkan::VulkanBindlessManager::AllocateResource()
{
	auto& list = m_resourceFreeList;

	std::lock_guard lock(list.mutex);

	if (!list.freeIndices.empty())
	{
		const uint32_t index = list.freeIndices.back();

		list.freeIndices.pop_back();

		return index;
	}

	PE_ASSERT(list.nextIndex < MaxBindlessDescriptors, "Bindless descriptor heap exhausted");

	return list.nextIndex++;
}

void Prism::Render::Vulkan::VulkanBindlessManager::FreeResource(uint32_t index)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	auto& list = m_resourceFreeList;

	std::lock_guard lock(list.mutex);

	list.freeIndices.push_back(index);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteSampledImage(VkDevice device, uint32_t index, VkImageView view,
                                                                     VkImageLayout layout)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	VkDescriptorImageInfo imageInfo{
	    .sampler = VK_NULL_HANDLE,
	    .imageView = view,
	    .imageLayout = layout,
	};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = ResourceHeapBinding,
	    .dstArrayElement = index,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	    .pImageInfo = &imageInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteStorageImage(VkDevice device, uint32_t index, VkImageView view,
                                                                     VkImageLayout layout)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	VkDescriptorImageInfo imageInfo{
	    .sampler = VK_NULL_HANDLE,
	    .imageView = view,
	    .imageLayout = layout,
	};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = ResourceHeapBinding,
	    .dstArrayElement = index,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	    .pImageInfo = &imageInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteUniformBuffer(VkDevice device, uint32_t index, VkBuffer buffer,
                                                                      VkDeviceSize offset, VkDeviceSize range)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	VkDescriptorBufferInfo bufferInfo{
	    .buffer = buffer,
	    .offset = offset,
	    .range = range,
	};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = ResourceHeapBinding,
	    .dstArrayElement = index,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    .pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteStorageBuffer(VkDevice device, uint32_t index, VkBuffer buffer,
                                                                      VkDeviceSize offset, VkDeviceSize range)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	VkDescriptorBufferInfo bufferInfo{
	    .buffer = buffer,
	    .offset = offset,
	    .range = range,
	};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = ResourceHeapBinding,
	    .dstArrayElement = index,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	    .pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteResourcesBuffer(VkDevice device, VkBuffer buffer, VkDeviceSize offset,
                                                                        VkDeviceSize range)
{
	VkDescriptorBufferInfo bufferInfo{
	    .buffer = buffer,
	    .offset = offset,
	    .range = range,
	};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = ResourcesBinding,
	    .dstArrayElement = 0,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    .pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteSampler(VkDevice device, uint32_t samplerIndex, VkSampler sampler)
{
	PE_ASSERT(samplerIndex < SamplerCount);

	VkDescriptorImageInfo samplerInfo{
	    .sampler = sampler,
	    .imageView = VK_NULL_HANDLE,
	    .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = SamplerBindingBegin + samplerIndex,
	    .dstArrayElement = 0,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
	    .pImageInfo = &samplerInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanBindlessManager::WriteLegacyBuffer(VkDevice device, uint32_t index, VkBuffer buffer,
                                                                     VkDeviceSize offset, VkDeviceSize range)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	VkDescriptorBufferInfo bufferInfo{
	    .buffer = buffer,
	    .offset = offset,
	    .range = range,
	};

	const VkWriteDescriptorSet write{
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = m_set,
	    .dstBinding = LegacyBufferHeapBinding,
	    .dstArrayElement = index,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	    .pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}
