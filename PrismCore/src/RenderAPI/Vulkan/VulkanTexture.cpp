#include "VulkanTexture.h"

#include "VulkanBuffer.h"
#include "VulkanRenderCommandList.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderDevice.h"
#include "VulkanTypeConversions.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Prism::Render::Vulkan::VulkanTexture::VulkanTexture(VulkanRenderDevice* renderDevice, const TextureDesc& desc,
                                                    BarrierLayout initLayout) : Texture(renderDevice), m_originalDesc(desc)
{
	CreateImage(renderDevice, desc);
	CreateImageView(renderDevice, desc);
	CreateSampler(renderDevice, desc);
}

Prism::Render::Vulkan::VulkanTexture::VulkanTexture(VulkanRenderDevice* renderDevice, std::wstring filepath, bool loadAsCubemap,
                                                    const bool waitForLoadFinish) : Texture(renderDevice)
{
	auto loadFunction = [this, renderDevice, filepath]
	{
		int width = 0;
		int height = 0;
		int channels = 0;
		void* pixels{};
		size_t bytesPerChannel = 0;

		const std::string path = WStringToString(filepath);

		if (stbi_is_hdr(path.c_str()))
		{
			pixels = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			bytesPerChannel = sizeof(float);
		}
		else
		{
			pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			bytesPerChannel = 1;
		}

		if (!pixels)
		{
			PE_RENDER_LOG(Error, "Failed loading texture {}", path);

			stbi_image_free(pixels);

			return;
		}

		const auto mipLevels = static_cast<int32_t>(std::floor(std::log2(std::max(width, height))) + 1);

		m_originalDesc = TextureDesc::CreateTex2D(filepath, width, height, TextureFormat::RGBA8_UNorm, BindFlags::ShaderResource,
		                                          ResourceUsage::Default, mipLevels);

		// should it be part of texture desc?
		const VkDeviceSize imageSize = width * height * STBI_rgb_alpha * bytesPerChannel;

		CreateImage(renderDevice, m_originalDesc);
		CreateImageView(renderDevice, m_originalDesc);
		CreateSampler(renderDevice, m_originalDesc);

		UploadTextureData(renderDevice, pixels, imageSize);

		stbi_image_free(pixels);
	};

	if (waitForLoadFinish)
	{
		loadFunction();
	}
	else
	{
		m_loadFuture = std::async(std::launch::async, loadFunction);
	}
}

Prism::Render::Vulkan::VulkanTexture::VulkanTexture(VulkanRenderDevice* renderDevice, std::wstring name, void* imageData,
                                                    int64_t dataSize, const bool loadAsCubemap, const bool waitForLoadFinish) :
    Texture(renderDevice)
{
	auto loadFunction = [this, renderDevice, name, imageData, dataSize]
	{
		int width = 0;
		int height = 0;
		int channels = 0;

		stbi_uc* pixels = stbi_load_from_memory(static_cast<stbi_uc*>(imageData), static_cast<int>(dataSize), &width, &height,
		                                        &channels, STBI_rgb_alpha);

		if (!pixels)
		{
			PE_RENDER_LOG(Error, "Failed loading texture from memory");

			stbi_image_free(pixels);

			return;
		}

		const auto mipLevels = static_cast<int32_t>(std::floor(std::log2(std::max(width, height))) + 1);

		m_originalDesc = TextureDesc::CreateTex2D(name, width, height, TextureFormat::RGBA8_UNorm, BindFlags::ShaderResource,
		                                          ResourceUsage::Default, mipLevels);

		const VkDeviceSize imageSize = width * height * STBI_rgb_alpha;

		CreateImage(renderDevice, m_originalDesc);
		CreateImageView(renderDevice, m_originalDesc);
		CreateSampler(renderDevice, m_originalDesc);

		UploadTextureData(renderDevice, pixels, imageSize);

		stbi_image_free(pixels);
	};

	if (waitForLoadFinish)
	{
		loadFunction();
	}
	else
	{
		m_loadFuture = std::async(std::launch::async, loadFunction);
	}
}

Prism::Render::Vulkan::VulkanTexture::~VulkanTexture()
{
	VulkanTexture::WaitForLoadFinish();

	const auto* device = dynamic_cast<VulkanRenderDevice*>(m_renderDevice);

	if (m_texture.image)
	{
		vmaDestroyImage(device->GetAllocator(), m_texture.image, m_texture.allocation);

		m_texture.image = VK_NULL_HANDLE;
		m_texture.allocation = nullptr;
	}

	if (m_texture.imageView)
	{
		vkDestroyImageView(device->GetDevice(), m_texture.imageView, nullptr);
		m_texture.imageView = VK_NULL_HANDLE;
	}

	if (m_texture.sampler)
	{
		vkDestroySampler(device->GetDevice(), m_texture.sampler, nullptr);
		m_texture.sampler = VK_NULL_HANDLE;
	}
}

