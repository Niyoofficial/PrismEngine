#pragma once

#include "VulkanDynamicGPURingBuffer.h"

#include <vector>

// TODO
// in future need be merged with D3D12DynamicBufferAllocator, as they are basically the same thing, just different APIs
namespace Prism::Render::Vulkan
{
class DynamicBufferAllocator
{
public:
	struct Allocation
	{
		[[nodiscard]] bool IsValid() const { return ringBufferID != -1 && gpuRingAllocation.IsValid(); }

		bool operator==(const Allocation&) const = default;

		int64_t ringBufferID = -1;
		DynamicGPURingBuffer::Allocation gpuRingAllocation{};
	};

	explicit DynamicBufferAllocator(VkDevice device, VmaAllocator allocator, int64_t initialSize, VkDeviceSize alignment,
									VkBufferUsageFlags additionalUsage = 0);

	DynamicBufferAllocator(const DynamicBufferAllocator&) = delete;
	DynamicBufferAllocator& operator=(const DynamicBufferAllocator&) = delete;
	DynamicBufferAllocator(DynamicBufferAllocator&&) = delete;
	DynamicBufferAllocator& operator=(DynamicBufferAllocator&&) = delete;

	~DynamicBufferAllocator() = default;

	[[nodiscard]] Allocation Allocate(int64_t size);

	void CloseCmdListAllocations(uint64_t fenceValue);
	void ReleaseStaleAllocations(uint64_t lastCompletedFenceValue);

	[[nodiscard]] VkBuffer GetVkBuffer(int64_t ringBufferID) const;
	[[nodiscard]] VkDeviceAddress GetDeviceAddress(int64_t ringBufferID) const;

	[[nodiscard]] VkResult Flush(const Allocation& allocation) const;

private:
	struct TrackedRingBuffer
	{
		int64_t ID = -1;
		DynamicGPURingBuffer ringBuffer;

		TrackedRingBuffer(int64_t id, DynamicGPURingBuffer&& ring) : ID(id), ringBuffer(std::move(ring)) {}

		TrackedRingBuffer(TrackedRingBuffer&&) noexcept = default;
		TrackedRingBuffer& operator=(TrackedRingBuffer&&) noexcept = default;

		TrackedRingBuffer(const TrackedRingBuffer&) = delete;
		TrackedRingBuffer& operator=(const TrackedRingBuffer&) = delete;
	};

	VkDevice m_device = VK_NULL_HANDLE;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VkDeviceSize m_alignment = 256;
	VkBufferUsageFlags m_additionalUsage = 0;

	int64_t m_nextRingBufferID = 0;
	int64_t m_initialSize = 0;

	std::vector<TrackedRingBuffer> m_ringBuffers;
};
} // namespace Prism::Render::Vulkan
