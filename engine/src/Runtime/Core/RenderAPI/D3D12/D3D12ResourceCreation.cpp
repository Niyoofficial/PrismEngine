#include "pcpch.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12Swapchain.h"
#include "RenderAPI/D3D12/D3D12RenderCommandList.h"


namespace Prism::Render::Private
{
void CreateRenderDevice(RenderDeviceParams params)
{
	StaticPointerSingleton<RenderDevice>::Create<D3D12::D3D12RenderDevice>(params);
}

Ref<RenderCommandList> CreateRenderCommandList()
{
	return Ref<D3D12::D3D12RenderCommandList>::Create();
}

Ref<Swapchain> CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc)
{
	return Ref<D3D12::D3D12Swapchain>::Create(window, swapchainDesc);
}

ShaderCompiler* CreateShaderCompiler()
{
	return new D3D12::D3D12ShaderCompiler();
}
}
