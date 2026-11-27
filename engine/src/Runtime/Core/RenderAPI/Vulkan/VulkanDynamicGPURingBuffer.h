#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include <cstdint>
#include <deque>

// TODO
// in future need be merged with D3D12DynamicGPURingBuffer, as they are basically the same thing, just different APIs
namespace Prism::Render::Vulkan
{
class DynamicGPURingBuffer
{
public:
	static constexpr int64_t INVALID_OFFSET = -1;

	struct CompletedTailAttributes
	{
		uint64_t fenceValue = 0;
		int64_t offset = INVALID_OFFSET;
		int64_t size = INVALID_OFFSET;
	};

	struct Allocation
	{
		bool IsValid() const;

		bool operator==(const Allocation&) const = default;

		int64_t offset = INVALID_OFFSET;
		int64_t size = INVALID_OFFSET;
		void* cpuAddress = nullptr;
		VkDeviceAddress gpuAddress = 0;
	};

	DynamicGPURingBuffer(VkDevice device, VmaAllocator allocator, int64_t maxSize, VkDeviceSize alignment = 256,
						 VkBufferUsageFlags additionalUsage = 0);

	DynamicGPURingBuffer(DynamicGPURingBuffer&& rhs) noexcept;
	DynamicGPURingBuffer& operator=(DynamicGPURingBuffer&& rhs) noexcept;

	DynamicGPURingBuffer(const DynamicGPURingBuffer&) = delete;
	DynamicGPURingBuffer& operator=(const DynamicGPURingBuffer&) = delete;

	~DynamicGPURingBuffer();

	[[nodiscard]] Allocation Allocate(int64_t size);

	void CloseCmdListAllocations(uint64_t fenceValue);
	void ReleaseCompletedResources(uint64_t completedFenceValue);

	[[nodiscard]] VkResult Flush(const Allocation& allocation) const;

	[[nodiscard]] VkBuffer GetVkBuffer() const { return m_buffer; }

	[[nodiscard]] VkDeviceAddress GetDeviceAddress() const { return m_deviceAddress; }

	[[nodiscard]] int64_t GetMaxSize() const { return m_maxSize; }

	[[nodiscard]] int64_t GetUsedSize() const { return m_usedSize; }

	[[nodiscard]] bool IsFull() const { return m_usedSize == m_maxSize; }

	[[nodiscard]] bool IsEmpty() const { return m_usedSize == 0; }

private:
	void Reset() noexcept;
	void MoveFrom(DynamicGPURingBuffer&& rhs) noexcept;

	VkDevice m_device = VK_NULL_HANDLE;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VmaAllocation m_vmaAllocation = VK_NULL_HANDLE;
	void* m_cpuAddress = nullptr;
	VkDeviceAddress m_deviceAddress = 0;

	std::deque<CompletedTailAttributes> m_completedTails;

	int64_t m_head = 0;
	int64_t m_tail = 0;
	int64_t m_maxSize = 0;
	int64_t m_usedSize = 0;
	int64_t m_currentCmdListSize = 0;
	VkDeviceSize m_alignment = 256;
};

} // namespace Prism::Render::Vulkan
