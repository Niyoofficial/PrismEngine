#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/Shader.h"
#include "Prism/Utilities/Flags.h"

namespace Prism::Render
{
enum class VertexAttribute;
}

namespace Prism::Render::Vulkan
{
VkImageType GetVkImageType(ResourceDimension dimension);

VkImageViewType GetVkImageViewType(ResourceDimension dimension, uint32_t arrayLayers);

VkImageUsageFlags GetVkImageUsageFlags(Flags<BindFlags> flags);

VkImageAspectFlags GetVkImageAspectFlags(TextureFormat format);

VkFormat GetVkFormat(TextureFormat format);

VkBufferUsageFlags GetVkBufferUsageFlags(Flags<BindFlags> flags);

VmaAllocationCreateInfo GetVmaAllocationCreateInfo(ResourceUsage usage, Flags<CPUAccess> cpuAccess);

VkImageLayout GetVkImageLayout(BarrierLayout layout);

VkShaderStageFlags GetVkShaderStageFlags(ShaderType type);

VkFormat GetVkFormat(VertexAttribute attribute);

VkPrimitiveTopology GetVkPrimitiveTopology(TopologyType topologyType);

VkPolygonMode GetVkPolygonMode(FillMode fillMode);

VkCullModeFlags GetVkCullModeFlags(CullMode cullMode);

VkSampleCountFlagBits GetVkSampleCountFlagBits(uint32_t sampleCount);

VkCompareOp GetVkCompareOp(ComparisionFunction function);

VkStencilOp GetVkStencilOp(StencilOperation operation);

VkStencilOpState GetVkStencilOpState(const DepthStencilOperationDesc& desc, uint8_t readMask = 0xFF, uint8_t writeMask = 0xFF,
                                     uint32_t reference = 0);

VkBlendFactor GetVkBlendFactor(BlendFactor factor);

VkBlendOp GetVkBlendOp(BlendOperation operation);

VkColorComponentFlags GetVkColorComponentFlags(ColorMask mask);

VkLogicOp GetVkLogicOp(LogicOperation);

uint32_t GetBytesPerPixel(TextureFormat format);
} // namespace Prism::Render::Vulkan
