#include "VulkanBuffer.h"

#include "VulkanRenderDevice.h"
#include "VulkanTypeConversions.h"

Prism::Render::Vulkan::VulkanBuffer::VulkanBuffer(VulkanRenderDevice* renderDevice, const BufferDesc& desc) :
    Buffer(renderDevice), m_originalDesc(desc)
{
	const VkBufferCreateInfo bufferCreateInfo{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    .size = static_cast<VkDeviceSize>(desc.size),
	    .usage = GetVkBufferUsageFlags(desc.bindFlags),
	    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	const VmaAllocationCreateInfo allocationCreateInfo = GetVmaAllocationCreateInfo(desc.usage, desc.cpuAccess);

	PE_ASSERT(vmaCreateBuffer(renderDevice->GetAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_buffer.buffer,
	                          &m_buffer.allocation, nullptr) == VK_SUCCESS);
}

Prism::Render::Vulkan::VulkanBuffer::~VulkanBuffer()
{
	if (m_buffer.buffer != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(VulkanRenderDevice::Get().GetAllocator(), m_buffer.buffer, m_buffer.allocation);
	}
}

void* Prism::Render::Vulkan::VulkanBuffer::Map(Flags<CPUAccess> access)
{
	PE_ASSERT(m_originalDesc.usage != ResourceUsage::Default);

	void* mappedData{};

	PE_ASSERT(vmaMapMemory(VulkanRenderDevice::Get().GetAllocator(), m_buffer.allocation, &mappedData) == VK_SUCCESS);

	return mappedData;
}

void Prism::Render::Vulkan::VulkanBuffer::Unmap()
{
	PE_ASSERT(m_originalDesc.usage != ResourceUsage::Default);

	vmaUnmapMemory(VulkanRenderDevice::Get().GetAllocator(), m_buffer.allocation);
}

std::any Prism::Render::Vulkan::VulkanBuffer::GetDynamicAllocation() const
{
	// TODO
	// implement proper ring buffer allocation for dynamic buffers
	return {};
}
