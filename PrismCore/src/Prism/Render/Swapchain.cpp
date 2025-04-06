#include "pcpch.h"
#include "Swapchain.h"

#include "Prism/Render/RenderResourceCreation.h"

namespace Prism::Render
{
Swapchain* Swapchain::Create(Core::Window* window, SwapchainDesc swapchainDesc)
{
	return Private::CreateSwapchain(window, swapchainDesc);
}
}
