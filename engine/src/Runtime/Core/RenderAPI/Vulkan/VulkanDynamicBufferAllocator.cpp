#include "VulkanDynamicBufferAllocator.h"

#include <algorithm>
#include <utility>

namespace Prism::Render::Vulkan
{

DynamicBufferAllocator::DynamicBufferAllocator(VkDevice device, VmaAllocator allocator, int64_t initialSize,
											   VkDeviceSize alignment, VkBufferUsageFlags additionalUsage)
{
	PE_ASSERT(m_ringBuffers.empty(), "DynamicBufferAllocator::Init called more than once");

	PE_ASSERT(initialSize > 0, "DynamicBufferAllocator::Init requires positive initialSize");

	m_device = device;
	m_allocator = allocator;
	m_initialSize = initialSize;
	m_alignment = alignment;
	m_additionalUsage = additionalUsage;

	DynamicGPURingBuffer ring(m_device, m_allocator, m_initialSize, m_alignment, m_additionalUsage);

	m_ringBuffers.emplace_back(m_nextRingBufferID++, std::move(ring));
}

DynamicBufferAllocator::Allocation DynamicBufferAllocator::Allocate(int64_t size)
{
	auto allocation = m_ringBuffers.back().ringBuffer.Allocate(size);

	if (!allocation.IsValid())
	{
		const int64_t currentMaxSize = m_ringBuffers.back().ringBuffer.GetMaxSize();

		int64_t newMaxSize = currentMaxSize > 0 ? currentMaxSize * 2 : m_initialSize;

		while (newMaxSize < size)
		{
			if (newMaxSize > INT64_MAX / 2)
			{
				PE_ASSERT_NO_ENTRY("DynamicBufferAllocator ring size overflow");
				break;
			}

			newMaxSize *= 2;
		}

		DynamicGPURingBuffer newRing(m_device, m_allocator, newMaxSize, m_alignment, m_additionalUsage);

		m_ringBuffers.emplace_back(m_nextRingBufferID++, std::move(newRing));

		allocation = m_ringBuffers.back().ringBuffer.Allocate(size);
		PE_ASSERT(allocation.IsValid());
	}

	return {
		.ringBufferID = m_ringBuffers.back().ID,
		.gpuRingAllocation = allocation,
	};
}

void DynamicBufferAllocator::CloseCmdListAllocations(uint64_t fenceValue)
{
	for (auto& ringBuffer : m_ringBuffers)
	{
		ringBuffer.ringBuffer.CloseCmdListAllocations(fenceValue);
	}
}

void DynamicBufferAllocator::ReleaseStaleAllocations(uint64_t lastCompletedFenceValue)
{
	size_t numBuffersToDelete = 0;

	for (size_t i = 0; i < m_ringBuffers.size(); ++i)
	{
		auto& ringBuffer = m_ringBuffers[i].ringBuffer;
		ringBuffer.ReleaseCompletedResources(lastCompletedFenceValue);

		if (numBuffersToDelete == i && i < m_ringBuffers.size() - 1 && ringBuffer.IsEmpty())
		{
			++numBuffersToDelete;
		}
	}

	if (numBuffersToDelete > 0)
	{
		m_ringBuffers.erase(m_ringBuffers.begin(), m_ringBuffers.begin() + static_cast<std::ptrdiff_t>(numBuffersToDelete));
	}
}

VkBuffer DynamicBufferAllocator::GetVkBuffer(int64_t ringBufferID) const
{
	const auto it = std::ranges::find_if(m_ringBuffers,
										 [ringBufferID](const TrackedRingBuffer& element) { return element.ID == ringBufferID; });

	return it != m_ringBuffers.end() ? it->ringBuffer.GetVkBuffer() : VK_NULL_HANDLE;
}

VkDeviceAddress DynamicBufferAllocator::GetDeviceAddress(int64_t ringBufferID) const
{
	const auto it = std::ranges::find_if(m_ringBuffers,
										 [ringBufferID](const TrackedRingBuffer& element) { return element.ID == ringBufferID; });

	return it != m_ringBuffers.end() ? it->ringBuffer.GetDeviceAddress() : 0;
}

VkResult DynamicBufferAllocator::Flush(const Allocation& allocation) const
{
	if (!allocation.IsValid())
	{
		return VK_ERROR_MEMORY_MAP_FAILED;
	}

	const auto it = std::ranges::find_if(m_ringBuffers, [&allocation](const TrackedRingBuffer& element)
										 { return element.ID == allocation.ringBufferID; });

	if (it == m_ringBuffers.end())
	{
		return VK_ERROR_MEMORY_MAP_FAILED;
	}

	return it->ringBuffer.Flush(allocation.gpuRingAllocation);
}
} // namespace Prism::Render::Vulkan
