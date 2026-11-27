#include "VulkanDynamicGPURingBuffer.h"

#include <algorithm>
#include <utility>

#include "VulkanUtils.h"

namespace Prism::Render::Vulkan
{
bool DynamicGPURingBuffer::Allocation::IsValid() const
{
	return offset != INVALID_OFFSET && size > 0 && cpuAddress != nullptr && gpuAddress != 0;
}

DynamicGPURingBuffer::DynamicGPURingBuffer(VkDevice device, VmaAllocator allocator, int64_t maxSize, VkDeviceSize alignment,
										   VkBufferUsageFlags additionalUsage) :
	m_device(device), m_allocator(allocator), m_alignment(alignment)
{
	m_maxSize = maxSize - (maxSize % static_cast<int64_t>(alignment));
	PE_ASSERT(m_maxSize > 0, "DynamicGPURingBuffer: adjusted maxSize is not positive");

	const VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = static_cast<VkDeviceSize>(m_maxSize),
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | additionalUsage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	constexpr VmaAllocationCreateInfo allocationInfo{
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	VmaAllocationInfo actualAllocationInfo{};
	const VkResult createResult =
		vmaCreateBuffer(m_allocator, &bufferInfo, &allocationInfo, &m_buffer, &m_vmaAllocation, &actualAllocationInfo);
	PE_ASSERT(createResult == VK_SUCCESS);

	m_cpuAddress = actualAllocationInfo.pMappedData;
	PE_ASSERT(m_cpuAddress);

	const VkBufferDeviceAddressInfo addressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = m_buffer,
	};
	m_deviceAddress = vkGetBufferDeviceAddress(m_device, &addressInfo);

	PE_ASSERT(m_deviceAddress != 0);
}

DynamicGPURingBuffer::DynamicGPURingBuffer(DynamicGPURingBuffer&& rhs) noexcept { MoveFrom(std::move(rhs)); }

DynamicGPURingBuffer& DynamicGPURingBuffer::operator=(DynamicGPURingBuffer&& rhs) noexcept
{
	if (this != &rhs)
	{
		Reset();
		MoveFrom(std::move(rhs));
	}
	return *this;
}

void DynamicGPURingBuffer::MoveFrom(DynamicGPURingBuffer&& rhs) noexcept
{
	m_device = std::exchange(rhs.m_device, VK_NULL_HANDLE);
	m_allocator = std::exchange(rhs.m_allocator, VK_NULL_HANDLE);
	m_buffer = std::exchange(rhs.m_buffer, VK_NULL_HANDLE);
	m_vmaAllocation = std::exchange(rhs.m_vmaAllocation, VK_NULL_HANDLE);
	m_cpuAddress = std::exchange(rhs.m_cpuAddress, nullptr);
	m_deviceAddress = std::exchange(rhs.m_deviceAddress, 0);

	m_completedTails = std::move(rhs.m_completedTails);

	m_head = std::exchange(rhs.m_head, 0);
	m_tail = std::exchange(rhs.m_tail, 0);
	m_maxSize = std::exchange(rhs.m_maxSize, 0);
	m_usedSize = std::exchange(rhs.m_usedSize, 0);
	m_currentCmdListSize = std::exchange(rhs.m_currentCmdListSize, 0);
	m_alignment = std::exchange(rhs.m_alignment, 256);
}

void DynamicGPURingBuffer::Reset() noexcept
{
	if (m_buffer != VK_NULL_HANDLE && m_allocator != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(m_allocator, m_buffer, m_vmaAllocation);
	}

	m_device = VK_NULL_HANDLE;
	m_allocator = VK_NULL_HANDLE;
	m_buffer = VK_NULL_HANDLE;
	m_vmaAllocation = VK_NULL_HANDLE;
	m_cpuAddress = nullptr;
	m_deviceAddress = 0;
	m_completedTails.clear();

	m_head = 0;
	m_tail = 0;
	m_maxSize = 0;
	m_usedSize = 0;
	m_currentCmdListSize = 0;
}

DynamicGPURingBuffer::~DynamicGPURingBuffer() { Reset(); }

DynamicGPURingBuffer::Allocation DynamicGPURingBuffer::Allocate(const int64_t size)
{
	if (size <= 0 || IsFull() || m_buffer == VK_NULL_HANDLE)
	{
		return {};
	}

	const auto alignedSize = static_cast<int64_t>(AlignUp(size, m_alignment));
	if (alignedSize == 0)
	{
		return {};
	}

	if (alignedSize > m_maxSize || m_maxSize - m_usedSize < alignedSize)
	{
		return {};
	}

	int64_t currentOffset = INVALID_OFFSET;

	if (m_tail >= m_head)
	{
		if (m_tail + alignedSize <= m_maxSize)
		{
			currentOffset = m_tail;
			m_tail += alignedSize;
			m_usedSize += alignedSize;
			m_currentCmdListSize += alignedSize;
		}
		else if (alignedSize <= m_head)
		{
			const int64_t consumed = m_maxSize - m_tail + alignedSize;
			m_usedSize += consumed;
			m_currentCmdListSize += consumed;
			m_tail = alignedSize;
			currentOffset = 0;
		}
	}
	else if (m_tail + alignedSize <= m_head)
	{
		currentOffset = m_tail;
		m_tail += alignedSize;
		m_usedSize += alignedSize;
		m_currentCmdListSize += alignedSize;
	}

	if (currentOffset == INVALID_OFFSET)
	{
		return {};
	}

	auto* cpu = static_cast<uint8_t*>(m_cpuAddress) + currentOffset;

	return {
		.offset = currentOffset,
		.size = alignedSize,
		.cpuAddress = cpu,
		.gpuAddress = m_deviceAddress + static_cast<VkDeviceAddress>(currentOffset),
	};
}

void DynamicGPURingBuffer::CloseCmdListAllocations(const uint64_t fenceValue)
{
	if (m_currentCmdListSize == 0)
	{
		return;
	}

	m_completedTails.push_back({
		.fenceValue = fenceValue,
		.offset = m_tail,
		.size = m_currentCmdListSize,
	});

	m_currentCmdListSize = 0;
}

void DynamicGPURingBuffer::ReleaseCompletedResources(const uint64_t completedFenceValue)
{
	while (!m_completedTails.empty() && m_completedTails.front().fenceValue <= completedFenceValue)
	{
		const CompletedTailAttributes& oldest = m_completedTails.front();
		PE_ASSERT(oldest.size <= m_usedSize);
		m_usedSize -= oldest.size;
		m_head = oldest.offset;
		m_completedTails.pop_front();
	}

	if (m_usedSize == 0 && m_currentCmdListSize == 0 && m_completedTails.empty())
	{
		m_head = 0;
		m_tail = 0;
	}
}

VkResult DynamicGPURingBuffer::Flush(const Allocation& allocation) const
{
	if (!allocation.IsValid() || allocation.offset < 0 || allocation.size <= 0 || allocation.offset + allocation.size > m_maxSize)
	{
		return VK_ERROR_MEMORY_MAP_FAILED;
	}

	return vmaFlushAllocation(m_allocator, m_vmaAllocation, static_cast<VkDeviceSize>(allocation.offset),
							  static_cast<VkDeviceSize>(allocation.size));
}
} // namespace Prism::Render::Vulkan
