#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/Swapchain.h"
#include "VulkanTexture.h"

namespace Prism::Render::Vulkan
{
class VulkanSwapchain : public Swapchain
{
public:
	VulkanSwapchain(Core::Window* window, SwapchainDesc desc);
	~VulkanSwapchain() override;

	void Present() override;
	void Resize() override;

	[[nodiscard]] SwapchainDesc GetSwapchainDesc() const override { return m_desc; }

	[[nodiscard]] TextureView* GetBackBufferRTV(int32_t index) const override;
	[[nodiscard]] TextureView* GetCurrentBackBufferRTV() const override;

	[[nodiscard]] uint32_t AcquireNextImage();

	VkSemaphore GetImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }

	VkSemaphore GetRenderFinishedSemaphore() const { return m_renderFinishedSemaphore; }

private:
	void CreateSwapchain(VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
	void DestroySwapchain();

	void CreateBackbuffers();
	void DestroyBackbuffers();

	Core::Window* m_window;

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

	VkExtent2D m_extent;
	VkSurfaceFormatKHR m_surfaceFormat{};
	VkSurfaceCapabilitiesKHR m_surfaceCapabilities{};

	std::vector<VkImage> m_images;

	std::vector<Ref<VulkanTexture>> m_backbuffers;
	std::vector<TextureView*> m_backbufferRTVs;

	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;

	SwapchainDesc m_desc;
};
} // namespace Prism::Render::Vulkan
