#include "VulkanTexture.h"

#include "VulkanBuffer.h"
#include "VulkanRenderCommandList.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderDevice.h"
#include "VulkanTypeConversions.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Prism::Render::Vulkan::VulkanTexture::VulkanTexture(VulkanRenderDevice* renderDevice, const TextureDesc& desc,
                                                    const BarrierLayout initLayout) : Texture(renderDevice), m_originalDesc(desc)
{
	CreateImage(renderDevice, desc);
	CreateSampler(renderDevice, desc);

	if (initLayout != BarrierLayout::Undefined)
	{
		const auto cmd = RenderCommandList::Create();

		auto* vulkanCmd = dynamic_cast<VulkanRenderCommandList*>(cmd.Raw());
		PE_ASSERT(vulkanCmd);

		vulkanCmd->Barrier(TextureBarrier{
		    .texture = this,
		    .syncBefore = BarrierSync::None,
		    .syncAfter = BarrierSync::All,
		    .accessBefore = BarrierAccess::NoAccess,
		    .accessAfter = BarrierAccess::Common,
		    .layoutBefore = BarrierLayout::Undefined,
		    .layoutAfter = initLayout,
		    .subresourceRange =
		        {
		            .firstMipLevel = 0,
		            .numMipLevels = m_originalDesc.mipLevels,
		            .firstArraySlice = 0,
		            .numArraySlices = m_originalDesc.Is3D() ? 1 : m_originalDesc.GetArraySize(),
		        },
		});

		auto* queue = dynamic_cast<VulkanRenderCommandQueue*>(renderDevice->GetRenderCommandQueue());
		PE_ASSERT(queue);

		queue->SubmitImmediate(cmd);
	}
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

		// TODO
		// add MipMap support
		// const auto mipLevels = static_cast<int32_t>(std::floor(std::log2(std::max(width, height))) + 1);
		constexpr int32_t mipLevels = 1;

		m_originalDesc = TextureDesc::CreateTex2D(filepath, width, height, TextureFormat::RGBA8_UNorm, BindFlags::ShaderResource,
		                                          ResourceUsage::Default, mipLevels);

		const VkDeviceSize imageSize = width * height * STBI_rgb_alpha * bytesPerChannel;

		CreateImage(renderDevice, m_originalDesc);
		CreateSampler(renderDevice, m_originalDesc);

		const auto cmd = RenderCommandList::Create();
		auto* vulkanCmd = dynamic_cast<VulkanRenderCommandList*>(cmd.Raw());

		vulkanCmd->Barrier(TextureBarrier{
		    .texture = this,
		    .syncBefore = BarrierSync::None,
		    .syncAfter = BarrierSync::Copy,
		    .accessBefore = BarrierAccess::NoAccess,
		    .accessAfter = BarrierAccess::CopyDest,
		    .layoutBefore = BarrierLayout::Undefined,
		    .layoutAfter = BarrierLayout::CopyDest,
		    .subresourceRange =
		        {
		            .firstMipLevel = 0,
		            .numMipLevels = 1,
		            .firstArraySlice = 0,
		            .numArraySlices = m_originalDesc.Is3D() ? 1 : m_originalDesc.GetArraySize(),
		        },
		});

		vulkanCmd->UpdateTexture(Ref<Texture>(this),
		                         RawData{
		                             .data = pixels,
		                             .sizeInBytes = static_cast<int64_t>(imageSize),
		                         },
		                         0);

		vulkanCmd->Barrier(TextureBarrier{
		    .texture = this,
		    .syncBefore = BarrierSync::Copy,
		    .syncAfter = Flags<BarrierSync>(BarrierSync::PixelShading, BarrierSync::ComputeShading),
		    .accessBefore = BarrierAccess::CopyDest,
		    .accessAfter = BarrierAccess::ShaderResource,
		    .layoutBefore = BarrierLayout::CopyDest,
		    .layoutAfter = BarrierLayout::ShaderResource,
		    .subresourceRange =
		        {
		            .firstMipLevel = 0,
		            .numMipLevels = 1,
		            .firstArraySlice = 0,
		            .numArraySlices = m_originalDesc.Is3D() ? 1 : m_originalDesc.GetArraySize(),
		        },
		});

		auto* queue = dynamic_cast<VulkanRenderCommandQueue*>(renderDevice->GetRenderCommandQueue());
		PE_ASSERT(queue);
		queue->SubmitImmediate(cmd);

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

		// TODO
		// add MipMap support
		// const auto mipLevels = static_cast<int32_t>(std::floor(std::log2(std::max(width, height))) + 1);
		constexpr int32_t mipLevels = 1;

		m_originalDesc = TextureDesc::CreateTex2D(name, width, height, TextureFormat::RGBA8_UNorm, BindFlags::ShaderResource,
		                                          ResourceUsage::Default, mipLevels);

		const VkDeviceSize imageSize = width * height * STBI_rgb_alpha;

		CreateImage(renderDevice, m_originalDesc);
		CreateSampler(renderDevice, m_originalDesc);

		const auto cmd = RenderCommandList::Create();
		auto* vulkanCmd = dynamic_cast<VulkanRenderCommandList*>(cmd.Raw());
		PE_ASSERT(vulkanCmd);
		vulkanCmd->Barrier(TextureBarrier{
		    .texture = this,
		    .syncBefore = BarrierSync::None,
		    .syncAfter = BarrierSync::Copy,
		    .accessBefore = BarrierAccess::NoAccess,
		    .accessAfter = BarrierAccess::CopyDest,
		    .layoutBefore = BarrierLayout::Undefined,
		    .layoutAfter = BarrierLayout::CopyDest,
		    .subresourceRange =
		        {
		            .firstMipLevel = 0,
		            .numMipLevels = 1,
		            .firstArraySlice = 0,
		            .numArraySlices = m_originalDesc.Is3D() ? 1 : m_originalDesc.GetArraySize(),
		        },
		});

		vulkanCmd->UpdateTexture(Ref<Texture>(this),
		                         RawData{
		                             .data = pixels,
		                             .sizeInBytes = static_cast<int64_t>(imageSize),
		                         },
		                         0);

		vulkanCmd->Barrier(TextureBarrier{
		    .texture = this,
		    .syncBefore = BarrierSync::Copy,
		    .syncAfter = Flags<BarrierSync>(BarrierSync::PixelShading, BarrierSync::ComputeShading),
		    .accessBefore = BarrierAccess::CopyDest,
		    .accessAfter = BarrierAccess::ShaderResource,
		    .layoutBefore = BarrierLayout::CopyDest,
		    .layoutAfter = BarrierLayout::ShaderResource,
		    .subresourceRange =
		        {
		            .firstMipLevel = 0,
		            .numMipLevels = 1,
		            .firstArraySlice = 0,
		            .numArraySlices = m_originalDesc.Is3D() ? 1 : m_originalDesc.GetArraySize(),
		        },
		});

		auto* queue = dynamic_cast<VulkanRenderCommandQueue*>(renderDevice->GetRenderCommandQueue());
		PE_ASSERT(queue);
		queue->SubmitImmediate(cmd);

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

Prism::Render::Vulkan::VulkanTexture::VulkanTexture(VulkanRenderDevice* renderDevice, VkImage image, const TextureDesc& desc) :
    Texture(renderDevice), m_originalDesc(desc)
{
	m_texture.image = image;
	m_texture.format = GetVkFormat(desc.format);
	m_texture.width = desc.width;
	m_texture.height = desc.height;
	m_texture.depth = desc.Is3D() ? desc.GetDepth() : 1;
	m_texture.mipLevels = desc.mipLevels;
	m_texture.arrayLayers = desc.GetArraySize();
	m_texture.aspectMask = GetVkImageAspectFlags(desc.format);
	m_texture.allocation = nullptr;
}

Prism::Render::Vulkan::VulkanTexture::~VulkanTexture()
{
	VulkanTexture::WaitForLoadFinish();

	const auto* device = dynamic_cast<VulkanRenderDevice*>(m_renderDevice);

	if (m_texture.image && m_texture.allocation)
	{
		vmaDestroyImage(device->GetAllocator(), m_texture.image, m_texture.allocation);

		m_texture.image = VK_NULL_HANDLE;
		m_texture.allocation = nullptr;
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

	m_texture.format = GetVkFormat(desc.format);
	m_texture.width = desc.width;
	m_texture.height = desc.height;
	m_texture.depth = desc.Is3D() ? desc.GetDepth() : 1;
	m_texture.mipLevels = desc.mipLevels;
	m_texture.arrayLayers = desc.Is3D() ? 1 : desc.GetArraySize();
	m_texture.aspectMask = GetVkImageAspectFlags(desc.format);

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
