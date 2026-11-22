#include "VulkanDescriptorHeap.h"

#include "VulkanRenderDevice.h"
#include "VulkanUtils.h"

namespace Prism::Render::Vulkan
{
bool VulkanDescriptorHeap::Create(VkDeviceSize applicationCapacity, VkDeviceSize heapAddressAlignment,
								  VkDeviceSize reservedOffsetAlignment, VkDeviceSize reservedSize, const std::wstring& bufferName)
{
	if (m_buffer != VK_NULL_HANDLE || m_allocation != VK_NULL_HANDLE)
	{
		return false;
	}

	if (applicationCapacity == 0 || heapAddressAlignment == 0 || reservedOffsetAlignment == 0 || reservedSize == 0)
	{
		return false;
	}

	const auto& renderDevice = VulkanRenderDevice::Get();
	const VkDevice device = renderDevice.GetDevice();
	const VmaAllocator allocator = renderDevice.GetAllocator();

	m_reservedOffset = AlignUp(applicationCapacity, reservedOffsetAlignment);
	m_reservedSize = reservedSize;

	if (m_reservedOffset > std::numeric_limits<VkDeviceSize>::max() - m_reservedSize)
	{
		Destroy();
		return false;
	}

	m_totalSize = m_reservedOffset + m_reservedSize;

	const VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = m_totalSize,
		.usage = VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	const VmaAllocationCreateInfo allocationInfo{
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
		.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	};

	VmaAllocationInfo resultInfo{};

	if (const VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo, &m_buffer, &m_allocation, &resultInfo);
		result != VK_SUCCESS)
	{
		Destroy();
		return false;
	}

	if (!bufferName.empty())
	{
		renderDevice.SetDebugName(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(m_buffer), WStringToString(bufferName));
	}

	m_mapped = resultInfo.pMappedData;

	const VkBufferDeviceAddressInfo addressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = m_buffer,
	};

	m_deviceAddress = vkGetBufferDeviceAddress(device, &addressInfo);

	if (m_deviceAddress == 0 || m_deviceAddress % heapAddressAlignment != 0 || m_mapped == nullptr)
	{
		Destroy();
		return false;
	}

	return true;
}

void VulkanDescriptorHeap::Destroy()
{
	const VmaAllocator allocator = VulkanRenderDevice::Get().GetAllocator();

	if (m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(allocator, m_buffer, m_allocation);
	}

	m_buffer = VK_NULL_HANDLE;
	m_allocation = VK_NULL_HANDLE;

	m_mapped = nullptr;
	m_deviceAddress = 0;

	m_reservedOffset = 0;
	m_reservedSize = 0;
	m_totalSize = 0;
}

VkBindHeapInfoEXT VulkanDescriptorHeap::GetBindHeapInfo() const
{
	PE_ASSERT(m_buffer != VK_NULL_HANDLE);
	PE_ASSERT(m_deviceAddress != 0);
	PE_ASSERT(m_totalSize != 0);

	return VkBindHeapInfoEXT{
		.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT,
		.heapRange =
			{
				.address = m_deviceAddress,
				.size = m_totalSize,
			},
		.reservedRangeOffset = m_reservedOffset,
		.reservedRangeSize = m_reservedSize,
	};
}
} // namespace Prism::Render::Vulkan
