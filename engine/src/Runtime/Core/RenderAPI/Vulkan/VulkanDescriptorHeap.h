#pragma once

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

	[[nodiscard]] bool Create(VkDeviceSize applicationSize, VkDeviceSize heapAlignment, VkDeviceSize reservedSize);

	void Destroy();

	[[nodiscard]] VkBindHeapInfoEXT GetBindHeapInfo() const;

	[[nodiscard]] void* GetMapped() const { return m_mapped; }

	[[nodiscard]] VkDeviceAddress GetAddress() const { return m_deviceAddress; }

	[[nodiscard]] VkDeviceSize GetApplicationSize() const { return m_applicationSize; }

	[[nodiscard]] VkDeviceSize GetTotalSize() const { return m_totalSize; }

private:
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;

	void* m_mapped = nullptr;
	VkDeviceAddress m_deviceAddress = 0;

	VkDeviceSize m_applicationSize = 0;
	VkDeviceSize m_reservedSize = 0;
	VkDeviceSize m_totalSize = 0;
};
} // namespace Prism::Render::Vulkan
