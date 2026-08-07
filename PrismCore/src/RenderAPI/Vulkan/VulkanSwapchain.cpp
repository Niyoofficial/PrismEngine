#include "VulkanSwapchain.h"

#include <SDL3/SDL_vulkan.h>

#include "Prism/Base/Window.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderDevice.h"
#include "VulkanTypeConversions.h"

Prism::Render::Vulkan::VulkanSwapchain::VulkanSwapchain(Core::Window* window, SwapchainDesc desc) :
    Swapchain(desc), m_window(window), m_desc(desc)
{
	const auto sdlWindow = std::any_cast<SDL_Window*>(window->GetPlatformNativeWindow());

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

	auto it = std::ranges::find_if(formats, [desiredFormat](const VkSurfaceFormatKHR& f) { return f.format == desiredFormat; });

	PE_ASSERT(it != formats.end());

	m_surfaceFormat = *it;

	CreateSwapchain();
	CreateBackbuffers();

	VkSemaphoreCreateInfo semaphoreInfo{
	    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	const auto device = VulkanRenderDevice::Get().GetDevice();

	PE_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) == VK_SUCCESS);
	PE_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) == VK_SUCCESS);
}

Prism::Render::Vulkan::VulkanSwapchain::~VulkanSwapchain()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	if (m_imageAvailableSemaphore)
	{
		vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
	}

	if (m_renderFinishedSemaphore)
	{
		vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
	}

	DestroyBackbuffers();
	DestroySwapchain();

	if (m_surface)
	{
		SDL_Vulkan_DestroySurface(VulkanRenderDevice::Get().GetVulkanInstance(), m_surface, nullptr);
	}
}

void Prism::Render::Vulkan::VulkanSwapchain::Present()
{
	const auto* queue = VulkanRenderDevice::Get().GetVulkanRenderCommandQueue();

	const VkPresentInfoKHR presentInfo{
	    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	    .waitSemaphoreCount = 1,
	    .pWaitSemaphores = &m_renderFinishedSemaphore,
	    .swapchainCount = 1,
	    .pSwapchains = &m_swapchain,
	    .pImageIndices = &m_currentBackBufferIndex,
	};

	const VkResult result = vkQueuePresentKHR(queue->GetQueue(), &presentInfo);

	PE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
}

void Prism::Render::Vulkan::VulkanSwapchain::Resize()
{
	const VkSwapchainKHR oldSwapchain = m_swapchain;

	m_swapchain = VK_NULL_HANDLE;

	CreateSwapchain(oldSwapchain);

	vkDestroySwapchainKHR(VulkanRenderDevice::Get().GetDevice(), oldSwapchain, nullptr);
}

Prism::Render::TextureView* Prism::Render::Vulkan::VulkanSwapchain::GetBackBufferRTV(const int32_t index) const
{
	PE_ASSERT(index >= 0);
	PE_ASSERT(index < static_cast<int32_t>(m_backbufferRTVs.size()));

	return m_backbufferRTVs[index];
}

Prism::Render::TextureView* Prism::Render::Vulkan::VulkanSwapchain::GetCurrentBackBufferRTV() const
{
	return m_backbufferRTVs[m_currentBackBufferIndex];
}

uint32_t Prism::Render::Vulkan::VulkanSwapchain::AcquireNextImage()
{
	PE_ASSERT(vkAcquireNextImageKHR(VulkanRenderDevice::Get().GetDevice(), m_swapchain, UINT64_MAX, m_imageAvailableSemaphore,
	                                VK_NULL_HANDLE, &m_currentBackBufferIndex) == VK_SUCCESS);

	return m_currentBackBufferIndex;
}

void Prism::Render::Vulkan::VulkanSwapchain::CreateSwapchain(VkSwapchainKHR oldSwapchain)
{
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanRenderDevice::Get().GetPhysicalDevice(), m_surface, &presentModeCount,
	                                          nullptr);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanRenderDevice::Get().GetPhysicalDevice(), m_surface, &presentModeCount,
	                                          presentModes.data());

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	// TODO
	// implement proper present mode selection
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

void Prism::Render::Vulkan::VulkanSwapchain::DestroySwapchain()
{
	if (m_swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(VulkanRenderDevice::Get().GetDevice(), m_swapchain, nullptr);

		m_swapchain = VK_NULL_HANDLE;
	}
}

void Prism::Render::Vulkan::VulkanSwapchain::CreateBackbuffers()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	uint32_t imageCount = 0;
	PE_ASSERT(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr) == VK_SUCCESS);

	m_images.resize(imageCount);

	PE_ASSERT(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data()) == VK_SUCCESS);

	m_backbuffers.reserve(imageCount);
	m_backbufferRTVs.reserve(imageCount);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		TextureDesc desc = TextureDesc::CreateTex2D(L"Backbuffer_" + std::to_wstring(i), static_cast<int32_t>(m_extent.width),
		                                            static_cast<int32_t>(m_extent.height), m_desc.format, BindFlags::RenderTarget,
		                                            ResourceUsage::Default, 1);

		Ref<VulkanTexture> texture = Ref<VulkanTexture>::Create(&VulkanRenderDevice::Get(), m_images[i], desc);

		m_backbuffers.push_back(texture);

		TextureViewDesc viewDesc{
		    .type = TextureViewType::RTV,
		    .format = m_desc.format,
		    .dimension = ResourceDimension::Tex2D,
		};

		m_backbufferRTVs.push_back(texture->CreateView(viewDesc));
	}
}

void Prism::Render::Vulkan::VulkanSwapchain::DestroyBackbuffers()
{
	m_backbufferRTVs.clear();
	m_backbuffers.clear();
	m_images.clear();
}
