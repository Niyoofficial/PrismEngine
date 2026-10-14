#include "VulkanSwapchain.h"

#include <SDL3/SDL_vulkan.h>

#include "Prism/Base/Platform.h"
#include "Prism/Base/Window.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderDevice.h"
#include "VulkanTypeConversions.h"

namespace Prism::Render::Vulkan
{
VulkanSwapchain::VulkanSwapchain(Core::Window* window, SwapchainDesc desc) : Swapchain(desc), m_window(window), m_desc(desc)
{
	const auto sdlWindow = std::any_cast<SDL_Window*>(window->GetNativeWindow());

	PE_ASSERT(SDL_Vulkan_CreateSurface(sdlWindow, VulkanRenderDevice::Get().GetVulkanInstance(), nullptr, &m_surface));

	VkBool32 supported = VK_FALSE;

	vkGetPhysicalDeviceSurfaceSupportKHR(VulkanRenderDevice::Get().GetPhysicalDevice(),
										 VulkanRenderDevice::Get().GetGraphicsQueueFamilyIndex(), m_surface, &supported);

	PE_ASSERT(supported);

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanRenderDevice::Get().GetPhysicalDevice(), m_surface, &m_surfaceCapabilities);

	m_extent = m_surfaceCapabilities.currentExtent;

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanRenderDevice::Get().GetPhysicalDevice(), m_surface, &formatCount, nullptr);

	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanRenderDevice::Get().GetPhysicalDevice(), m_surface, &formatCount, formats.data());

	VkFormat desiredFormat = GetVkFormat(desc.format);

	const auto it =
		std::ranges::find_if(formats, [desiredFormat](const VkSurfaceFormatKHR& f) { return f.format == desiredFormat; });

	PE_ASSERT(it != formats.end());

	m_surfaceFormat = *it;

	CreateSwapchain();
	CreateSwapchainImages();
	CreateRenderFinishedSemaphores();
	CreateBackbuffers();
	CreatePresentFrames();

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::WindowResized>([this](Core::AppEvent event) { Resize(); });
}

VulkanSwapchain::~VulkanSwapchain()
{
	vkDeviceWaitIdle(VulkanRenderDevice::Get().GetDevice());

	DestroyPresentFrames();
	DestroyRenderFinishedSemaphores();
	DestroyBackbuffers();
	DestroySwapchain();

	if (m_surface)
	{
		SDL_Vulkan_DestroySurface(VulkanRenderDevice::Get().GetVulkanInstance(), m_surface, nullptr);
	}
}

void VulkanSwapchain::PrepareFrame()
{
	if (!m_backbuffersNeedInitialTransition)
	{
		return;
	}

	auto& device = VulkanRenderDevice::Get();
	auto context = device.AllocateContext(L"InitializeBackbufferPresentLayout");

	for (auto& backbuffer : m_backbuffers)
	{
		context->Barrier({
			.texture = backbuffer.Raw(),
			.syncBefore = BarrierSync::None,
			.syncAfter = BarrierSync::None,
			.accessBefore = BarrierAccess::NoAccess,
			.accessAfter = BarrierAccess::NoAccess,
			.layoutBefore = BarrierLayout::Undefined,
			.layoutAfter = BarrierLayout::Present,
		});
	}

	device.SubmitContext(context);

	m_backbuffersNeedInitialTransition = false;
}

