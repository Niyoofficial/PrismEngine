#include "VulkanRenderCommandList.h"

#include <vulkan/vulkan_core.h>

#include "VulkanBuffer.h"
#include "VulkanBufferView.h"
#include "VulkanPipelineLayoutCache.h"
#include "VulkanRenderDevice.h"
#include "VulkanTexture.h"
#include "VulkanTextureView.h"
#include "VulkanTypeConversions.h"

namespace
{
PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXTFunction = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXTFunction = nullptr;
PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXTFunction = nullptr;
} // namespace

Prism::Render::Vulkan::VulkanRenderCommandList::VulkanRenderCommandList()
{
	const auto& device = VulkanRenderDevice::Get();

	vkCmdBeginDebugUtilsLabelEXTFunction = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
	    vkGetDeviceProcAddr(device.GetDevice(), "vkCmdBeginDebugUtilsLabelEXT"));

	vkCmdEndDebugUtilsLabelEXTFunction =
	    reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device.GetDevice(), "vkCmdEndDebugUtilsLabelEXT"));

	vkCmdInsertDebugUtilsLabelEXTFunction = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(
	    vkGetDeviceProcAddr(device.GetDevice(), "vkCmdInsertDebugUtilsLabelEXT"));

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
	PE_RENDER_LOG(Info, "Vulkan SetPSO graphics: desc={}, vs={}, ps={}", static_cast<const void*>(&desc),
	              static_cast<const void*>(&desc.vs), static_cast<const void*>(&desc.ps));

	m_currentGraphicsPSO = desc;


	PE_RENDER_LOG(Info, "Vulkan SetPSO AFTER COPY: current={}, vs={}, ps={}", static_cast<const void*>(&m_currentGraphicsPSO),
	              static_cast<const void*>(&m_currentGraphicsPSO.vs), static_cast<const void*>(&m_currentGraphicsPSO.ps));
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

	glm::float4 rtClearColor{0.f, 0.f, 0.f, 0.f};
	if (clearColor)
	{
		rtClearColor.x = clearColor->x;
		rtClearColor.y = clearColor->y;
		rtClearColor.z = clearColor->z;
		rtClearColor.w = clearColor->w;
	}

	const VkClearColorValue vkClearColor{
	    .float32 = {rtClearColor.x, rtClearColor.y, rtClearColor.z, rtClearColor.w},
	};

	constexpr VkImageSubresourceRange range{
	    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	    .baseMipLevel = 0,
	    .levelCount = VK_REMAINING_MIP_LEVELS,
	    .baseArrayLayer = 0,
	    .layerCount = VK_REMAINING_ARRAY_LAYERS,
	};

	const auto* texture = dynamic_cast<VulkanTexture*>(view->GetTexture());

	vkCmdClearColorImage(m_commandBuffer, texture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_GENERAL, &vkClearColor, 1,
	                     &range);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::ClearDepthStencilView(const Ref<TextureView>& dsv, Flags<ClearFlags> flags,
                                                                           DepthStencilValue* clearValue)
{
	const auto* view = dynamic_cast<VulkanTextureView*>(dsv.Raw());

	const DepthStencilValue depthStencilValue = clearValue ? *clearValue : DepthStencilValue{.depth = 1.0f, .stencil = 0};

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
	    .depth = depthStencilValue.depth,
	    .stencil = depthStencilValue.stencil,
	};

	const VkImageSubresourceRange range{
	    .aspectMask = aspectMask,
	    .baseMipLevel = 0,
	    .levelCount = VK_REMAINING_MIP_LEVELS,
	    .baseArrayLayer = 0,
	    .layerCount = VK_REMAINING_ARRAY_LAYERS,
	};

	const auto* texture = dynamic_cast<VulkanTexture*>(view->GetTexture());

	vkCmdClearDepthStencilImage(m_commandBuffer, texture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_GENERAL,
	                            &clearDepthStencil, 1, &range);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::float4 values)
{
	const auto* view = dynamic_cast<VulkanTextureView*>(uav.Raw());

	const VkClearColorValue vkClearColor{
	    .float32 = {values.r, values.g, values.b, values.a},
	};

	const auto* texture = dynamic_cast<VulkanTexture*>(view->GetTexture());

	vkCmdClearColorImage(m_commandBuffer, texture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_GENERAL, &vkClearColor, 1,
	                     nullptr);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::uint4 values)
{
	const auto* view = dynamic_cast<VulkanTextureView*>(uav.Raw());

	const VkClearColorValue clearValue{
	    .uint32 = {values.r, values.g, values.b, values.a},
	};

	const auto* texture = dynamic_cast<VulkanTexture*>(view->GetTexture());

	vkCmdClearColorImage(m_commandBuffer, texture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1,
	                     nullptr);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::Barrier(const BufferBarrier barrier)
{
	const auto* buffer = dynamic_cast<VulkanBuffer*>(barrier.buffer);

	const VkBufferMemoryBarrier bufferBarrier{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
	    .srcAccessMask = GetVkAccessFlags(barrier.accessBefore),
	    .dstAccessMask = GetVkAccessFlags(barrier.accessAfter),
	    .buffer = buffer->GetVkBuffer(),
	    .offset = static_cast<VkDeviceSize>(barrier.offset),
	    .size = barrier.size == -1 ? VK_WHOLE_SIZE : static_cast<VkDeviceSize>(barrier.size),
	};

	const auto srcStage = GetVkPipelineStageFlags(barrier.syncBefore);
	const auto dstStage = GetVkPipelineStageFlags(barrier.syncAfter);

	vkCmdPipelineBarrier(m_commandBuffer, srcStage, dstStage, 0, 0, nullptr, 1, &bufferBarrier, 0, nullptr);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::Barrier(const TextureBarrier barrier)
{
	const auto* texture = dynamic_cast<VulkanTexture*>(barrier.texture);
	PE_ASSERT(texture);

	const auto& [firstMipLevel, numMipLevels, firstArraySlice, numArraySlices] = barrier.subresourceRange;

	const VkImageMemoryBarrier imageBarrier{
	    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	    .srcAccessMask = GetVkAccessFlags(barrier.accessBefore),
	    .dstAccessMask = GetVkAccessFlags(barrier.accessAfter),
	    .oldLayout = GetVkImageLayout(barrier.layoutBefore),
	    .newLayout = GetVkImageLayout(barrier.layoutAfter),
	    .image = texture->GetVulkanTextureResource().image,
	    .subresourceRange =
	        {
	            .aspectMask = GetVkImageAspectFlags(texture->GetTextureDesc().format),
	            .baseMipLevel = static_cast<uint32_t>(firstMipLevel),
	            .levelCount = static_cast<uint32_t>(numMipLevels),
	            .baseArrayLayer = static_cast<uint32_t>(firstArraySlice),
	            .layerCount = static_cast<uint32_t>(numArraySlices),
	        },
	};

	const auto srcStage = GetVkPipelineStageFlags(barrier.syncBefore);
	const auto dstStage = GetVkPipelineStageFlags(barrier.syncAfter);

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
	PE_ASSERT(texture);
	PE_ASSERT(data.data);
	PE_ASSERT(data.sizeInBytes > 0);

	const BufferDesc bufferDesc{
	    .size = data.sizeInBytes,
	    .bindFlags = BindFlags::None,
	    .usage = ResourceUsage::Staging,
	    .cpuAccess = CPUAccess::Write,
	};

	const auto stagingBuffer = VulkanRenderDevice::Get().CreateBuffer(bufferDesc);

	void* mapped = stagingBuffer->Map(CPUAccess::Write);
	PE_ASSERT(mapped);
	memcpy(mapped, data.data, data.sizeInBytes);
	stagingBuffer->Unmap();

	const auto* vulkanBuffer = dynamic_cast<VulkanBuffer*>(stagingBuffer.Raw());
	const auto* vulkanTexture = dynamic_cast<VulkanTexture*>(texture.Raw());

	const VkImage image = vulkanTexture->GetVulkanTextureResource().image;
	const TextureDesc& desc = vulkanTexture->GetTextureDesc();

	const VkBufferImageCopy region{
	    .bufferOffset = 0,
	    .imageSubresource =
	        {
	            .aspectMask = GetVkImageAspectFlags(desc.format),
	            .mipLevel = static_cast<uint32_t>(subresourceIndex),
	            .baseArrayLayer = 0,
	            .layerCount = desc.Is3D() ? 1u : static_cast<uint32_t>(desc.GetArraySize()),
	        },
	    .imageOffset = {0, 0, 0},
	    .imageExtent =
	        {
	            static_cast<uint32_t>(vulkanTexture->GetTextureDesc().GetWidth()),
	            static_cast<uint32_t>(vulkanTexture->GetTextureDesc().GetHeight()),
	            desc.Is3D() ? static_cast<uint32_t>(desc.GetDepth()) : 1u,
	        },
	};

	vkCmdCopyBufferToImage(m_commandBuffer, vulkanBuffer->GetVkBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	KeepAlive(stagingBuffer);
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
	    .imageSubresource =
	        {
	            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	            .mipLevel = static_cast<uint32_t>(srcSubresourceIndex),
	            .baseArrayLayer = 0,
	            .layerCount = 1,
	        },
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

	const auto& destDesc = dest->GetTextureDesc();
	const auto& srcDesc = src->GetTextureDesc();

	const auto srcMipLevels = static_cast<uint32_t>(srcDesc.GetMipLevels());
	const auto destMipLevels = static_cast<uint32_t>(destDesc.GetMipLevels());

	const auto srcSubresource = static_cast<uint32_t>(srcSubresourceIndex);
	const auto destSubresource = static_cast<uint32_t>(destSubresourceIndex);

	const uint32_t srcMipLevel = srcSubresource % srcMipLevels;
	const uint32_t srcArrayLayer = srcSubresource / srcMipLevels;

	const uint32_t destMipLevel = destSubresource % destMipLevels;
	const uint32_t destArrayLayer = destSubresource / destMipLevels;

	PE_ASSERT(srcArrayLayer < static_cast<uint32_t>(srcDesc.GetArraySize()));
	PE_ASSERT(destArrayLayer < static_cast<uint32_t>(destDesc.GetArraySize()));

	const uint32_t srcMipWidth = std::max(1u, static_cast<uint32_t>(srcDesc.GetWidth()) >> srcMipLevel);
	const uint32_t srcMipHeight = std::max(1u, static_cast<uint32_t>(srcDesc.GetHeight()) >> srcMipLevel);
	const uint32_t srcMipDepth = srcDesc.Is3D() ? std::max(1u, static_cast<uint32_t>(srcDesc.GetDepth()) >> srcMipLevel) : 1u;

	const bool copyWholeSubresource = srcBox.size.x <= 0 || srcBox.size.y <= 0 || srcBox.size.z <= 0;

	VkOffset3D srcOffset{};
	VkExtent3D extent{};

	if (copyWholeSubresource)
	{
		srcOffset = {0, 0, 0};

		extent = {srcMipWidth, srcMipHeight, srcMipDepth};
	}
	else
	{
		srcOffset = {srcBox.location.x, srcBox.location.y, srcBox.location.z};
		extent = {static_cast<uint32_t>(srcBox.size.x), static_cast<uint32_t>(srcBox.size.y),
		          static_cast<uint32_t>(srcBox.size.z)};

		// For 1D/2D Vulkan images depth must be 1
		if (!srcDesc.Is3D())
		{
			extent.depth = 1;
			srcOffset.z = 0;
		}
	}

	const VkOffset3D dstOffset{destLoc.x, destLoc.y, destDesc.Is3D() ? destLoc.z : 0};

	if (!srcDesc.Is3D() && !destDesc.Is3D())
	{
		extent.depth = 1;
	}

	PE_ASSERT(extent.width > 0);
	PE_ASSERT(extent.height > 0);
	PE_ASSERT(extent.depth > 0);

	const VkImageCopy region{
	    .srcSubresource =
	        {
	            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	            .mipLevel = srcMipLevel,
	            .baseArrayLayer = srcArrayLayer,
	            .layerCount = 1,
	        },
	    .srcOffset = srcOffset,
	    .dstSubresource =
	        {
	            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	            .mipLevel = destMipLevel,
	            .baseArrayLayer = destArrayLayer,
	            .layerCount = 1,
	        },
	    .dstOffset = dstOffset,
	    .extent = extent,
	};

	vkCmdCopyImage(m_commandBuffer, srcTexture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	               destTexture->GetVulkanTextureResource().image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::RenderImGui(Swapchain* swapchain, int32_t backbufferIndex,
                                                                 ImDrawData* drawData)
{
	if (!drawData)
	{
		return;
	}

	if (m_renderingActive)
	{
		EndDynamicRendering();
	}

	const auto backbuffer = swapchain->GetBackBufferRTV(backbufferIndex);

	m_renderTargetViews.clear();
	m_renderTargetViews.push_back(backbuffer);
	m_depthStencilView = nullptr;

	BeginDynamicRendering();

	ImGui_ImplVulkan_RenderDrawData(drawData, m_commandBuffer);

	EndDynamicRendering();
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetMarker(glm::float3 color, std::wstring string)
{
	const std::string utf8LabelName = WStringToString(string);

	const VkDebugUtilsLabelEXT labelInfo{
	    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
	    .pLabelName = utf8LabelName.c_str(),
	    .color = {color.r, color.g, color.b, 1.0f},
	};

	if (vkCmdInsertDebugUtilsLabelEXTFunction)
	{
		vkCmdInsertDebugUtilsLabelEXTFunction(m_commandBuffer, &labelInfo);
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandList::BeginEvent(glm::float3 color, std::wstring string)
{
	const std::string utf8LabelName = WStringToString(string);

	const VkDebugUtilsLabelEXT labelInfo{
	    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
	    .pLabelName = utf8LabelName.c_str(),
	    .color = {color.r, color.g, color.b, 1.0f},
	};

	if (vkCmdBeginDebugUtilsLabelEXTFunction)
	{
		vkCmdBeginDebugUtilsLabelEXTFunction(m_commandBuffer, &labelInfo);
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandList::EndEvent()
{
	if (vkCmdEndDebugUtilsLabelEXTFunction)
	{
		vkCmdEndDebugUtilsLabelEXTFunction(m_commandBuffer);
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandList::KeepAlive(const Ref<Buffer>& buffer)
{
	m_pendingStagingBuffers.push_back(buffer);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::Finalize() { Close(); }

void Prism::Render::Vulkan::VulkanRenderCommandList::Close()
{
	RenderCommandList::Close();

	EndDynamicRendering();

	PE_ASSERT(vkEndCommandBuffer(m_commandBuffer) == VK_SUCCESS);
}

void Prism::Render::Vulkan::VulkanRenderCommandList::BindDescriptorSets(PipelineStateType type)
{
	auto& device = VulkanRenderDevice::Get();

	std::vector<const VulkanShaderReflection*> reflections;

	auto* compiler = device.GetVulkanShaderCompiler();

	if (type == PipelineStateType::Graphics)
	{
		const auto& vs = compiler->GetOrCreateShader(m_currentGraphicsPSO.vs);
		const auto& ps = compiler->GetOrCreateShader(m_currentGraphicsPSO.ps);
		reflections = {&vs.reflection, &ps.reflection};
	}
	else
	{
		const auto& cs = compiler->GetOrCreateShader(m_currentComputePSO.cs);
		reflections = {&cs.reflection};
	}

	const VkPipelineLayout pipelineLayout = device.GetPipelineLayoutCache()->GetOrCreatePipelineLayout(reflections);

	const VkPipelineBindPoint bindPoint =
	    type == PipelineStateType::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

	VkDescriptorSet bindlessSet = device.GetBindlessManager().GetSet();
	vkCmdBindDescriptorSets(m_commandBuffer, bindPoint, pipelineLayout, 0, 1, &bindlessSet, 0, nullptr);

	std::array<uint8_t, 128> pushConstantData{};
	uint32_t pushConstantOffset = 0;
	uint32_t pushConstantEnd = 0;
	bool hasPushConstants = false;

	for (const auto* reflection : reflections)
	{
		for (uint32_t i = 0; i < reflection->GetPushConstantBlockCount(); ++i)
		{
			const auto& block = reflection->GetPushConstantBlock(i);

			hasPushConstants = true;
			pushConstantOffset = block.offset;
			pushConstantEnd = std::max(pushConstantEnd, block.offset + block.size);

			for (uint32_t m = 0; m < block.member_count; ++m)
			{
				const auto& member = block.members[m];

				if (!member.name)
				{
					continue;
				}

				const std::wstring memberName = StringToWString(member.name);

				auto it = m_boundResources.find(memberName);
				if (it == m_boundResources.end() || it->second.empty())
				{
					continue;
				}

				RenderResourceView* resourceView = it->second[0].Raw();

				uint32_t bindlessIndex = UINT32_MAX;

				if (auto* textureView = dynamic_cast<VulkanTextureView*>(resourceView))
				{
					bindlessIndex = textureView->GetBindlessIndex();
				}
				else if (auto* bufferView = dynamic_cast<VulkanBufferView*>(resourceView))
				{
					bindlessIndex = bufferView->GetBindlessIndex();
				}

				if (bindlessIndex != UINT32_MAX)
				{
					PE_ASSERT(member.offset + sizeof(uint32_t) <= pushConstantData.size(), "Resources block too large");
					std::memcpy(pushConstantData.data() + member.offset, &bindlessIndex, sizeof(uint32_t));
				}
			}
		}
	}

	if (hasPushConstants)
	{
		const VkShaderStageFlags stageFlags = type == PipelineStateType::Graphics
		    ? (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
		    : VK_SHADER_STAGE_COMPUTE_BIT;

		vkCmdPushConstants(m_commandBuffer, pipelineLayout, stageFlags, pushConstantOffset, pushConstantEnd - pushConstantOffset,
		                   pushConstantData.data());
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandList::SetupDrawOrDispatch(PipelineStateType type)
{
	PE_ASSERT(type == PipelineStateType::Graphics && m_currentGraphicsPSO.IsValid() ||
	          type == PipelineStateType::Compute && m_currentComputePSO.IsValid());

	if (type == PipelineStateType::Graphics)
	{
		BeginDynamicRendering();

		const VkPipeline pipeline = VulkanRenderDevice::Get().GetPipelineCache()->GetOrCreatePipeline(
		    m_currentGraphicsPSO, m_renderTargetViews, m_depthStencilView);

		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}
	else if (type == PipelineStateType::Compute)
	{
		EndDynamicRendering();

		const VkPipeline pipeline = VulkanRenderDevice::Get().GetPipelineCache()->GetOrCreatePipeline(m_currentComputePSO);

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
		const auto* view = dynamic_cast<VulkanTextureView*>(rtv.Raw());
		PE_ASSERT(view);

		const VkRenderingAttachmentInfo attachment{
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

	// TODO
	// what to do if we have no render targets? We need to support this for compute shaders that write to UAVs
	PE_ASSERT(!m_renderTargetViews.empty(), "BeginDynamicRendering requires at least one render target");
	const auto* firstRTV = dynamic_cast<VulkanTextureView*>(m_renderTargetViews[0].Raw());

	const TextureDesc& textureDesc = dynamic_cast<VulkanTexture*>(firstRTV->GetTexture())->GetTextureDesc();


	VkRenderingInfo renderingInfo{
	    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
	    .renderArea =
	        {
	            .offset = {0, 0},
	            .extent =
	                {
	                    static_cast<uint32_t>(textureDesc.GetWidth()),
	                    static_cast<uint32_t>(textureDesc.GetHeight()),
	                },
	        },
	    .layerCount = 1,
	    .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
	    .pColorAttachments = colorAttachments.data(),
	};

	if (m_depthStencilView)
	{
		const auto* dsv = dynamic_cast<VulkanTextureView*>(m_depthStencilView.Raw());
		PE_ASSERT(dsv);

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
