#include "VulkanDescriptorHeap.h"

#include "VulkanRenderDevice.h"

namespace Prism::Render::Vulkan
{
static VkDeviceSize AlignUp(VkDeviceSize value, VkDeviceSize alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

bool VulkanDescriptorHeap::Create(VkDeviceSize applicationSize, VkDeviceSize heapAlignment, VkDeviceSize reservedSize)
{
	const VkDevice device = VulkanRenderDevice::Get().GetDevice();
	const VkPhysicalDevice physicalDevice = VulkanRenderDevice::Get().GetPhysicalDevice();

	m_reservedSize = reservedSize;
	m_applicationSize = AlignUp(applicationSize, heapAlignment);
	m_totalSize = m_applicationSize + m_reservedSize;

	VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
								  .size = m_totalSize,
								  .usage = VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
								  .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS)
	{
		return false;
	}

	VkMemoryRequirements requirements{};
	vkGetBufferMemoryRequirements(device, m_buffer, &requirements);

	VkPhysicalDeviceMemoryProperties memoryProperties{};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	uint32_t memoryType = UINT32_MAX;

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((requirements.memoryTypeBits & (1u << i)) == 0)
		{
			continue;
		}

		const auto flags = memoryProperties.memoryTypes[i].propertyFlags;
		const auto required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		if ((flags & required) == required)
		{
			memoryType = i;
			break;
		}
	}

	if (memoryType == UINT32_MAX)
	{
		return false;
	}

	VkMemoryAllocateFlagsInfo flagsInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
	};

	VkMemoryAllocateInfo allocationInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &flagsInfo,
		.allocationSize = requirements.size,
		.memoryTypeIndex = memoryType,
	};

	if (vkAllocateMemory(device, &allocationInfo, nullptr, &m_memory) != VK_SUCCESS)
	{
		return false;
	}

	if (vkBindBufferMemory(device, m_buffer, m_memory, 0) != VK_SUCCESS)
	{
		return false;
	}

	if (vkMapMemory(device, m_memory, 0, requirements.size, 0, &m_mapped) != VK_SUCCESS)
	{
		return false;
	}

	VkBufferDeviceAddressInfo addressInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = m_buffer};

	m_deviceAddress = vkGetBufferDeviceAddress(device, &addressInfo);

	if (m_deviceAddress == 0)
	{
		return false;
	}

	std::memset(m_mapped, 0, requirements.size);

	return true;
}

void VulkanDescriptorHeap::Destroy()
{
	const VkDevice device = VulkanRenderDevice::Get().GetDevice();

	if (m_mapped)
	{
		vkUnmapMemory(device, m_memory);
		m_mapped = nullptr;
	}

	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}

	if (m_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}

	m_deviceAddress = 0;
	m_totalSize = 0;
	m_applicationSize = 0;
	m_reservedSize = 0;
}

VkBindHeapInfoEXT VulkanDescriptorHeap::GetBindHeapInfo() const
{
	return VkBindHeapInfoEXT{
		.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT,
		.heapRange =
			{
				.address = m_deviceAddress,
				.size = m_totalSize,
			},
		.reservedRangeOffset = m_applicationSize,
		.reservedRangeSize = m_reservedSize,
	};
}
} // namespace Prism::Render::Vulkan