void VulkanSwapchain::Present(const int32_t backBufferIndex)
{
	std::scoped_lock presentLock(m_presentMutex);

	const VkDevice device = VulkanRenderDevice::Get().GetDevice();
	auto* renderCommandQueue = VulkanRenderDevice::Get().GetVulkanRenderCommandQueue();

	PE_ASSERT(backBufferIndex >= 0 && backBufferIndex < static_cast<int32_t>(m_backbuffers.size()));

	const PresentFrame& frame = m_presentFrames[m_presentFrameIndex];
	vkWaitForFences(device, 1, &frame.fence, VK_TRUE, UINT64_MAX);

	uint32_t swapchainImageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(device, m_swapchain, UINT64_MAX, frame.acquire, VK_NULL_HANDLE, &swapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		return;
	}

	PE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

	vkResetFences(device, 1, &frame.fence);
	vkResetCommandPool(device, frame.pool, 0);

	const VkImage backbufferImage = m_backbuffers[backBufferIndex]->GetVulkanTextureResource().image;
	const VkImage swapchainImage = m_images[swapchainImageIndex];

	const VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(frame.cmd, &beginInfo);

	auto MakeImageBarrier = [](VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags2 sourceStage,
							   VkAccessFlags2 sourceAccess, VkPipelineStageFlags2 destinationStage,
							   VkAccessFlags2 destinationAccess)
	{
		return VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = sourceStage,
			.srcAccessMask = sourceAccess,
			.dstStageMask = destinationStage,
			.dstAccessMask = destinationAccess,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
		};
	};

	const VkImageMemoryBarrier2 beforeBlitBarriers[2] = {
		MakeImageBarrier(backbufferImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						 VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
						 VK_ACCESS_2_TRANSFER_READ_BIT),
		MakeImageBarrier(swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						 VK_PIPELINE_STAGE_2_TRANSFER_BIT, 0, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT),
	};
	const VkDependencyInfo beforeBlitDependency{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 2,
		.pImageMemoryBarriers = beforeBlitBarriers,
	};
	vkCmdPipelineBarrier2(frame.cmd, &beforeBlitDependency);

	const VkImageBlit blitRegion{
		.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
		.srcOffsets = {{0, 0, 0},
					   {static_cast<int32_t>(m_backbufferExtent.width), static_cast<int32_t>(m_backbufferExtent.height), 1}},
		.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
		.dstOffsets = {{0, 0, 0}, {static_cast<int32_t>(m_extent.width), static_cast<int32_t>(m_extent.height), 1}},
	};
	vkCmdBlitImage(frame.cmd, backbufferImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchainImage,
				   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_LINEAR);

	const VkImageMemoryBarrier2 afterBlitBarriers[2] = {
		MakeImageBarrier(backbufferImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						 VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
						 0),
		MakeImageBarrier(swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						 VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, 0),
	};
	const VkDependencyInfo afterBlitDependency{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 2,
		.pImageMemoryBarriers = afterBlitBarriers,
	};
	vkCmdPipelineBarrier2(frame.cmd, &afterBlitDependency);

	vkEndCommandBuffer(frame.cmd);

	const VkSemaphore renderFinishedSemaphore = m_renderFinishedSemaphores[swapchainImageIndex];

	const VkSemaphoreSubmitInfo waitSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = frame.acquire,
		.stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
	};
	const VkSemaphoreSubmitInfo signalSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = renderFinishedSemaphore,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
	};
	const VkCommandBufferSubmitInfo commandBufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = frame.cmd,
	};
	const VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &waitSemaphoreInfo,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &commandBufferInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo,
	};

	const VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderFinishedSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &m_swapchain,
		.pImageIndices = &swapchainImageIndex,
	};

	{
		std::scoped_lock queueLock(renderCommandQueue->GetSubmitMutex());

		PE_ASSERT(vkQueueSubmit2(renderCommandQueue->GetQueue(), 1, &submitInfo, frame.fence) == VK_SUCCESS);

		result = vkQueuePresentKHR(renderCommandQueue->GetQueue(), &presentInfo);
	}

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
	}
	else
	{
		PE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
	}

	m_presentFrameIndex = (m_presentFrameIndex + 1) % static_cast<uint32_t>(m_presentFrames.size());
}

void VulkanSwapchain::Resize()
{
	const auto& device = VulkanRenderDevice::Get();

	device.GetVulkanRenderCommandQueue()->Flush(CommandQueueFlushType::WaitForCompletion);

	std::scoped_lock lock(m_presentMutex);

	if (!RecreateSwapchain())
	{
		return;
	}

	DestroyBackbuffers();
	CreateBackbuffers();

	m_backbuffersNeedInitialTransition = true;
	m_currentBackBufferIndex = 0;
}

TextureView* VulkanSwapchain::GetBackBufferRTV(const int32_t index) const
{
	PE_ASSERT(index >= 0);
	PE_ASSERT(index < static_cast<int32_t>(m_backbufferRTVs.size()));

	return m_backbufferRTVs[index];
}

TextureView* VulkanSwapchain::GetCurrentBackBufferRTV() const { return m_backbufferRTVs[m_currentBackBufferIndex]; }

void VulkanSwapchain::CreateSwapchain(VkSwapchainKHR oldSwapchain)
{
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanRenderDevice::Get().GetPhysicalDevice(), m_surface, &presentModeCount,
											  nullptr);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanRenderDevice::Get().GetPhysicalDevice(), m_surface, &presentModeCount,
											  presentModes.data());

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const VkPresentModeKHR mode : presentModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = mode;
			break;
		}
	}

	uint32_t imageCount = std::max(m_surfaceCapabilities.minImageCount, static_cast<uint32_t>(m_desc.bufferCount));

	if (m_surfaceCapabilities.maxImageCount > 0)
	{
		imageCount = std::min(imageCount, m_surfaceCapabilities.maxImageCount);
	}

	const VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_surface,
		.minImageCount = imageCount,
		.imageFormat = m_surfaceFormat.format,
		.imageColorSpace = m_surfaceFormat.colorSpace,
		.imageExtent = m_extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = m_surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = oldSwapchain,
	};

	PE_ASSERT(vkCreateSwapchainKHR(VulkanRenderDevice::Get().GetDevice(), &createInfo, nullptr, &m_swapchain) == VK_SUCCESS);
}

