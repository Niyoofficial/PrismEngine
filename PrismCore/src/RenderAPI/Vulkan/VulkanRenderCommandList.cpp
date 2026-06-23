#include "VulkanRenderCommandList.h"

#include <vulkan/vulkan_core.h>

#include "VulkanBuffer.h"
#include "VulkanRenderDevice.h"
#include "VulkanTexture.h"
#include "VulkanTextureView.h"

// TODO
// move to VulkanTypeConversions
static VkImageLayout BarrierLayoutToVkImageLayout(const Prism::Render::BarrierLayout layout)
{
	switch (layout)
	{
	case Prism::Render::BarrierLayout::Undefined:
		return VK_IMAGE_LAYOUT_UNDEFINED;
	case Prism::Render::BarrierLayout::Common:
		return VK_IMAGE_LAYOUT_GENERAL;
	case Prism::Render::BarrierLayout::Present:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	case Prism::Render::BarrierLayout::GenericRead:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case Prism::Render::BarrierLayout::RenderTarget:
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case Prism::Render::BarrierLayout::UnorderedAccess:
		return VK_IMAGE_LAYOUT_GENERAL;
	case Prism::Render::BarrierLayout::DepthStencilWrite:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case Prism::Render::BarrierLayout::DepthStencilRead:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case Prism::Render::BarrierLayout::ShaderResource:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case Prism::Render::BarrierLayout::CopySource:
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case Prism::Render::BarrierLayout::CopyDest:
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	default:
		return VK_IMAGE_LAYOUT_GENERAL;
	}
}

Prism::Render::Vulkan::VulkanRenderCommandList::VulkanRenderCommandList()
{
	const auto& device = VulkanRenderDevice::Get();

	uint32_t queueFamily = 0;

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device.GetPhysicalDevice(), &count, nullptr);

	std::vector<VkQueueFamilyProperties> props(count);

	vkGetPhysicalDeviceQueueFamilyProperties(device.GetPhysicalDevice(), &count, props.data());

	for (uint32_t i = 0; i < count; ++i)
	{
		if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamily = i;
			break;
		}
	}

	const VkCommandPoolCreateInfo poolInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	    .queueFamilyIndex = queueFamily,
	};

	PE_ASSERT(vkCreateCommandPool(device.GetDevice(), &poolInfo, nullptr, &m_commandPool) == VK_SUCCESS);

	const VkCommandBufferAllocateInfo allocInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    .commandPool = m_commandPool,
	    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    .commandBufferCount = 1,
	};

	PE_ASSERT(vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, &m_commandBuffer) == VK_SUCCESS);

	constexpr VkCommandBufferBeginInfo beginInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	PE_ASSERT(vkBeginCommandBuffer(m_commandBuffer, &beginInfo) == VK_SUCCESS);
}

