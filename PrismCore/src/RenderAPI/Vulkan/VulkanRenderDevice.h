#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "Prism/Render/RenderDevice.h"
#include "VulkanBindlessManager.h"
#include "VulkanPipelineCache.h"
#include "VulkanShaderCompiler.h"

namespace Prism::Render::Vulkan
{
class VulkanDescriptorSetLayoutCache;
class VulkanPipelineLayoutCache;
class VulkanRenderCommandQueue;

class VulkanRenderDevice : public RenderDevice
{
public:
	static VulkanRenderDevice& Get();
	static VulkanRenderDevice* TryGet();

	explicit VulkanRenderDevice(const RenderDeviceParams& params);
	~VulkanRenderDevice() override;

	[[nodiscard]] Ref<Buffer> CreateBuffer(const BufferDesc& desc) override;

	[[nodiscard]] Ref<Texture> CreateTexture(const TextureDesc& desc, BarrierLayout initLayout) override;

	[[nodiscard]] Ref<Texture> CreateTexture(std::wstring filepath, bool loadAsCubemap, bool waitForLoadFinish) override;

	[[nodiscard]] Ref<Texture> CreateTexture(std::wstring name, void* imageData, int64_t dataSize, bool loadAsCubemap,
	                                         bool waitForLoadFinish) override;

	[[nodiscard]] int64_t GetTotalSizeInBytes(BufferDesc buffDesc) const override;

	[[nodiscard]] int64_t GetTotalSizeInBytes(TextureDesc texDesc, int32_t firstSubresource,
	                                          int32_t numSubresources) const override;

	[[nodiscard]] SubresourceFootprint GetSubresourceFootprint(TextureDesc texDesc, int32_t subresourceIndex) const override;

	[[nodiscard]] int64_t GetTexturePitchAlignment() const override;

	void InitializeImGui(Core::Window* window, TextureFormat depthFormat) override;

	void ShutdownImGui() override;

	void ImGuiNewFrame() override;

	[[nodiscard]] VkDevice GetDevice() const { return m_device; }

	[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }

	[[nodiscard]] VmaAllocator GetAllocator() const { return m_allocator; }

	[[nodiscard]] VkInstance GetVulkanInstance() const { return m_instance; }

	[[nodiscard]] RenderCommandQueue* GetRenderCommandQueue() const override;

	[[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const { return m_graphicsQueueFamilyIndex; }

	[[nodiscard]] VulkanPipelineLayoutCache* GetPipelineLayoutCache() const { return m_pipelineLayoutCache.get(); }

	[[nodiscard]] VulkanPipelineCache* GetPipelineCache() const { return m_pipelineCache.get(); }

	[[nodiscard]] VulkanShaderCompiler* GetVulkanShaderCompiler() const
	{
		return static_cast<VulkanShaderCompiler*>(GetShaderCompiler());
	}

	[[nodiscard]] VulkanRenderCommandQueue* GetVulkanRenderCommandQueue() const { return m_commandQueue.get(); }

	[[nodiscard]] VulkanBindlessManager& GetBindlessManager() { return m_bindlessManager; }

private:
	Ref<BufferView> CreateBufferView_Impl(const BufferViewDesc& desc, Buffer* buffer) override;
	Ref<TextureView> CreateTextureView_Impl(const TextureViewDesc& desc, Texture* texture) override;

	void CreateVulkanInstance(const RenderDeviceParams& params);

	void CreateDebugMessenger();

	void PickPhysicalDevice();

	[[nodiscard]] bool AreValidationLayerSupported() const;

	bool AreDeviceExtensionSupported(VkPhysicalDevice physicalDevice) const;

	bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice) const;

	void CreateLogicalDevice();

	void CreateAllocator();

	void CreateRenderCommandQueue(VkQueue queue, uint32_t queueFamilyIndex);

	uint32_t FindGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice) const;

	void CreateDescriptorSetLayoutCache();

	void CreatePipelineLayoutCache();

	void CreatePipelineCache();

	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	uint32_t m_graphicsQueueFamilyIndex = UINT32_MAX;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

	std::unique_ptr<VulkanRenderCommandQueue> m_commandQueue;
	std::unique_ptr<VulkanDescriptorSetLayoutCache> m_descriptorSetLayoutCache;
	std::unique_ptr<VulkanPipelineLayoutCache> m_pipelineLayoutCache;
	std::unique_ptr<VulkanPipelineCache> m_pipelineCache;
	VulkanBindlessManager m_bindlessManager;

	bool m_initializedImGui = false;
};
} // namespace Prism::Render::Vulkan
