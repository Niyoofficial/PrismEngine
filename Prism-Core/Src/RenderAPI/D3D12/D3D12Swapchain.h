#pragma once

#include "D3D12Base.h"

#include "Prism-Core/Render/Swapchain.h"
#include "Prism-Core/Render/Texture.h"

namespace Prism::Core
{
class Window;
}

namespace Prism::Render::D3D12
{
class D3D12Swapchain : public Swapchain
{
public:
	D3D12Swapchain(Core::Window* window, SwapchainDesc swapchainDesc);

	virtual void Present() override;
	virtual void Resize() override;

	virtual TextureView* GetBackBufferRTV(int32_t index) const override;
	virtual TextureView* GetCurrentBackBufferRTV() const override;
	virtual int32_t GetCurrentBackBufferIndex() const override;

private:
	Core::Window* m_owningWindow = nullptr;

	ComPtr<IDXGISwapChain3> m_swapchain;
	std::vector<std::unique_ptr<Texture>> m_backBuffers;
	std::vector<std::unique_ptr<TextureView>> m_backBufferRTVs;
};
}
