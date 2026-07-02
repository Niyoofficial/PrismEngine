#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/RenderDevice.h"

namespace Prism::Render::Vulkan
{
class VulkanRenderCommandQueue;

class VulkanRenderDevice : public RenderDevice
{
public:
	static VulkanRenderDevice& Get();
	static VulkanRenderDevice* TryGet();

	explicit VulkanRenderDevice(const RenderDeviceParams& params);
	~VulkanRenderDevice() override;

	Ref<Buffer> CreateBuffer(const BufferDesc& desc) override;

	[[nodiscard]] VkDevice GetDevice() const { return m_device; }

	[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }

	[[nodiscard]] VmaAllocator GetAllocator() const { return m_allocator; }

	[[nodiscard]] VkInstance GetVulkanInstance() const { return m_instance; }

	[[nodiscard]] RenderCommandQueue* GetRenderCommandQueue() const override;

	[[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const { return m_graphicsQueueFamilyIndex; }

private:
	Ref<BufferView> CreateBufferView_Impl(const BufferViewDesc& desc, Buffer* buffer) override;
	Ref<TextureView> CreateTextureView_Impl(const TextureViewDesc& desc, Texture* texture) override;

	void CreateVulkanInstance(const RenderDeviceParams& params);

	void CreateDebugMessenger();

	void PickPhysicalDevice();

	bool AreValidationLayerSupported() const;

	bool AreDeviceExtensionSupported(VkPhysicalDevice physicalDevice) const;

	bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice) const;

	void CreateLogicalDevice();

	void CreateAllocator();

	void CreateRenderCommandQueue(VkQueue queue, uint32_t queueFamilyIndex);

	uint32_t FindGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice) const;

	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	uint32_t m_graphicsQueueFamilyIndex = UINT32_MAX;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

	std::unique_ptr<VulkanRenderCommandQueue> m_commandQueue;

	bool m_initializedImGui = false;
};
} // namespace Prism::Render::Vulkan
