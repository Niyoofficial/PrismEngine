#include "pcpch.h"
#include "D3D12Swapchain.h"

#include "Prism-Core/Base/Window.h"
#include "RenderAPI/D3D12/D3D12Base.h"

#include "RenderAPI/D3D12/D3D12RenderContext.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
D3D12Swapchain::D3D12Swapchain(Core::Window* window, SwapchainDesc swapchainDesc)
	: m_owningWindow(window)
{
	// TODO: add viewport support https://www.reddit.com/r/vulkan/comments/vxxu1k/using_multiple_swapchains/
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapchainDesc = {
		.Width = (UINT)m_owningWindow->GetSize().x,
		.Height = (UINT)m_owningWindow->GetSize().y,
		.Format = GetDXGIFormat(swapchainDesc.format),
		.Stereo = false,
		.SampleDesc = GetDXGISampleDesc(swapchainDesc.sampleDesc),
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = (UINT)swapchainDesc.bufferCount,
		.Scaling = DXGI_SCALING_NONE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {
		.RefreshRate = GetDXGIRational(swapchainDesc.refreshRate.numerator,
									   swapchainDesc.refreshRate.denominator),
		.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
		.Windowed = !m_owningWindow->GetIsFullscreen()
	};

	HWND hwnd = std::any_cast<HWND>(m_owningWindow->GetPlatformNativeWindow());

	IDXGIFactory2* factory = D3D12RenderDevice::Get().GetDXGIFactory();
	ID3D12CommandQueue* commandQueue = D3D12RenderDevice::Get().GetD3D12CommandQueue();
	ComPtr<IDXGISwapChain1> swapchain1;
	PE_ASSERT_HR(factory->CreateSwapChainForHwnd(commandQueue, hwnd, &dxgiSwapchainDesc, &fullscreenDesc, nullptr, &swapchain1));
	PE_ASSERT_HR(swapchain1->QueryInterface(IID_PPV_ARGS(&m_swapchain)));

	PE_ASSERT_HR(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES));

	m_backBuffers.reserve(swapchainDesc.bufferCount);
	for (int32_t i = 0; i < swapchainDesc.bufferCount; ++i)
	{
		ID3D12Resource* buffer = nullptr;
		PE_ASSERT_HR(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&buffer)));

		TextureDesc desc = {
			.textureName = std::wstring(L"Backbuffer_").append(std::to_wstring(i)),
			.optimizedClearValue = RenderTargetClearValue{
				.format = swapchainDesc.format,
				.color = {0.f, 0.f, 0.f, 1.f}
			}
		};
		m_backBuffers.push_back(std::make_unique<D3D12Texture>(buffer, desc));

		TextureViewDesc viewDesc = {
			.type = TextureViewType::RTV,
			.format = swapchainDesc.format,
			.dimension = ResourceDimension::Tex2D
		};
		TextureView* view = m_backBuffers.back()->CreateView(viewDesc);
		m_backBufferRTVs.push_back(std::unique_ptr<TextureView>(view));
	}
}

void D3D12Swapchain::Present()
{
	PE_ASSERT_HR(m_swapchain->Present(0, 0));
}

void D3D12Swapchain::Resize()
{
	//TODO: implement Resize
	PE_ASSERT_NO_ENTRY();
}

TextureView* D3D12Swapchain::GetBackBufferRTV(int32_t index) const
{
	return m_backBufferRTVs[index].get();
}

TextureView* D3D12Swapchain::GetCurrentBackBufferRTV() const
{
	return GetBackBufferRTV(GetCurrentBackBufferIndex());
}

int32_t D3D12Swapchain::GetCurrentBackBufferIndex() const
{
	return m_swapchain->GetCurrentBackBufferIndex();
}
}
