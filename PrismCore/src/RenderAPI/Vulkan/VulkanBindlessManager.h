#pragma once

#include <mutex>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Prism::Render::Vulkan
{
enum class BindlessBinding : uint32_t
{
	Texture2D = 0,
	TextureCube = 1,
	RawBuffer = 2,
	Count
};

constexpr uint32_t BindlessSet = 0;
constexpr uint32_t MaxBindlessDescriptorsPerBinding = 4096;

class VulkanBindlessManager
{
public:
	void Initialize(VkDevice device);
	void Shutdown(VkDevice device);

	[[nodiscard]] VkDescriptorSetLayout GetLayout() const { return m_layout; }

	[[nodiscard]] VkDescriptorSet GetSet() const { return m_set; }

	[[nodiscard]] uint32_t AllocateSlot(BindlessBinding binding);
	void FreeSlot(BindlessBinding binding, uint32_t slot);

	void WriteTexture(VkDevice device, uint32_t slot, VkImageView view, VkImageLayout layout);
	void WriteRawBuffer(VkDevice device, uint32_t slot, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

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

	FreeList m_freeLists[static_cast<size_t>(BindlessBinding::Count)];
};
} // namespace Prism::Render::Vulkan
