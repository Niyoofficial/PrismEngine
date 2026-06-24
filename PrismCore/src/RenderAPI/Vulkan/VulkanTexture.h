#pragma once

#include <future>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/Texture.h"
#include "vk_mem_alloc.h"

namespace Prism::Render::Vulkan
{
class VulkanRenderDevice;

struct VulkanTextureResource
{
	VkImage image = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
	VmaAllocation allocation = nullptr;

	VkFormat format = VK_FORMAT_UNDEFINED;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t depth = 1;

	uint32_t mipLevels = 1;
	uint32_t arrayLayers = 1;

	VkImageAspectFlags aspectMask = 0;

	VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

class VulkanTexture : public Texture
{
public:
	VulkanTexture(VulkanRenderDevice* renderDevice, const TextureDesc& desc, BarrierLayout initLayout);

	explicit VulkanTexture(VulkanRenderDevice* renderDevice, std::wstring filepath, bool loadAsCubemap = false,
	                       bool waitForLoadFinish = true);

	explicit VulkanTexture(VulkanRenderDevice* renderDevice, std::wstring name, void* imageData, int64_t dataSize,
	                       bool loadAsCubemap = false, bool waitForLoadFinish = true);

	~VulkanTexture() override;

	void WaitForLoadFinish() override;

	[[nodiscard]] TextureDesc GetTextureDesc() const override { return m_originalDesc; }

	[[nodiscard]] VulkanTextureResource GetVulkanTextureResource() const { return m_texture; }

private:
	void CreateImage(const VulkanRenderDevice* renderDevice, const TextureDesc& desc);

	void CreateImageView(const VulkanRenderDevice* renderDevice, const TextureDesc& desc);

	void CreateSampler(VulkanRenderDevice* renderDevice, const TextureDesc& desc);

	void UploadTextureData(const VulkanRenderDevice* renderDevice, const void* pixels, VkDeviceSize imageSize);

	void GenerateMipMaps(VkCommandBuffer cmd);

	TextureDesc m_originalDesc;

	VulkanTextureResource m_texture;

	std::future<void> m_loadFuture;
};
} // namespace Prism::Render::Vulkan
