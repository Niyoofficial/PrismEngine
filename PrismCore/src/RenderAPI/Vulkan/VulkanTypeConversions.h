#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/RenderTypes.h"
#include "Prism/Utilities/Flags.h"

namespace Prism::Render::Vulkan
{
VkImageType GetVkImageType(ResourceDimension dimension);

VkImageViewType GetVkImageViewType(ResourceDimension dimension, uint32_t arrayLayers);

VkImageUsageFlags GetVkImageUsageFlags(Flags<BindFlags> flags);

VkImageAspectFlags GetVkImageAspectFlags(TextureFormat format);

VkFormat GetVkFormat(TextureFormat format);

VkBufferUsageFlags GetVkBufferUsageFlags(Flags<BindFlags> flags);

VmaAllocationCreateInfo GetVmaAllocationCreateInfo(ResourceUsage usage, Flags<CPUAccess> cpuAccess);
} // namespace Prism::Render::Vulkan
