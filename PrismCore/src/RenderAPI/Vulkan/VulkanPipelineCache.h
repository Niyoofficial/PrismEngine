#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/PipelineState.h"
#include "Prism/Render/VertexBufferCache.h"

namespace Prism::Render::Vulkan
{
struct GraphicsPipeline
{
	VkPipeline pipeline = VK_NULL_HANDLE;
};

struct ComputePipeline
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
};

struct VertexInputState
{
	VkVertexInputBindingDescription binding{};
	std::vector<VkVertexInputAttributeDescription> attributes;
	VkPipelineVertexInputStateCreateInfo createInfo{};
};

struct DynamicState
{
	std::array<VkDynamicState, 2> states;
	VkPipelineDynamicStateCreateInfo createInfo;
};

struct ColorBlendState
{
	std::vector<VkPipelineColorBlendAttachmentState> attachments;
	VkPipelineColorBlendStateCreateInfo createInfo{};
};

struct RenderingInfo
{
	std::vector<VkFormat> colorFormats;
	VkPipelineRenderingCreateInfo createInfo{};
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
	                                             const TextureView* dsv) const;
	[[nodiscard]] uint64_t HashPipelineStateDesc(const ComputePipelineStateDesc& desc) const;

	[[nodiscard]] VertexInputState BuildVertexInputState(const VertexAttributeList& vertexAttributeList);

	[[nodiscard]] VkPipelineInputAssemblyStateCreateInfo BuildInputAssemblyState(const GraphicsPipelineStateDesc& desc);

	[[nodiscard]] VkPipelineViewportStateCreateInfo BuildViewportState();

	[[nodiscard]] VkPipelineRasterizationStateCreateInfo BuildPipelineRasterizationState(const GraphicsPipelineStateDesc& desc);

	[[nodiscard]] VkPipelineMultisampleStateCreateInfo BuildMultisampleState(const GraphicsPipelineStateDesc& desc);

	[[nodiscard]] DynamicState BuildDynamicState();

	[[nodiscard]] VkPipelineDepthStencilStateCreateInfo BuildDepthStencilState(const GraphicsPipelineStateDesc& desc);

	[[nodiscard]] ColorBlendState BuildColorBlendState(const GraphicsPipelineStateDesc& desc, uint32_t colorAttachmentCount);

	[[nodiscard]] RenderingInfo BuildRenderingInfo(const std::vector<Ref<TextureView>>& rtvs, TextureView* dsv);

	VkPipelineCache m_pipelineCache{};

	std::unordered_map<uint64_t, GraphicsPipeline> m_graphicsPipelines;

	std::unordered_map<uint64_t, ComputePipeline> m_computePipelines;
};
} // namespace Prism::Render::Vulkan