Prism::Render::Vulkan::VulkanRenderCommandList::~VulkanRenderCommandList()
{
	const auto& device = VulkanRenderDevice::Get();

	if (m_commandBuffer)
	{
		vkFreeCommandBuffers(device.GetDevice(), m_commandPool, 1, &m_commandBuffer);
	}

	if (m_commandPool)
	{
		vkDestroyCommandPool(device.GetDevice(), m_commandPool, nullptr);
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandList::Draw(const DrawCommandDesc desc)
{
	SetupDrawOrDispatch(PipelineStateType::Graphics);

	vkCmdDraw(m_commandBuffer, static_cast<uint32_t>(desc.numVertices), static_cast<uint32_t>(desc.numInstances),
	          static_cast<uint32_t>(desc.startVertexLocation), 0);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::DrawIndexed(const DrawIndexedCommandDesc desc)
{
	SetupDrawOrDispatch(PipelineStateType::Graphics);

	vkCmdDrawIndexed(m_commandBuffer, static_cast<uint32_t>(desc.numIndices), static_cast<uint32_t>(desc.numInstances),
	                 static_cast<uint32_t>(desc.startIndexLocation), static_cast<int32_t>(desc.baseVertexLocation), 0);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::Dispatch(const glm::int3 threadGroupCount)
{
	SetupDrawOrDispatch(PipelineStateType::Compute);

	vkCmdDispatch(m_commandBuffer, threadGroupCount.x, threadGroupCount.y, threadGroupCount.z);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetPSO(const GraphicsPipelineStateDesc& desc)
{
	m_currentGraphicsPSO = desc;
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetPSO(const ComputePipelineStateDesc& desc) { m_currentComputePSO = desc; }

void Prism::Render::Vulkan::VulkanRenderCommandList::SetStencilRef(const uint32_t ref)
{
	vkCmdSetStencilReference(m_commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, ref);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetRenderTargets(std::vector<Ref<TextureView>> rtvs,
                                                                      const Ref<TextureView>& dsv)
{
	m_renderTargetViews = std::move(rtvs);
	m_depthStencilView = dsv;
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetViewports(std::vector<Viewport> viewports)
{
	std::vector<VkViewport> vkViewports;

	for (const auto& [topLeft, size, depthRange] : viewports)
	{
		VkViewport vkViewport{
		    .x = topLeft.x,
		    .y = topLeft.y,
		    .width = size.x,
		    .height = size.y,
		    .minDepth = depthRange.x,
		    .maxDepth = depthRange.y,
		};
		vkViewports.push_back(vkViewport);
	}

	vkCmdSetViewport(m_commandBuffer, 0, static_cast<uint32_t>(vkViewports.size()), vkViewports.data());
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetScissors(std::vector<Scissor> scissors)
{
	std::vector<VkRect2D> vkScissors;

	for (const auto& [topLeft, size] : scissors)
	{
		VkRect2D rect{
		    .offset = {topLeft.x, topLeft.y},
		    .extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)},
		};
		vkScissors.push_back(rect);
	}

	vkCmdSetScissor(m_commandBuffer, 0, static_cast<uint32_t>(vkScissors.size()), vkScissors.data());
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetVertexBuffer(const Ref<Buffer>& buffer, int64_t vertexSizeInBytes)
{
	const auto* vulkanBuffer = dynamic_cast<VulkanBuffer*>(buffer.Raw());

	VkBuffer vkBuffer = vulkanBuffer->GetVkBuffer();

	constexpr VkDeviceSize offset = 0;

	vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &vkBuffer, &offset);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetIndexBuffer(const Ref<Buffer>& buffer, IndexBufferFormat format)
{
	const auto* vulkanBuffer = dynamic_cast<VulkanBuffer*>(buffer.Raw());

	const VkIndexType indexType = format == IndexBufferFormat::Uint16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

	vkCmdBindIndexBuffer(m_commandBuffer, vulkanBuffer->GetVkBuffer(), 0, indexType);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetTextures(const std::vector<Ref<TextureView>>& textureViews,
                                                                 const std::wstring& paramName)
{
	std::vector<Ref<RenderResourceView>> resources;

	resources.reserve(textureViews.size());

	for (auto& tex : textureViews)
	{
		resources.push_back(tex);
	}

	m_boundResources[paramName] = std::move(resources);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetBuffers(const std::vector<Ref<BufferView>>& bufferViews,
                                                                const std::vector<std::any> dynamicAllocations,
                                                                const std::wstring& paramName)
{
	std::vector<Ref<RenderResourceView>> resources;

	resources.reserve(bufferViews.size());

	for (auto& buffer : bufferViews)
	{
		resources.push_back(buffer);
	}

	m_boundResources[paramName] = std::move(resources);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::ClearRenderTargetView(const Ref<TextureView>& rtv, glm::float4* clearColor)
{
	auto* view = dynamic_cast<VulkanTextureView*>(rtv.Raw());

	const VkClearColorValue vkClearColor{
	    .float32 = {clearColor->x, clearColor->y, clearColor->z, clearColor->w},
	};

	constexpr VkImageSubresourceRange range{
	    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	    .baseMipLevel = 0,
	    .levelCount = VK_REMAINING_MIP_LEVELS,
	    .baseArrayLayer = 0,
	    .layerCount = VK_REMAINING_ARRAY_LAYERS,
	};

	vkCmdClearColorImage(m_commandBuffer, view->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, &vkClearColor, 1, &range);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::ClearDepthStencilView(const Ref<TextureView>& dsv, Flags<ClearFlags> flags,
                                                                           DepthStencilValue* clearValue)
{
	const auto* view = dynamic_cast<VulkanTextureView*>(dsv.Raw());

	VkImageAspectFlags aspectMask = 0;
	if (flags & ClearFlags::ClearDepth)
	{
		aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	if (flags & ClearFlags::ClearStencil)
	{
		aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	const VkClearDepthStencilValue clearDepthStencil{
	    .depth = clearValue->depth,
	    .stencil = clearValue->stencil,
	};

	const VkImageSubresourceRange range{
	    .aspectMask = aspectMask,
	    .baseMipLevel = 0,
	    .levelCount = VK_REMAINING_MIP_LEVELS,
	    .baseArrayLayer = 0,
	    .layerCount = VK_REMAINING_ARRAY_LAYERS,
	};

	vkCmdClearDepthStencilImage(m_commandBuffer, view->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, &clearDepthStencil, 1, &range);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::float4 values)
{
	const auto* view = dynamic_cast<VulkanTextureView*>(uav.Raw());

	const VkClearColorValue vkClearColor{
	    .float32 = {values.r, values.g, values.b, values.a},
	};

	vkCmdClearColorImage(m_commandBuffer, view->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, &vkClearColor, 1, nullptr);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::uint4 values)
{
	const auto* view = dynamic_cast<VulkanTextureView*>(uav.Raw());

	const VkClearColorValue clearValue{
	    .uint32 = {values.r, values.g, values.b, values.a},
	};

	vkCmdClearColorImage(m_commandBuffer, view->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, nullptr);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::Barrier(const BufferBarrier barrier)
{
	const auto* buffer = dynamic_cast<VulkanBuffer*>(barrier.buffer);

	const VkBufferMemoryBarrier bufferBarrier{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
	    .srcAccessMask = static_cast<VkAccessFlags>(barrier.accessBefore.GetUnderlyingType()),
	    .dstAccessMask = static_cast<VkAccessFlags>(barrier.accessAfter.GetUnderlyingType()),
	    .buffer = buffer->GetVkBuffer(),
	    .offset = static_cast<VkDeviceSize>(barrier.offset),
	    .size = barrier.size == -1 ? VK_WHOLE_SIZE : static_cast<VkDeviceSize>(barrier.size),
	};

	const auto srcStage = static_cast<VkPipelineStageFlags>(barrier.syncBefore.GetUnderlyingType());
	const auto dstStage = static_cast<VkPipelineStageFlags>(barrier.syncAfter.GetUnderlyingType());

	vkCmdPipelineBarrier(m_commandBuffer, srcStage, dstStage, 0, 0, nullptr, 1, &bufferBarrier, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::Barrier(const TextureBarrier barrier)
{
	const auto* texture = dynamic_cast<VulkanTexture*>(barrier.texture);

	const VkImageMemoryBarrier imageBarrier{
	    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	    .srcAccessMask = static_cast<VkAccessFlags>(barrier.accessBefore.GetUnderlyingType()),
	    .dstAccessMask = static_cast<VkAccessFlags>(barrier.accessAfter.GetUnderlyingType()),
	    .oldLayout = BarrierLayoutToVkImageLayout(barrier.layoutBefore),
	    .newLayout = BarrierLayoutToVkImageLayout(barrier.layoutAfter),
	    .image = texture->GetVulkanTextureResource().image,
	    .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                         .baseMipLevel = 0,
	                         .levelCount = VK_REMAINING_MIP_LEVELS,
	                         .baseArrayLayer = 0,
	                         .layerCount = VK_REMAINING_ARRAY_LAYERS},
	};


	const auto srcStage = static_cast<VkPipelineStageFlags>(barrier.syncBefore.GetUnderlyingType());
	const auto dstStage = static_cast<VkPipelineStageFlags>(barrier.syncAfter.GetUnderlyingType());

	vkCmdPipelineBarrier(m_commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::UpdateBuffer(const Ref<Buffer>& buffer, const RawData data)
{
	const auto* vulkanBuffer = dynamic_cast<VulkanBuffer*>(buffer.Raw());

	vkCmdUpdateBuffer(m_commandBuffer, vulkanBuffer->GetVkBuffer(), 0, data.sizeInBytes, data.data);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::UpdateTexture(const Ref<Texture>& texture, RawData data,
                                                                   const int32_t subresourceIndex)
{
	const auto* vulkanTexture = dynamic_cast<VulkanTexture*>(texture.Raw());

	const VkBufferImageCopy region{
	    .bufferOffset = 0,
	    .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                         .mipLevel = static_cast<uint32_t>(subresourceIndex),
	                         .baseArrayLayer = 0,
	                         .layerCount = 1},
	    .imageOffset = {0, 0, 0},
	    .imageExtent = {static_cast<uint32_t>(vulkanTexture->GetTextureDesc().GetWidth()),
	                    static_cast<uint32_t>(vulkanTexture->GetTextureDesc().GetHeight()), 1},
	};

	vkCmdCopyBufferToImage(m_commandBuffer, nullptr, vulkanTexture->GetVulkanTextureResource().image,
	                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::CopyBufferRegion(const Ref<Buffer>& dest, const int64_t destOffset,
                                                                      const Ref<Buffer>& src, const int64_t srcOffset,
                                                                      const int64_t numBytes)
{
	const auto* destBuffer = dynamic_cast<VulkanBuffer*>(dest.Raw());
	const auto* srcBuffer = dynamic_cast<VulkanBuffer*>(src.Raw());

	const VkBufferCopy region{
	    .srcOffset = static_cast<VkDeviceSize>(srcOffset),
	    .dstOffset = static_cast<VkDeviceSize>(destOffset),
	    .size = static_cast<VkDeviceSize>(numBytes),
	};

	vkCmdCopyBuffer(m_commandBuffer, srcBuffer->GetVkBuffer(), destBuffer->GetVkBuffer(), 1, &region);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::CopyBufferRegion(const Ref<Texture>& dest, const glm::int3 destLoc,
                                                                      const int32_t destSubresourceIndex, const Ref<Buffer>& src,
                                                                      const int64_t srcOffset)
{
	const auto* destTexture = dynamic_cast<VulkanTexture*>(dest.Raw());
	const auto* srcBuffer = dynamic_cast<VulkanBuffer*>(src.Raw());

	const VkBufferImageCopy region{
	    .bufferOffset = static_cast<VkDeviceSize>(srcOffset),
	    .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                         .mipLevel = static_cast<uint32_t>(destSubresourceIndex),
	                         .baseArrayLayer = 0,
	                         .layerCount = 1},
	    .imageOffset = {destLoc.x, destLoc.y, destLoc.z},
	    .imageExtent = {static_cast<uint32_t>(destTexture->GetTextureDesc().GetWidth()),
	                    static_cast<uint32_t>(destTexture->GetTextureDesc().GetHeight()), 1},
	};

	vkCmdCopyBufferToImage(m_commandBuffer, srcBuffer->GetVkBuffer(), destTexture->GetVulkanTextureResource().image,
	                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::CopyTextureRegion(const Ref<Buffer>& dest, const int64_t destOffset,
                                                                       const Ref<Texture>& src, const int32_t srcSubresourceIndex,
                                                                       Box3I srcBox)
{
	const auto* destBuffer = dynamic_cast<VulkanBuffer*>(dest.Raw());
	const auto* srcTexture = dynamic_cast<VulkanTexture*>(src.Raw());

	const glm::int3 boxMin = srcBox.location;
	const glm::int3 boxMax = srcBox.location + srcBox.size;

	const VkBufferImageCopy region{
	    .bufferOffset = static_cast<VkDeviceSize>(destOffset),
	    .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                         .mipLevel = static_cast<uint32_t>(srcSubresourceIndex),
	                         .baseArrayLayer = 0,
	                         .layerCount = 1},
	    .imageOffset = {boxMin.x, boxMin.y, boxMin.z},
	    .imageExtent = {static_cast<uint32_t>(boxMax.x - boxMin.x), static_cast<uint32_t>(boxMax.y - boxMin.y),
	                    static_cast<uint32_t>(boxMax.z - boxMin.z)},
	};

	vkCmdCopyImageToBuffer(m_commandBuffer, srcTexture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	                       destBuffer->GetVkBuffer(), 1, &region);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::CopyTextureRegion(const Ref<Texture>& dest, const glm::int3 destLoc,
                                                                       const int32_t destSubresourceIndex,
                                                                       const Ref<Texture>& src, const int32_t srcSubresourceIndex,
                                                                       Box3I srcBox)
{
	const auto* destTexture = dynamic_cast<VulkanTexture*>(dest.Raw());
	const auto* srcTexture = dynamic_cast<VulkanTexture*>(src.Raw());

	const glm::int3 boxMin = srcBox.location;
	const glm::int3 boxMax = srcBox.location + srcBox.size;

	const VkImageCopy region{
	    .srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                       .mipLevel = static_cast<uint32_t>(srcSubresourceIndex),
	                       .baseArrayLayer = 0,
	                       .layerCount = 1},
	    .srcOffset = {boxMin.x, boxMin.y, boxMin.z},
	    .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                       .mipLevel = static_cast<uint32_t>(destSubresourceIndex),
	                       .baseArrayLayer = 0,
	                       .layerCount = 1},
	    .dstOffset = {destLoc.x, destLoc.y, destLoc.z},
	    .extent = {static_cast<uint32_t>(boxMax.x - boxMin.x), static_cast<uint32_t>(boxMax.y - boxMin.y),
	               static_cast<uint32_t>(boxMax.z - boxMin.z)},
	};

	vkCmdCopyImage(m_commandBuffer, srcTexture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	               destTexture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::RenderImGui(Swapchain* swapchain, int32_t backbufferIndex,
                                                                 ImDrawData* drawData)
{}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetMarker(glm::float3 color, std::wstring string)
{
	if (vkCmdInsertDebugUtilsLabelEXT)
	{
		const VkDebugUtilsLabelEXT labelInfo{
		    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		    .color = {color.r, color.g, color.b, 1.0f},
		};

		vkCmdInsertDebugUtilsLabelEXT(m_commandBuffer, &labelInfo);
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandList::BeginEvent(glm::float3 color, std::wstring string)
{
	if (vkCmdBeginDebugUtilsLabelEXT)
	{
		const VkDebugUtilsLabelEXT labelInfo{
		    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		    .color = {color.r, color.g, color.b, 1.0f},
		};

		vkCmdBeginDebugUtilsLabelEXT(m_commandBuffer, &labelInfo);
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandList::EndEvent()
{
	if (vkCmdEndDebugUtilsLabelEXT)
	{
		vkCmdEndDebugUtilsLabelEXT(m_commandBuffer);
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandList::BindDescriptorSets(PipelineStateType type) {}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetupDrawOrDispatch(PipelineStateType type)
{
	if (type == PipelineStateType::Graphics)
	{
		BeginDynamicRendering();

		const VkPipeline pipeline = VulkanRenderDevice::Get().GetPipelineCache().GetOrCreatePipeline(
		    m_currentGraphicsPSO, m_renderTargetViews, m_depthStencilView);

		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}
	else if (type == PipelineStateType::Compute)
	{
		EndDynamicRendering();

		const VkPipeline pipeline = VulkanRenderDevice::Get().GetPipelineCache().GetOrCreatePipeline(m_currentComputePSO);

		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	}

	BindDescriptorSets(type);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::BeginDynamicRendering()
{
	if (m_renderingActive)
	{
		return;
	}

	std::vector<VkRenderingAttachmentInfo> colorAttachments;
	colorAttachments.reserve(m_renderTargetViews.size());

	for (const auto& rtv : m_renderTargetViews)
	{
		auto* view = dynamic_cast<VulkanTextureView*>(rtv.Raw());

		VkRenderingAttachmentInfo attachment{
		    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		    .imageView = view->GetVkImageView(),
		    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		    .resolveMode = VK_RESOLVE_MODE_NONE,
		    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
		    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		};

		colorAttachments.push_back(attachment);
	}

	VkRenderingAttachmentInfo depthAttachment{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};

	VkRenderingAttachmentInfo stencilAttachment{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};

	VkRenderingInfo renderingInfo{.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
	                              .layerCount = 1,
	                              .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
	                              .pColorAttachments = colorAttachments.data()};

	if (m_depthStencilView)
	{
		auto* dsv = dynamic_cast<VulkanTextureView*>(m_depthStencilView.Raw());

		depthAttachment.imageView = dsv->GetVkImageView();
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		stencilAttachment = depthAttachment;

		renderingInfo.pDepthAttachment = &depthAttachment;
		renderingInfo.pStencilAttachment = &stencilAttachment;
	}

	vkCmdBeginRendering(m_commandBuffer, &renderingInfo);

	m_renderingActive = true;
}

void Prism::Render::Vulkan::VulkanRenderCommandList::EndDynamicRendering()
{
	if (!m_renderingActive)
	{
		return;
	}

	vkCmdEndRendering(m_commandBuffer);

	m_renderingActive = false;
}
