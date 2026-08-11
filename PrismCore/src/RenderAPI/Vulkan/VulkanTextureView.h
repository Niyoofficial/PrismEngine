#pragma once

#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Prism/Render/TextureView.h"
#include "VulkanTexture.h"

namespace Prism::Render::Vulkan
{
class VulkanTextureView : public TextureView
{
public:
	VulkanTextureView(TextureViewDesc desc, Texture* texture);
	~VulkanTextureView() override;

	[[nodiscard]] TextureViewDesc GetViewDesc() const override { return m_viewDesc; }

	VkImageView GetVkImageView() const { return m_vkImageView; }

	// TODO
	// bad solution, just for first engine build. should be handled other way
	VkDescriptorSet GetImGuiDescriptorSet()
	{
		if (m_imguiDescriptorSet)
		{
			return m_imguiDescriptorSet;
		}

		const auto* texture = dynamic_cast<VulkanTexture*>(m_owningTexture.Raw());

		const auto resource = texture->GetVulkanTextureResource();

		m_imguiDescriptorSet =
		    ImGui_ImplVulkan_AddTexture(resource.sampler, m_vkImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		return m_imguiDescriptorSet;
	}

private:
	TextureViewDesc m_viewDesc;

	VkImageView m_vkImageView{};

	// TODO
	// bad solution, just for first engine build. should be handled other way
	VkDescriptorSet m_imguiDescriptorSet = VK_NULL_HANDLE;
};
} // namespace Prism::Render::Vulkan
