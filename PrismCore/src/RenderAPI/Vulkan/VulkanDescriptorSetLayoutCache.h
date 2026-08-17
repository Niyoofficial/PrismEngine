#pragma once

#include <span>
#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include <spirv_reflect.h>
#include <vulkan/vulkan_core.h>

namespace Prism::Render::Vulkan
{
struct VulkanShaderReflection;

struct DescriptorBindingKey
{
	uint32_t binding = 0;
	VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
	uint32_t count = 0;
	VkShaderStageFlags stageFlags = 0;

	auto operator<=>(const DescriptorBindingKey&) const = default;
};

struct DescriptorSetLayoutKey
{
	std::vector<DescriptorBindingKey> bindings;

	auto operator<=>(const DescriptorSetLayoutKey&) const = default;
};

class VulkanDescriptorSetLayoutCache
{
public:
	VulkanDescriptorSetLayoutCache() = default;
	~VulkanDescriptorSetLayoutCache();

	VulkanDescriptorSetLayoutCache(const VulkanDescriptorSetLayoutCache&) = delete;
	VulkanDescriptorSetLayoutCache& operator=(const VulkanDescriptorSetLayoutCache&) = delete;

	VulkanDescriptorSetLayoutCache(VulkanDescriptorSetLayoutCache&&) = delete;
	VulkanDescriptorSetLayoutCache& operator=(VulkanDescriptorSetLayoutCache&&) = delete;

	[[nodiscard]] VkDescriptorSetLayout
	GetOrCreateDescriptorSetLayout(std::span<const VulkanShaderReflection* const> shaderReflections, uint32_t set);

	[[nodiscard]] VkDescriptorSetLayout GetOrCreateDescriptorSetLayout(const DescriptorSetLayoutKey& layoutKey);

	[[nodiscard]] static DescriptorSetLayoutKey BuildLayoutKey(std::span<const VulkanShaderReflection* const> shaderReflections,
	                                                           uint32_t set);

private:
	static uint32_t GetDescriptorCount(const SpvReflectDescriptorBinding& binding);

	// TODO
	// implement hash for DescriptorSetLayoutKey and use unordered_map instead of map
	std::map<DescriptorSetLayoutKey, VkDescriptorSetLayout> m_cache;
};
} // namespace Prism::Render::Vulkan
