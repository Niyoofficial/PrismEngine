#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/Buffer.h"

namespace Prism::Render::Vulkan
{
class VulkanRenderDevice;

struct VulkanBufferResources
{
	VkBuffer buffer{};
	VmaAllocation allocation{};
};

class VulkanBuffer : public Buffer
{
public:
	VulkanBuffer(VulkanRenderDevice* renderDevice, const BufferDesc& desc);
	~VulkanBuffer() override;

	void* Map(Flags<CPUAccess> access) override;
	void Unmap() override;

	BufferDesc GetBufferDesc() const override { return m_originalDesc; }

	VkBuffer GetVkBuffer() const { return m_buffer.buffer; }

protected:
	std::any GetDynamicAllocation() const override;

private:
	VulkanBufferResources m_buffer;

	BufferDesc m_originalDesc;
};
} // namespace Prism::Render::Vulkan
