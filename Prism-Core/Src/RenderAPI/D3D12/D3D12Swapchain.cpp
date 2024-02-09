#include "pcpch.h"
#include "D3D12Swapchain.h"

#include "Prism-Core/Base/Window.h"
#include "RenderAPI/D3D12/D3D12Base.h"

#include "RenderAPI/D3D12/D3D12RenderAPI.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::D3D12
{
D3D12Swapchain::D3D12Swapchain(Core::Window* window, Render::SwapchainDesc swapchainDesc)
	: m_owningWindow(window)
{
	// TODO: add viewport support https://www.reddit.com/r/vulkan/comments/vxxu1k/using_multiple_swapchains/
	DXGI_SWAP_CHAIN_DESC dxgiSwapchainDesc = {};
	dxgiSwapchainDesc.BufferDesc.Width = m_owningWindow->GetSize().x;
	dxgiSwapchainDesc.BufferDesc.Height = m_owningWindow->GetSize().y;
	dxgiSwapchainDesc.BufferDesc.RefreshRate = GetDXGIRational(swapchainDesc.refreshRate.numerator,
															   swapchainDesc.refreshRate.denominator);
	dxgiSwapchainDesc.BufferDesc.Format = GetDXGIFormat(swapchainDesc.format);
	dxgiSwapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapchainDesc.SampleDesc = GetDXGISampleDesc(swapchainDesc.sampleDesc);
	dxgiSwapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapchainDesc.BufferCount = swapchainDesc.bufferCount;

	HWND hwnd = std::any_cast<HWND>(m_owningWindow->GetPlatformNativeWindow());
	dxgiSwapchainDesc.OutputWindow = hwnd;

	dxgiSwapchainDesc.Windowed = !m_owningWindow->GetIsFullscreen();
	dxgiSwapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGIFactory* factory = D3D12RenderAPI::Get()->GetDXGIFactory();
	ID3D12CommandQueue* commandQueue = D3D12RenderAPI::Get()->GetCommandQueue();
	PE_ASSERT_HR(factory->CreateSwapChain(commandQueue, &dxgiSwapchainDesc, &m_swapchain));

	PE_ASSERT_HR(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES));
}

void D3D12Swapchain::Resize()
{
	//TODO: implement Resize
	PE_ASSERT_NO_ENTRY();
}
}
