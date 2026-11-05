#include "VulkanBuffer.h"

#include "Prism/Render/RenderConstants.h"
#include "VulkanRenderDevice.h"
#include "VulkanTypeConversions.h"

namespace Prism::Render::Vulkan
{
VulkanBuffer::VulkanBuffer(VulkanRenderDevice* renderDevice, const BufferDesc& desc) :
	Buffer(renderDevice), m_originalDesc(desc)
{
	if (m_originalDesc.bindFlags.HasAllFlags(BindFlags::UniformBuffer))
	{
		m_originalDesc.size = Align(m_originalDesc.size, Constants::UNIFORM_BUFFER_ALIGNMENT);
	}

	const VkBufferCreateInfo bufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = static_cast<VkDeviceSize>(m_originalDesc.size),
		.usage = GetVkBufferUsageFlags(m_originalDesc.bindFlags),
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	const VmaAllocationCreateInfo allocationCreateInfo =
		GetVmaAllocationCreateInfo(m_originalDesc.usage, m_originalDesc.cpuAccess);

	PE_ASSERT(vmaCreateBuffer(renderDevice->GetAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_buffer.buffer,
							  &m_buffer.allocation, nullptr) == VK_SUCCESS);
}

VulkanBuffer::~VulkanBuffer()
{
	if (m_buffer.buffer != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(VulkanRenderDevice::Get().GetAllocator(), m_buffer.buffer, m_buffer.allocation);
	}
}

void* VulkanBuffer::Map(Flags<CPUAccess> access)
{
	PE_ASSERT(m_originalDesc.usage != ResourceUsage::Default);

	void* mappedData{};

	PE_ASSERT(vmaMapMemory(VulkanRenderDevice::Get().GetAllocator(), m_buffer.allocation, &mappedData) == VK_SUCCESS);

	return mappedData;
}

void VulkanBuffer::Unmap()
{
	PE_ASSERT(m_originalDesc.usage != ResourceUsage::Default);

	vmaUnmapMemory(VulkanRenderDevice::Get().GetAllocator(), m_buffer.allocation);
}

std::any VulkanBuffer::GetDynamicAllocation() const
{
	// TODO
	// implement proper ring buffer allocation for dynamic buffers
	return {};
}
} // namespace Prism::Render::Vulkan
