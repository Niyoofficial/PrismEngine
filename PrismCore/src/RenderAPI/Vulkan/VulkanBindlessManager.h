#pragma once

#include <mutex>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Prism::Render::Vulkan
{

class VulkanBindlessManager
{
public:
	static constexpr uint32_t SamplerCount = 7;

	static constexpr uint32_t SamplerBindingBegin = 0;
	static constexpr uint32_t ResourcesBinding = 7;
	static constexpr uint32_t ResourceHeapBinding = 8;
	static constexpr uint32_t LegacyBufferHeapBinding = 9;

	static constexpr uint32_t MaxBindlessDescriptors = 4096;

	void Initialize(VkDevice device);
	void Shutdown(VkDevice device);

	[[nodiscard]] VkDescriptorSetLayout GetLayout() const { return m_layout; }

	[[nodiscard]] VkDescriptorSet GetSet() const { return m_set; }

	[[nodiscard]] uint32_t AllocateResource();

	void FreeResource(uint32_t index);

	void WriteSampledImage(VkDevice device, uint32_t index, VkImageView view, VkImageLayout layout);

	void WriteStorageImage(VkDevice device, uint32_t index, VkImageView view, VkImageLayout layout);

	void WriteUniformBuffer(VkDevice device, uint32_t index, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

	void WriteStorageBuffer(VkDevice device, uint32_t index, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

	void WriteResourcesBuffer(VkDevice device, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

	void WriteSampler(VkDevice device, uint32_t samplerIndex, VkSampler sampler);

	void WriteLegacyBuffer(VkDevice device, uint32_t index, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

private:
	struct FreeList
	{
		std::vector<uint32_t> freeIndices;
		uint32_t nextIndex = 0;
		std::mutex mutex;
	};

	VkDescriptorSetLayout m_layout{};
	VkDescriptorPool m_pool{};
	VkDescriptorSet m_set{};

	FreeList m_resourceFreeList;
};

} // namespace Prism::Render::Vulkan
