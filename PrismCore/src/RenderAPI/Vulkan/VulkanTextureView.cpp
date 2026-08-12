#include "VulkanTextureView.h"

#include "Prism/Render/Texture.h"
#include "VulkanRenderDevice.h"
#include "VulkanTexture.h"
#include "VulkanTypeConversions.h"

Prism::Render::Vulkan::VulkanTextureView::VulkanTextureView(TextureViewDesc desc, Texture* texture) : m_viewDesc(desc)
{
	PE_ASSERT(texture, "Invalid texture!");

	m_owningTexture = texture;

	if (m_viewDesc.format == TextureFormat::Unknown)
	{
		m_viewDesc.format = texture->GetTextureDesc().format;
	}

	if (m_viewDesc.type == TextureViewType::Unknown)
	{
		constexpr Flags<BindFlags> textureViewPossibleFlags = Flags(BindFlags::ShaderResource) |
		    Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::RenderTarget) | Flags(BindFlags::DepthStencil);

		const Flags<BindFlags> flags = texture->GetTextureDesc().bindFlags & textureViewPossibleFlags;

		PE_ASSERT(glm::bitCount(flags.GetUnderlyingType()) == 1,
		          "Can't determine view type, to automatically determine view type the owning texture must have exactly one of "
		          "the view-compatible bind flags");

		if (flags.HasAllFlags(BindFlags::ShaderResource))
		{
			m_viewDesc.type = TextureViewType::SRV;
		}
		else if (flags.HasAllFlags(BindFlags::UnorderedAccess))
		{
			m_viewDesc.type = TextureViewType::UAV;
		}
		else if (flags.HasAllFlags(BindFlags::RenderTarget))
		{
			m_viewDesc.type = TextureViewType::RTV;
		}
		else if (flags.HasAllFlags(BindFlags::DepthStencil))
		{
			m_viewDesc.type = TextureViewType::DSV;
		}
	}

	if (m_viewDesc.dimension == ResourceDimension::Undefined)
	{
		m_viewDesc.dimension = texture->GetTextureDesc().dimension;
	}

	const auto* vkTexture = dynamic_cast<VulkanTexture*>(texture);

	const VkImage image = vkTexture->GetVulkanTextureResource().image;

	VkImageViewCreateInfo createInfo{
	    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	    .image = image,
	    .viewType = GetVkImageViewType(m_viewDesc.dimension, texture->GetTextureDesc().GetArraySize()),
	    .format = GetVkFormat(m_viewDesc.format),
	    .components{
	        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
	        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
	        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
	        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
	    },
	    .subresourceRange{
	        .aspectMask = GetVkImageAspectFlags(m_viewDesc.format),
	        .baseMipLevel = static_cast<uint32_t>(m_viewDesc.subresourceRange.firstMipLevel),
	        .levelCount = static_cast<uint32_t>(m_viewDesc.subresourceRange.numMipLevels),
	        .baseArrayLayer = static_cast<uint32_t>(m_viewDesc.subresourceRange.firstArraySlice),
	        .layerCount = static_cast<uint32_t>(m_viewDesc.subresourceRange.numArraySlices),
	    },
	};

	PE_ASSERT(vkCreateImageView(VulkanRenderDevice::Get().GetDevice(), &createInfo, nullptr, &m_vkImageView) == VK_SUCCESS);
}

Prism::Render::Vulkan::VulkanTextureView::~VulkanTextureView()
{
	if (m_bindlessIndex != UINT32_MAX)
	{
		VulkanRenderDevice::Get().GetBindlessManager().FreeSlot(BindlessBinding::Texture2D, m_bindlessIndex);
	}

	if (m_vkImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(VulkanRenderDevice::Get().GetDevice(), m_vkImageView, nullptr);
	}
}

uint32_t Prism::Render::Vulkan::VulkanTextureView::GetBindlessIndex()
{
	if (m_bindlessIndex != UINT32_MAX)
	{
		return m_bindlessIndex;
	}

	auto& device = VulkanRenderDevice::Get();
	auto& bindless = device.GetBindlessManager();

	m_bindlessIndex = bindless.AllocateSlot(BindlessBinding::Texture2D);
	bindless.WriteTexture(device.GetDevice(), m_bindlessIndex, m_vkImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	return m_bindlessIndex;
}