void Prism::Render::Vulkan::VulkanTexture::WaitForLoadFinish()
{
	if (m_loadFuture.valid())
	{
		m_loadFuture.wait();
		m_loadFuture = std::future<void>();
	}
}

void Prism::Render::Vulkan::VulkanTexture::CreateImage(const VulkanRenderDevice* renderDevice, const TextureDesc& desc)
{
	VkImageCreateInfo imageCreateInfo{
	    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	    .imageType = GetVkImageType(desc.dimension),
	    .format = GetVkFormat(desc.format),
	    .extent{
	        .width = static_cast<uint32_t>(desc.width),
	        .height = static_cast<uint32_t>(desc.height),
	        .depth = desc.Is3D() ? static_cast<uint32_t>(desc.GetDepth()) : 1,
	    },
	    .mipLevels = static_cast<uint32_t>(desc.mipLevels),
	    .arrayLayers = desc.Is3D() ? 1 : static_cast<uint32_t>(desc.GetArraySize()),
	    .samples = static_cast<VkSampleCountFlagBits>(desc.sampleDesc.count),
	    .tiling = VK_IMAGE_TILING_OPTIMAL,
	    .usage = GetVkImageUsageFlags(desc.bindFlags),
	    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	if (desc.IsCube())
	{
		imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}
	if (desc.GetArraySize() > 1)
	{
		imageCreateInfo.arrayLayers = desc.GetArraySize();
	}

	constexpr VmaAllocationCreateInfo imageAllocInfo{
	    .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
	};

	const VkResult result = vmaCreateImage(renderDevice->GetAllocator(), &imageCreateInfo, &imageAllocInfo, &m_texture.image,
	                                       &m_texture.allocation, nullptr);

	PE_ASSERT(result == VK_SUCCESS);

	m_texture.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

#ifdef VK_EXT_debug_utils
	if (!desc.textureName.empty())
	{
		const std::string utf8Name = WStringToString(desc.textureName);

		const VkDebugUtilsObjectNameInfoEXT nameInfo{
		    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		    .objectType = VK_OBJECT_TYPE_IMAGE,
		    .objectHandle = reinterpret_cast<uint64_t>(m_texture.image),
		    .pObjectName = utf8Name.c_str(),
		};
		const auto function = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
		    vkGetDeviceProcAddr(renderDevice->GetDevice(), "vkSetDebugUtilsObjectNameEXT"));

		if (function)
		{
			function(renderDevice->GetDevice(), &nameInfo);
		}
	}
#endif
}

void Prism::Render::Vulkan::VulkanTexture::CreateImageView(const VulkanRenderDevice* renderDevice, const TextureDesc& desc)
{
	const VkImageViewCreateInfo info{
	    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	    .image = m_texture.image,
	    .viewType = GetVkImageViewType(desc.dimension, desc.GetArraySize()),
	    .format = GetVkFormat(m_originalDesc.format),
	    .subresourceRange =
	        {
	            .aspectMask = GetVkImageAspectFlags(m_originalDesc.format),
	            .baseMipLevel = 0,
	            .levelCount = static_cast<uint32_t>(m_originalDesc.mipLevels),
	            .baseArrayLayer = 0,
	            .layerCount = m_originalDesc.Is3D() ? 1 : static_cast<uint32_t>(m_originalDesc.GetArraySize()),
	        },
	};

	PE_ASSERT(vkCreateImageView(renderDevice->GetDevice(), &info, nullptr, &m_texture.imageView) == VK_SUCCESS);
}

void Prism::Render::Vulkan::VulkanTexture::CreateSampler(VulkanRenderDevice* renderDevice, const TextureDesc& desc)
{
	constexpr VkSamplerCreateInfo info{
	    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	    .magFilter = VK_FILTER_LINEAR,
	    .minFilter = VK_FILTER_LINEAR,
	    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	    .maxLod = VK_LOD_CLAMP_NONE,
	};

	PE_ASSERT(vkCreateSampler(renderDevice->GetDevice(), &info, nullptr, &m_texture.sampler) == VK_SUCCESS);
}

