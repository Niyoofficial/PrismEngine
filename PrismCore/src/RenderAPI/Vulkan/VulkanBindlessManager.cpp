#include "VulkanBindlessManager.h"

#include <array>

#include "Prism/Base/Assert.h"
#include "VulkanRenderDevice.h"

namespace
{
uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties{};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if (typeFilter & 1u << i && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	return UINT32_MAX;
}

constexpr std::array MutableResourceTypes = {
    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
};
} // namespace

Prism::Render::Vulkan::VulkanBindlessManager::VulkanBindlessManager()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

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
	    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	    .descriptorCount = 1,
	    .stageFlags = VK_SHADER_STAGE_ALL,
	    .pImmutableSamplers = nullptr,
	};

	// ResourceDescriptorHeap[]
	bindings[ResourceHeapBinding] = {
	    .binding = ResourceHeapBinding,
	    .descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
	    .descriptorCount = MaxBindlessDescriptors,
	    .stageFlags = VK_SHADER_STAGE_ALL,
	    .pImmutableSamplers = nullptr,
	};

	// g_bindlessBuffers[]
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

	bindingFlags[ResourceHeapBinding] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	bindingFlags[LegacyBufferHeapBinding] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

	const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
	    .pNext = &mutableDescriptorInfo,
	    .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
	    .pBindingFlags = bindingFlags.data(),
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	    .pNext = &bindingFlagsInfo,
	    .flags = 0,
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
	        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
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
	// [1] = UNIFORM_BUFFER_DYNAMIC
	// [2] = MUTABLE
	// [3] = STORAGE_BUFFER
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
	    .flags = 0,
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

	const auto physicalDevice = VulkanRenderDevice::Get().GetPhysicalDevice();

	CreateResourcesBuffer(device, physicalDevice);

	WriteResourcesBuffer(device, m_resourcesBuffer, 0, ResourcesSize);

	CreateSamplers(device, physicalDevice);
}

Prism::Render::Vulkan::VulkanBindlessManager::ResourcesAllocation
Prism::Render::Vulkan::VulkanBindlessManager::AllocateResources()
{
	const uint32_t index = m_resourcesCursor.fetch_add(1, std::memory_order_relaxed);

	PE_ASSERT(index < ResourcesRingCapacity, "Resources buffer exhausted");

	const VkDeviceSize offset = static_cast<VkDeviceSize>(index) * m_resourcesStride;

	auto* data = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(m_resourcesMapped) + offset);

	std::memset(data, 0, ResourcesSize);

	return {
	    .offset = offset,
	    .data = data,
	};
}

void Prism::Render::Vulkan::VulkanBindlessManager::ResetResourcesAllocator()
{
	m_resourcesCursor.store(0, std::memory_order_relaxed);
}