void VulkanSwapchain::DestroySwapchain()
{
	if (m_swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(VulkanRenderDevice::Get().GetDevice(), m_swapchain, nullptr);

		m_swapchain = VK_NULL_HANDLE;
	}
}

bool VulkanSwapchain::RecreateSwapchain()
{
	const auto& device = VulkanRenderDevice::Get();

	VkSurfaceCapabilitiesKHR capabilities{};
	PE_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysicalDevice(), m_surface, &capabilities) == VK_SUCCESS);

	if (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0)
	{
		return false;
	}

	vkDeviceWaitIdle(device.GetDevice());

	m_surfaceCapabilities = capabilities;
	m_extent = capabilities.currentExtent;

	const VkSwapchainKHR oldSwapchain = m_swapchain;

	DestroyRenderFinishedSemaphores();

	CreateSwapchain(oldSwapchain);

	vkDestroySwapchainKHR(device.GetDevice(), oldSwapchain, nullptr);

	CreateSwapchainImages();
	CreateRenderFinishedSemaphores();

	return true;
}

void VulkanSwapchain::CreateSwapchainImages()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	uint32_t imageCount = 0;
	PE_ASSERT(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr) == VK_SUCCESS);

	m_images.resize(imageCount);

	PE_ASSERT(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data()) == VK_SUCCESS);
}

void VulkanSwapchain::CreateBackbuffers()
{
	auto& device = VulkanRenderDevice::Get();

	m_backbufferExtent = m_extent;

	m_backbuffers.reserve(m_desc.bufferCount);
	m_backbufferRTVs.reserve(m_desc.bufferCount);

	for (int32_t i = 0; i < m_desc.bufferCount; i++)
	{
		TextureDesc desc = TextureDesc::CreateTex2D(L"Backbuffer_" + std::to_wstring(i), static_cast<int32_t>(m_extent.width),
													static_cast<int32_t>(m_extent.height), m_desc.format, BindFlags::RenderTarget,
													ResourceUsage::Default, 1);

		Ref<VulkanTexture> texture = Ref<VulkanTexture>::Create(&device, desc, BarrierLayout::Undefined);

		m_backbuffers.push_back(texture);

		TextureViewDesc viewDesc{
			.type = TextureViewType::RTV,
			.format = m_desc.format,
			.dimension = ResourceDimension::Tex2D,
		};

		m_backbufferRTVs.push_back(texture->CreateView(viewDesc));
	}
}

void VulkanSwapchain::DestroyBackbuffers()
{
	m_backbufferRTVs.clear();
	m_backbuffers.clear();
}

void VulkanSwapchain::CreateRenderFinishedSemaphores()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	constexpr VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	m_renderFinishedSemaphores.resize(m_images.size());

	for (auto& semaphore : m_renderFinishedSemaphores)
	{
		PE_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) == VK_SUCCESS);
	}
}

void VulkanSwapchain::DestroyRenderFinishedSemaphores()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	for (const auto semaphore : m_renderFinishedSemaphores)
	{
		vkDestroySemaphore(device, semaphore, nullptr);
	}

	m_renderFinishedSemaphores.clear();
}

void VulkanSwapchain::CreatePresentFrames()
{
	const VkDevice device = VulkanRenderDevice::Get().GetDevice();

	for (auto& frame : m_presentFrames)
	{
		const VkCommandPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = VulkanRenderDevice::Get().GetGraphicsQueueFamilyIndex(),
		};
		PE_ASSERT(vkCreateCommandPool(device, &poolInfo, nullptr, &frame.pool) == VK_SUCCESS);

		const VkCommandBufferAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = frame.pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		PE_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, &frame.cmd) == VK_SUCCESS);

		constexpr VkFenceCreateInfo fenceInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		PE_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &frame.fence) == VK_SUCCESS);

		constexpr VkSemaphoreCreateInfo semaphoreInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		PE_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.acquire) == VK_SUCCESS);
	}
}

void VulkanSwapchain::DestroyPresentFrames()
{
	const VkDevice device = VulkanRenderDevice::Get().GetDevice();

	for (auto& frame : m_presentFrames)
	{
		vkDestroySemaphore(device, frame.acquire, nullptr);
		vkDestroyFence(device, frame.fence, nullptr);
		vkDestroyCommandPool(device, frame.pool, nullptr);

		frame = {};
	}
}
} // namespace Prism::Render::Vulkan
