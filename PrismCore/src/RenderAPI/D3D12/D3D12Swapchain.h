#pragma once

#include "D3D12Base.h"

#include "Prism/Render/Swapchain.h"
#include "Prism/Render/Texture.h"

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

	virtual SwapchainDesc GetSwapchainDesc() const override;

	virtual TextureView* GetBackBufferRTV(int32_t index) const override;
	virtual TextureView* GetCurrentBackBufferRTV() const override;
	virtual int32_t GetCurrentBackBufferIndex() const override;

private:
	void GatherBackbuffersAndCreateRTVs();

private:
	WeakRef<Core::Window> m_owningWindow;
	SwapchainDesc m_swapchainDesc;

	ComPtr<IDXGISwapChain3> m_swapchain;
	std::vector<Ref<Texture>> m_backBuffers;
	std::vector<Ref<TextureView>> m_backBufferRTVs;
};
}