void Prism::Render::Vulkan::VulkanBindlessManager::CreateSamplers(VkDevice device, VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	const float maxAnisotropy = std::min(8.0f, properties.limits.maxSamplerAnisotropy);

	// 0 - s_pointWrap
	{
		constexpr VkSamplerCreateInfo info{
		    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		    .magFilter = VK_FILTER_NEAREST,
		    .minFilter = VK_FILTER_NEAREST,
		    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .mipLodBias = 0.0f,
		    .anisotropyEnable = VK_FALSE,
		    .maxAnisotropy = 1.0f,
		    .compareEnable = VK_FALSE,
		    .compareOp = VK_COMPARE_OP_ALWAYS,
		    .minLod = 0.0f,
		    .maxLod = VK_LOD_CLAMP_NONE,
		    .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		    .unnormalizedCoordinates = VK_FALSE,
		};

		PE_ASSERT(vkCreateSampler(device, &info, nullptr, &m_samplers[0]) == VK_SUCCESS);
	}

	// 1 - s_pointClamp
	{
		constexpr VkSamplerCreateInfo info{
		    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		    .magFilter = VK_FILTER_NEAREST,
		    .minFilter = VK_FILTER_NEAREST,
		    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .mipLodBias = 0.0f,
		    .anisotropyEnable = VK_FALSE,
		    .maxAnisotropy = 1.0f,
		    .compareEnable = VK_FALSE,
		    .compareOp = VK_COMPARE_OP_ALWAYS,
		    .minLod = 0.0f,
		    .maxLod = VK_LOD_CLAMP_NONE,
		    .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		    .unnormalizedCoordinates = VK_FALSE,
		};

		PE_ASSERT(vkCreateSampler(device, &info, nullptr, &m_samplers[1]) == VK_SUCCESS);
	}

	// 2 - s_linearWrap
	{
		constexpr VkSamplerCreateInfo info{
		    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		    .magFilter = VK_FILTER_LINEAR,
		    .minFilter = VK_FILTER_LINEAR,
		    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .mipLodBias = 0.0f,
		    .anisotropyEnable = VK_FALSE,
		    .maxAnisotropy = 1.0f,
		    .compareEnable = VK_FALSE,
		    .compareOp = VK_COMPARE_OP_ALWAYS,
		    .minLod = 0.0f,
		    .maxLod = VK_LOD_CLAMP_NONE,
		    .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		    .unnormalizedCoordinates = VK_FALSE,
		};

		PE_ASSERT(vkCreateSampler(device, &info, nullptr, &m_samplers[2]) == VK_SUCCESS);
	}

	// 3 - s_linearClamp
	{
		constexpr VkSamplerCreateInfo info{
		    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		    .magFilter = VK_FILTER_LINEAR,
		    .minFilter = VK_FILTER_LINEAR,
		    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .mipLodBias = 0.0f,
		    .anisotropyEnable = VK_FALSE,
		    .maxAnisotropy = 1.0f,
		    .compareEnable = VK_FALSE,
		    .compareOp = VK_COMPARE_OP_ALWAYS,
		    .minLod = 0.0f,
		    .maxLod = VK_LOD_CLAMP_NONE,
		    .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		    .unnormalizedCoordinates = VK_FALSE,
		};

		PE_ASSERT(vkCreateSampler(device, &info, nullptr, &m_samplers[3]) == VK_SUCCESS);
	}

	// 4 - s_anisotropicWrap
	{
		const VkSamplerCreateInfo info{
		    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		    .magFilter = VK_FILTER_LINEAR,
		    .minFilter = VK_FILTER_LINEAR,
		    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .mipLodBias = 0.0f,
		    .anisotropyEnable = VK_TRUE,
		    .maxAnisotropy = maxAnisotropy,
		    .compareEnable = VK_FALSE,
		    .compareOp = VK_COMPARE_OP_ALWAYS,
		    .minLod = 0.0f,
		    .maxLod = VK_LOD_CLAMP_NONE,
		    .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		    .unnormalizedCoordinates = VK_FALSE,
		};

		PE_ASSERT(vkCreateSampler(device, &info, nullptr, &m_samplers[4]) == VK_SUCCESS);
	}

	// 5 - s_anisotropicClamp
	{
		const VkSamplerCreateInfo info{
		    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		    .magFilter = VK_FILTER_LINEAR,
		    .minFilter = VK_FILTER_LINEAR,
		    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		    .mipLodBias = 0.0f,
		    .anisotropyEnable = VK_TRUE,
		    .maxAnisotropy = maxAnisotropy,
		    .compareEnable = VK_FALSE,
		    .compareOp = VK_COMPARE_OP_ALWAYS,
		    .minLod = 0.0f,
		    .maxLod = VK_LOD_CLAMP_NONE,
		    .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		    .unnormalizedCoordinates = VK_FALSE,
		};

		PE_ASSERT(vkCreateSampler(device, &info, nullptr, &m_samplers[5]) == VK_SUCCESS);
	}

	// 6 - s_shadow
	{
		constexpr VkSamplerCreateInfo info{
		    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		    .magFilter = VK_FILTER_LINEAR,
		    .minFilter = VK_FILTER_LINEAR,
		    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		    .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		    .mipLodBias = 0.0f,
		    .anisotropyEnable = VK_FALSE,
		    .maxAnisotropy = 1.0f,
		    .compareEnable = VK_TRUE,
		    .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
		    .minLod = 0.0f,
		    .maxLod = VK_LOD_CLAMP_NONE,
		    .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		    .unnormalizedCoordinates = VK_FALSE,
		};

		PE_ASSERT(vkCreateSampler(device, &info, nullptr, &m_samplers[6]) == VK_SUCCESS);
	}

	for (uint32_t i = 0; i < SamplerCount; ++i)
	{
		WriteSampler(device, i, m_samplers[i]);
	}
}

