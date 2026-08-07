#include "VulkanRenderCommandList.h"
#include "VulkanRenderDevice.h"
#include "VulkanShaderCompiler.h"
#include "VulkanSwapchain.h"

namespace Prism::Render::Private
{
void CreateRenderDevice(RenderDeviceParams params)
{
	StaticPointerSingleton<RenderDevice>::Create<Vulkan::VulkanRenderDevice>(params);
}

Ref<RenderCommandList> CreateRenderCommandList() { return Ref<Vulkan::VulkanRenderCommandList>::Create(); }

Ref<Swapchain> CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc)
{
	return Ref<Vulkan::VulkanSwapchain>::Create(window, swapchainDesc);
}

ShaderCompiler* CreateShaderCompiler() { return new Vulkan::VulkanShaderCompiler(); }
} // namespace Prism::Render::Private
