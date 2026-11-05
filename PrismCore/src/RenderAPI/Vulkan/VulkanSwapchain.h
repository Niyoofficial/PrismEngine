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

	void Present(int32_t backBufferIndex) override;
	void Resize() override;

	void PrepareFrame();

	[[nodiscard]] SwapchainDesc GetSwapchainDesc() const override { return m_desc; }

	[[nodiscard]] TextureView* GetBackBufferRTV(int32_t index) const override;
	[[nodiscard]] TextureView* GetCurrentBackBufferRTV() const override;

private:
	void CreateSwapchain(VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
	void DestroySwapchain();

	bool RecreateSwapchain();
	void CreateSwapchainImages();

	void CreateBackbuffers();
	void DestroyBackbuffers();

	void CreateRenderFinishedSemaphores();
	void DestroyRenderFinishedSemaphores();

	void CreatePresentFrames();
	void DestroyPresentFrames();

	WeakRef<Core::Window> m_window;

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

	VkExtent2D m_extent{};
	VkExtent2D m_backbufferExtent{};
	VkSurfaceFormatKHR m_surfaceFormat{};
	VkSurfaceCapabilitiesKHR m_surfaceCapabilities{};

	std::vector<VkImage> m_images;

	std::vector<Ref<VulkanTexture>> m_backbuffers;
	std::vector<TextureView*> m_backbufferRTVs;

	bool m_backbuffersNeedInitialTransition = true;

	std::vector<VkSemaphore> m_renderFinishedSemaphores;

	struct PresentFrame
	{
		VkCommandPool pool = VK_NULL_HANDLE;
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		VkFence fence = VK_NULL_HANDLE;
		VkSemaphore acquire = VK_NULL_HANDLE;
	};

	std::array<PresentFrame, Constants::MAX_FRAMES_IN_FLIGHT> m_presentFrames{};
	uint32_t m_presentFrameIndex = 0;

	std::mutex m_presentMutex;

	SwapchainDesc m_desc;
};
} // namespace Prism::Render::Vulkan