void Prism::Render::Vulkan::VulkanBindlessManager::DestroySamplers(VkDevice device)
{
	for (VkSampler& sampler : m_samplers)
	{
		if (sampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(device, sampler, nullptr);
			sampler = VK_NULL_HANDLE;
		}
	}
}

void Prism::Render::Vulkan::VulkanBindlessManager::CreateResourcesBuffer(VkDevice device, VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	m_resourcesAlignment = std::max<VkDeviceSize>(1, properties.limits.minUniformBufferOffsetAlignment);

	m_resourcesStride = AlignResourcesOffset(ResourcesSize);

	m_resourcesBufferSize = m_resourcesStride * ResourcesRingCapacity;

	const VkBufferCreateInfo bufferInfo{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    .pNext = nullptr,
	    .flags = 0,
	    .size = m_resourcesBufferSize,
	    .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	    .queueFamilyIndexCount = 0,
	    .pQueueFamilyIndices = nullptr,
	};

	PE_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_resourcesBuffer) == VK_SUCCESS);

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(device, m_resourcesBuffer, &memoryRequirements);

	const uint32_t memoryType = FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits,
	                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	PE_ASSERT(memoryType != UINT32_MAX, "Failed to find host visible memory for Resources buffer");

	VkMemoryAllocateInfo allocationInfo{
	    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	    .pNext = nullptr,
	    .allocationSize = memoryRequirements.size,
	    .memoryTypeIndex = memoryType,
	};

	PE_ASSERT(vkAllocateMemory(device, &allocationInfo, nullptr, &m_resourcesMemory) == VK_SUCCESS);

	PE_ASSERT(vkBindBufferMemory(device, m_resourcesBuffer, m_resourcesMemory, 0) == VK_SUCCESS);

	PE_ASSERT(vkMapMemory(device, m_resourcesMemory, 0, m_resourcesBufferSize, 0, &m_resourcesMapped) == VK_SUCCESS);

	std::memset(m_resourcesMapped, 0, m_resourcesBufferSize);

	m_resourcesCursor.store(0);
}

void Prism::Render::Vulkan::VulkanBindlessManager::DestroyResourcesBuffer(VkDevice device)
{
	if (m_resourcesMapped)
	{
		vkUnmapMemory(device, m_resourcesMemory);

		m_resourcesMapped = nullptr;
	}

	if (m_resourcesBuffer)
	{
		vkDestroyBuffer(device, m_resourcesBuffer, nullptr);

		m_resourcesBuffer = VK_NULL_HANDLE;
	}

	if (m_resourcesMemory)
	{
		vkFreeMemory(device, m_resourcesMemory, nullptr);

		m_resourcesMemory = VK_NULL_HANDLE;
	}
}

VkDeviceSize Prism::Render::Vulkan::VulkanBindlessManager::AlignResourcesOffset(VkDeviceSize offset) const
{
	const VkDeviceSize alignment = m_resourcesAlignment;

	PE_ASSERT(alignment != 0);

	return offset + alignment - 1 & ~(alignment - 1);
}

Prism::Render::Vulkan::VulkanBindlessManager::~VulkanBindlessManager()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	DestroySamplers(device);
	DestroyResourcesBuffer(device);

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

	const VkDescriptorBufferInfo bufferInfo{
	    .buffer = buffer,
	    .offset = offset,
	    .range = range,
	};

	// ResourceDescriptorHeap[index]
	{
		const VkWriteDescriptorSet write{
		    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		    .pNext = nullptr,
		    .dstSet = m_set,
		    .dstBinding = ResourceHeapBinding,
		    .dstArrayElement = index,
		    .descriptorCount = 1,
		    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		    .pImageInfo = nullptr,
		    .pBufferInfo = &bufferInfo,
		    .pTexelBufferView = nullptr,
		};

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	// GET_BINDLESS_CBUFFER(...)
	{
		WriteLegacyBuffer(device, index, buffer, offset, range);
	}
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
	    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
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
