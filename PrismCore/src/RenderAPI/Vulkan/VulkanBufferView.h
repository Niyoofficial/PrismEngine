#pragma once

#include <vulkan/vulkan_core.h>
#include "Prism/Render/BufferView.h"

namespace Prism::Render::Vulkan
{
class VulkanBufferView : public BufferView
{
public:
	VulkanBufferView(const BufferViewDesc& desc, Buffer* buffer);

	[[nodiscard]] BufferViewDesc GetViewDesc() const override { return m_viewDesc; }

	[[nodiscard]] VkDescriptorBufferInfo GetDescriptorBufferInfo() const { return m_descriptorBufferInfo; }

	[[nodiscard]] VkDescriptorType GetDescriptorType() const { return m_descriptorType; }

private:
	BufferViewDesc m_viewDesc;

	VkDescriptorBufferInfo m_descriptorBufferInfo{};
	VkDescriptorType m_descriptorType{};
};
} // namespace Prism::Render::Vulkan
