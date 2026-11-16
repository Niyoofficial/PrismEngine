#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Prism::Render::Vulkan
{
class VulkanRenderDevice;

class VulkanDescriptorHeap
{
public:
	VulkanDescriptorHeap() = default;

	VulkanDescriptorHeap(const VulkanDescriptorHeap&) = delete;
	VulkanDescriptorHeap& operator=(const VulkanDescriptorHeap&) = delete;

	[[nodiscard]] bool Create(VkDeviceSize applicationCapacity, VkDeviceSize heapAddressAlignment,
							  VkDeviceSize reservedOffsetAlignment, VkDeviceSize reservedSize);

	void Destroy();

	[[nodiscard]] VkBindHeapInfoEXT GetBindHeapInfo() const;

	[[nodiscard]] void* GetMapped() const { return m_mapped; }

	[[nodiscard]] VkDeviceAddress GetAddress() const { return m_deviceAddress; }

	[[nodiscard]] VkDeviceSize GetReservedOffset() const { return m_reservedOffset; }

	[[nodiscard]] VkDeviceSize GetReservedSize() const { return m_reservedSize; }

	[[nodiscard]] VkDeviceSize GetTotalSize() const { return m_totalSize; }

private:
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VmaAllocation m_allocation = VK_NULL_HANDLE;

	void* m_mapped = nullptr;
	VkDeviceAddress m_deviceAddress = 0;

	VkDeviceSize m_reservedOffset = 0;
	VkDeviceSize m_reservedSize = 0;
	VkDeviceSize m_totalSize = 0;
};
} // namespace Prism::Render::Vulkan
