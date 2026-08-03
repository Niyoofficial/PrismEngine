#pragma once

#include <vulkan/vulkan_core.h>
#include "VulkanDescriptorSetLayoutCache.h"

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
	std::vector<DescriptorSetLayoutKey> descriptorSets;
	std::vector<PushConstantKey> pushConstants;

	auto operator<=>(const PipelineLayoutKey&) const = default;
};

class VulkanPipelineLayoutCache
{
public:
	explicit VulkanPipelineLayoutCache(VulkanDescriptorSetLayoutCache& descriptorSetLayoutCache) :
	    m_descriptorSetLayoutCache(descriptorSetLayoutCache)
	{}

	~VulkanPipelineLayoutCache();

	[[nodiscard]] VkPipelineLayout GetOrCreatePipelineLayout(std::span<const VulkanShaderReflection* const> shaderReflections);

private:
	VulkanDescriptorSetLayoutCache& m_descriptorSetLayoutCache;

	// TODO
	// implement hash for PipelineLayoutKey and use unordered_map instead of map
	std::map<PipelineLayoutKey, VkPipelineLayout> m_cache;
};
} // namespace Prism::Render::Vulkan
