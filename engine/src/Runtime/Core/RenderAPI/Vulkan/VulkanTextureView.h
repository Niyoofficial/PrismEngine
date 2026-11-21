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

	[[nodiscard]] VkDescriptorSet GetImGuiDescriptorSet()
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

	[[nodiscard]] uint32_t GetBindlessIndex() const;

	void RegisterBindless();

private:
	TextureViewDesc m_viewDesc;

	VkImageViewCreateInfo m_vkImageViewCreateInfo{};

	VkImageView m_vkImageView{};

	VkDescriptorSet m_imguiDescriptorSet = VK_NULL_HANDLE;

	uint32_t m_bindlessIndex = UINT32_MAX;
};
} // namespace Prism::Render::Vulkan
