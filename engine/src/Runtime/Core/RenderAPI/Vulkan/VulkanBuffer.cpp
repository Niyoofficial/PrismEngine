#include "VulkanBuffer.h"

#include "Prism/Render/RenderConstants.h"
#include "VulkanRenderDevice.h"
#include "VulkanTypeConversions.h"

namespace Prism::Render::Vulkan
{
VulkanBuffer::VulkanBuffer(VulkanRenderDevice* renderDevice, const BufferDesc& desc) : Buffer(renderDevice), m_originalDesc(desc)
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

	if (!m_originalDesc.bufferName.empty())
	{
		renderDevice->SetDebugName(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(m_buffer.buffer),
								   WStringToString(m_originalDesc.bufferName));
	}
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
	PE_ASSERT(!m_isMapped, "Buffer is already mapped");

	if (m_originalDesc.usage == ResourceUsage::Dynamic)
	{
		PE_ASSERT(m_originalDesc.cpuAccess.HasAllFlags(CPUAccess::Write), "Dynamic buffers must support CPU write access");

		auto& device = VulkanRenderDevice::Get();

		m_dynamicAllocation = device.AllocateDynamicBufferMemory(m_originalDesc.size);

		if (!m_dynamicAllocation.IsValid())
		{
			PE_ASSERT(false, "Failed to allocate dynamic buffer memory");
			return nullptr;
		}

		m_isMapped = true;

		return m_dynamicAllocation.gpuRingAllocation.cpuAddress;
	}

	void* mappedData = nullptr;

	const VkResult result = vmaMapMemory(VulkanRenderDevice::Get().GetAllocator(), m_buffer.allocation, &mappedData);

	PE_ASSERT(result == VK_SUCCESS, "Failed to map Vulkan buffer");

	if (result != VK_SUCCESS)
	{
		return nullptr;
	}

	m_isMapped = true;
	m_isVmaMapped = true;

	return mappedData;
}

void VulkanBuffer::Unmap()
{
	PE_ASSERT(m_originalDesc.usage != ResourceUsage::Default);
	PE_ASSERT(m_isMapped, "Buffer is not mapped");

	auto& device = VulkanRenderDevice::Get();

	if (m_originalDesc.usage == ResourceUsage::Dynamic)
	{
		device.FlushDynamicBufferMemory(m_dynamicAllocation);

		m_isMapped = false;
		return;
	}

	if (m_isVmaMapped)
	{
		vmaFlushAllocation(device.GetAllocator(), m_buffer.allocation, 0, VK_WHOLE_SIZE);

		vmaUnmapMemory(device.GetAllocator(), m_buffer.allocation);

		m_isVmaMapped = false;
	}

	m_isMapped = false;
}

std::any VulkanBuffer::GetDynamicAllocation() const { return m_dynamicAllocation; }
} // namespace Prism::Render::Vulkan
