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

	static constexpr VkDeviceSize ResourcesSize = 256;

	static constexpr uint32_t ResourcesRingCapacity = 16384;

	VulkanBindlessManager();
	~VulkanBindlessManager();

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

	struct ResourcesAllocation
	{
		VkDeviceSize offset = 0;
		uint32_t* data = nullptr;
	};

	ResourcesAllocation AllocateResources();

	[[nodiscard]] VkBuffer GetResourcesBuffer() const { return m_resourcesBuffer; }

	void ResetResourcesAllocator();

private:
	void CreateSamplers(VkDevice device, VkPhysicalDevice physicalDevice);

	void DestroySamplers(VkDevice device);

	void CreateResourcesBuffer(VkDevice device, VkPhysicalDevice physicalDevice);

	void DestroyResourcesBuffer(VkDevice device);

	VkDeviceSize AlignResourcesOffset(VkDeviceSize offset) const;

	struct FreeList
	{
		std::vector<uint32_t> freeIndices;
		uint32_t nextIndex = 0;
		std::mutex mutex;
	};

	VkDescriptorSetLayout m_layout{};
	VkDescriptorPool m_pool{};
	VkDescriptorSet m_set{};

	std::array<VkSampler, SamplerCount> m_samplers{};

	FreeList m_resourceFreeList;

	VkBuffer m_resourcesBuffer{};
	VkDeviceMemory m_resourcesMemory{};
	void* m_resourcesMapped = nullptr;

	VkDeviceSize m_resourcesStride = 0;
	VkDeviceSize m_resourcesBufferSize = 0;
	VkDeviceSize m_resourcesAlignment = 1;

	std::atomic<uint32_t> m_resourcesCursor = 0;
};

} // namespace Prism::Render::Vulkan
