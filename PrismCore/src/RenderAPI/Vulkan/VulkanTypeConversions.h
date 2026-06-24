#pragma once

#include <vulkan/vulkan_core.h>
#include "Prism/Render/RenderTypes.h"

namespace Prism::Render::Vulkan
{
static VkImageType GetVkImageType(ResourceDimension dimension);

static VkImageViewType GetVkImageViewType(ResourceDimension dimension, uint32_t arrayLayers);

static VkImageUsageFlags GetVkImageUsageFlags(Flags<BindFlags> flags);

static VkImageAspectFlags GetVkImageAspectFlags(TextureFormat format);

static VkFormat GetVkFormat(TextureFormat format);
} // namespace Prism::Render::Vulkan
