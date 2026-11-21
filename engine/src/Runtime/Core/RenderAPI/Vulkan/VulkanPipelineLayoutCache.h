#pragma once

#include <vulkan/vulkan_core.h>

namespace Prism::Render::Vulkan
{
class VulkanDescriptorSetLayoutCache;
struct VulkanShaderReflection;

struct PushConstantKey
{
	uint32_t offset;
	uint32_t size;
	VkShaderStageFlags stageFlags;

	auto operator<=>(const PushConstantKey&) const = default;
};

struct PipelineLayoutKey
{
	std::vector<PushConstantKey> pushConstants;

	auto operator<=>(const PipelineLayoutKey&) const = default;
};

class VulkanPipelineLayoutCache
{
public:
	VulkanPipelineLayoutCache() = default;
	~VulkanPipelineLayoutCache();

	VulkanPipelineLayoutCache(const VulkanPipelineLayoutCache&) = delete;
	VulkanPipelineLayoutCache& operator=(const VulkanPipelineLayoutCache&) = delete;

	VulkanPipelineLayoutCache(VulkanPipelineLayoutCache&&) = delete;
	VulkanPipelineLayoutCache& operator=(VulkanPipelineLayoutCache&&) = delete;

	[[nodiscard]] VkPipelineLayout GetOrCreatePipelineLayout(std::span<const VulkanShaderReflection* const> shaderReflections);

private:
	// TODO
	// implement hash for PipelineLayoutKey and use unordered_map instead of map
	std::map<PipelineLayoutKey, VkPipelineLayout> m_cache;
};
} // namespace Prism::Render::Vulkan