void Prism::Render::Vulkan::VulkanTexture::UploadTextureData(const VulkanRenderDevice* renderDevice, const void* pixels,
                                                             const VkDeviceSize imageSize)
{
	VkBuffer stagingBuffer{};
	VmaAllocation stagingAllocation{};

	const VkBufferCreateInfo stagingBufferInfo{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    .size = imageSize,
	    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};

	constexpr VmaAllocationCreateInfo stagingAllocInfo{
	    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
	    .usage = VMA_MEMORY_USAGE_AUTO,
	};

	PE_ASSERT(vmaCreateBuffer(renderDevice->GetAllocator(), &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer,
	                          &stagingAllocation, nullptr) == VK_SUCCESS);

	void* mappedData{};
	vmaMapMemory(renderDevice->GetAllocator(), stagingAllocation, &mappedData);
	memcpy(mappedData, pixels, imageSize);
	vmaUnmapMemory(renderDevice->GetAllocator(), stagingAllocation);

	auto cmd = std::make_unique<VulkanRenderCommandList>();

	VkCommandBuffer vkCmd = cmd->GetVkCommandBuffer();

	VkImageMemoryBarrier barrier{
	    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	    .image = m_texture.image,
	    .subresourceRange{
	        .aspectMask = GetVkImageAspectFlags(m_originalDesc.format),
	        .baseMipLevel = 0,
	        .levelCount = static_cast<uint32_t>(m_originalDesc.mipLevels),
	        .baseArrayLayer = 0,
	        .layerCount = m_originalDesc.Is3D() ? 1 : static_cast<uint32_t>(m_originalDesc.GetArraySize()),
	    },
	};

	vkCmdPipelineBarrier(vkCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
	                     &barrier);

	m_texture.currentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	const VkBufferImageCopy region{
	    .imageSubresource =
	        {
	            .aspectMask = GetVkImageAspectFlags(m_originalDesc.format),
	            .mipLevel = 0,
	            .baseArrayLayer = 0,
	            .layerCount = m_originalDesc.Is3D() ? 1 : static_cast<uint32_t>(m_originalDesc.GetArraySize()),
	        },
	    .imageExtent =
	        {
	            .width = static_cast<uint32_t>(m_originalDesc.width),
	            .height = static_cast<uint32_t>(m_originalDesc.height),
	            .depth = m_originalDesc.Is3D() ? static_cast<uint32_t>(m_originalDesc.GetDepth()) : 1,
	        },
	};

	vkCmdCopyBufferToImage(vkCmd, stagingBuffer, m_texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	if (m_originalDesc.mipLevels > 1)
	{
		GenerateMipMaps(vkCmd);
	}
	else
	{
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkCmdPipelineBarrier(vkCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
		                     nullptr, 1, &barrier);
	}

	const auto vulkanQueue = dynamic_cast<VulkanRenderCommandQueue*>(renderDevice->GetRenderCommandQueue());

	vulkanQueue->Execute(cmd.release());
	vulkanQueue->WaitForFenceToComplete(vulkanQueue->GetFenceValue());

	vmaDestroyBuffer(renderDevice->GetAllocator(), stagingBuffer, stagingAllocation);

	m_texture.currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void Prism::Render::Vulkan::VulkanTexture::GenerateMipMaps(VkCommandBuffer cmd)
{
	if (m_originalDesc.dimension == ResourceDimension::Tex3D)
	{
		return;
	}

	int32_t mipWidth = m_originalDesc.width;
	int32_t mipHeight = m_originalDesc.height;

	const uint32_t layerCount = static_cast<uint32_t>(m_originalDesc.GetArraySize());

	for (uint32_t i = 1; i < m_originalDesc.mipLevels; i++)
	{
		VkImageMemoryBarrier barrier{
		    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		    .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .image = m_texture.image,
		    .subresourceRange =
		        {
		            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		            .baseMipLevel = i - 1,
		            .levelCount = 1,
		            .baseArrayLayer = 0,
		            .layerCount = layerCount,
		        },
		};
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
		                     &barrier);

		const int32_t nextWidth = std::max(1, mipWidth / 2);
		const int32_t nextHeight = std::max(1, mipHeight / 2);

		VkImageBlit blit{
		    .srcSubresource =
		        {
		            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		            .mipLevel = (i - 1),
		            .baseArrayLayer = 0,
		            .layerCount = layerCount,
		        },
		    .srcOffsets = {{0, 0, 0}, {mipWidth, mipHeight, 1}},
		    .dstSubresource =
		        {
		            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		            .mipLevel = i,
		            .baseArrayLayer = 0,
		            .layerCount = layerCount,
		        },
		    .dstOffsets = {{0, 0, 0}, {nextWidth, nextHeight, 1}},
		};
		vkCmdBlitImage(cmd, m_texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_texture.image,
		               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
		                     nullptr, 1, &barrier);

		mipWidth = nextWidth;
		mipHeight = nextHeight;
	}

	const VkImageMemoryBarrier finalBarrier{
	    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
	    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
	    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	    .image = m_texture.image,
	    .subresourceRange =
	        {
	            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	            .baseMipLevel = static_cast<uint32_t>(m_originalDesc.mipLevels - 1),
	            .levelCount = 1,
	            .baseArrayLayer = 0,
	            .layerCount = layerCount,
	        },
	};

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1,
	                     &finalBarrier);

	m_texture.currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}
