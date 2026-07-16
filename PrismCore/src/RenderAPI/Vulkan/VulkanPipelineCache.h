#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/PipelineState.h"

namespace Prism::Render::Vulkan
{
struct GraphicsPipeline
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
};

struct ComputePipeline
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
};

class VulkanPipelineCache
{
public:
	VulkanPipelineCache();
	~VulkanPipelineCache();

	[[nodiscard]] VkPipeline GetOrCreatePipeline(const GraphicsPipelineStateDesc& desc, const std::vector<Ref<TextureView>>& rtvs,
	                                             TextureView* dsv);
	[[nodiscard]] VkPipeline GetOrCreatePipeline(const ComputePipelineStateDesc& desc);

private:
	[[nodiscard]] uint64_t HashPipelineStateDesc(const GraphicsPipelineStateDesc& desc, const std::vector<Ref<TextureView>>& rtvs,
	                                             TextureView* dsv) const;
	[[nodiscard]] uint64_t HashPipelineStateDesc(const ComputePipelineStateDesc& desc) const;

	VkPipelineCache m_pipelineCache{};

	std::unordered_map<uint64_t, GraphicsPipeline> m_graphicsPipelines;

	std::unordered_map<uint64_t, ComputePipeline> m_computePipelines;
};
} // namespace Prism::Render::Vulkan
