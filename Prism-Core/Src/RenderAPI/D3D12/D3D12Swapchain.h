#pragma once

#include "D3D12Base.h"

#include "Prism-Core/Render/Swapchain.h"

namespace Prism::Core
{
class Window;
}

namespace Prism::D3D12
{
class D3D12Swapchain : public Render::Swapchain
{
public:
	D3D12Swapchain(Core::Window* window, Render::SwapchainDesc swapchainDesc);

	virtual void Resize() override;

private:
	Core::Window* m_owningWindow = nullptr;

	ComPtr<IDXGISwapChain> m_swapchain;
};
}
