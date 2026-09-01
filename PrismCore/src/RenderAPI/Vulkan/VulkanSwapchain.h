#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "Prism/Render/RenderConstants.h"
#include "Prism/Render/Swapchain.h"
#include "VulkanTexture.h"

namespace Prism::Render::Vulkan
{
class VulkanSwapchain : public Swapchain
{
public:
	VulkanSwapchain(Core::Window* window, SwapchainDesc desc);
	~VulkanSwapchain() override;

	void PreparePresent() override;
	void Present() override;
	void Resize() override;

	[[nodiscard]] SwapchainDesc GetSwapchainDesc() const override { return m_desc; }

	[[nodiscard]] TextureView* GetBackBufferRTV(int32_t index) const override;
	[[nodiscard]] TextureView* GetCurrentBackBufferRTV() const override;

	[[nodiscard]] VkResult AcquireNextImage();

	[[nodiscard]] VkSemaphore GetImageAvailableSemaphore() const { return m_imageAvailableSemaphores[m_frameIndex]; }

	[[nodiscard]] VkSemaphore GetRenderFinishedSemaphore() const { return m_renderFinishedSemaphores[m_currentBackBufferIndex]; }

private:
	void CreateSwapchain(VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
	void DestroySwapchain();

	void CreateBackbuffers();
	void DestroyBackbuffers();

	void AdvanceFrame() { m_frameIndex = (m_frameIndex + 1) % FramesInFlight; }

	Core::Window* m_window;

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

	VkExtent2D m_extent;
	VkSurfaceFormatKHR m_surfaceFormat{};
	VkSurfaceCapabilitiesKHR m_surfaceCapabilities{};

	std::vector<VkImage> m_images;

	std::vector<Ref<VulkanTexture>> m_backbuffers;
	std::vector<TextureView*> m_backbufferRTVs;

	static constexpr uint32_t FramesInFlight = Constants::MAX_FRAMES_IN_FLIGHT;

	std::array<VkSemaphore, FramesInFlight> m_imageAvailableSemaphores{};
	std::vector<VkSemaphore> m_renderFinishedSemaphores;

	uint32_t m_frameIndex = 0;

	SwapchainDesc m_desc;
};
} // namespace Prism::Render::Vulkan
