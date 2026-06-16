#include "pcpch.h"
#include "Swapchain.h"

#include "Prism/Render/RenderResourceCreation.h"

namespace Prism::Render
{
Ref<Swapchain> Swapchain::Create(Core::Window* window, SwapchainDesc swapchainDesc)
{
	return Private::CreateSwapchain(window, swapchainDesc);
}

Swapchain::Swapchain(SwapchainDesc desc)
	: m_desc(desc)
{
}

void Swapchain::AdvanceBackBufferIndex()
{
	m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % m_desc.bufferCount;
}
}
