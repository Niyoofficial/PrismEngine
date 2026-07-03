#pragma once

#include <vulkan/vulkan_core.h>
#include "Prism/Render/TextureView.h"

namespace Prism::Render::Vulkan
{
class VulkanTextureView : public TextureView
{
public:
	VulkanTextureView(TextureViewDesc desc, Texture* texture);
	~VulkanTextureView() override;

	[[nodiscard]] TextureViewDesc GetViewDesc() const override { return m_viewDesc; }

	VkImageView GetVkImageView() const { return m_vkImageView; }

private:
	TextureViewDesc m_viewDesc;

	VkImageView m_vkImageView{};
};
} // namespace Prism::Render::Vulkan
