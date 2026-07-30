#pragma once

#include <span>
#include <spirv_reflect.h>
#include <vulkan/vulkan_core.h>

namespace Prism::Render::Vulkan
{
struct VulkanShaderReflection;

struct DescriptorBindingKey
{
	uint32_t binding;
	VkDescriptorType type;
	uint32_t count;
	VkShaderStageFlags stageFlags;

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
	~VulkanDescriptorSetLayoutCache();

	[[nodiscard]] VkDescriptorSetLayout GetOrCreateDescriptorSetLayout(std::span<const VulkanShaderReflection> shaderReflections,
	                                                                   uint32_t set);

private:
	static uint32_t GetDescriptorCount(const SpvReflectDescriptorBinding& binding);

	// TODO
	// implement hash for DescriptorSetLayoutKey and use unordered_map instead of map
	std::map<DescriptorSetLayoutKey, VkDescriptorSetLayout> m_cache;
};
} // namespace Prism::Render::Vulkan
