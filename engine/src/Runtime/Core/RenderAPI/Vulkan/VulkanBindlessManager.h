#pragma once

#include <mutex>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "VulkanDescriptorHeap.h"

namespace Prism::Render::Vulkan
{
class VulkanBindlessManager
{
public:
	static constexpr uint32_t SamplerCount = 7;
	static constexpr uint32_t MaxBindlessDescriptors = 4096;

	VulkanBindlessManager();
	~VulkanBindlessManager();

	VulkanBindlessManager(const VulkanBindlessManager&) = delete;
	VulkanBindlessManager& operator=(const VulkanBindlessManager&) = delete;

	[[nodiscard]] uint32_t AllocateResource();

	void FreeResource(uint32_t index);

	void WriteSampledImage(VkDevice device, uint32_t index, const VkImageViewCreateInfo& viewInfo, VkImageLayout layout);

	void WriteStorageImage(VkDevice device, uint32_t index, const VkImageViewCreateInfo& viewInfo, VkImageLayout layout);

	void WriteUniformBuffer(VkDevice device, uint32_t index, VkDeviceAddress address, VkDeviceSize range);

	void WriteStorageBuffer(VkDevice device, uint32_t index, VkDeviceAddress address, VkDeviceSize range);

	void WriteSampler(VkDevice device, uint32_t index, const VkSamplerCreateInfo& samplerInfo);

	void BindHeaps(VkCommandBuffer commandBuffer) const;

	[[nodiscard]] VkDeviceAddress GetResourceDescriptorAddress(uint32_t index) const;

	[[nodiscard]] VkDeviceSize GetResourceDescriptorSize() const { return m_resourceDescriptorSize; }

	[[nodiscard]] VkDeviceSize GetSamplerDescriptorSize() const { return m_samplerDescriptorSize; }

private:
	void CreateSamplers(VkDevice device, float maxAnisotropy);

	struct FreeList
	{
		std::vector<uint32_t> freeIndices;
		uint32_t nextIndex = 0;
		std::mutex mutex;
	};

	VulkanDescriptorHeap m_resourceHeap;
	VulkanDescriptorHeap m_samplerHeap;

	VkDeviceSize m_resourceDescriptorSize = 0;
	VkDeviceSize m_samplerDescriptorSize = 0;

	VkDeviceSize m_resourceHeapAlignment = 0;
	VkDeviceSize m_samplerHeapAlignment = 0;

	FreeList m_resourceFreeList;
};
} // namespace Prism::Render::Vulkan
