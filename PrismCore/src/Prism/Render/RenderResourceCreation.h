#pragma once
#include "Prism/Base/Window.h"
#include "Prism/Render/RenderDevice.h"

namespace Prism::Render::Private
{
void CreateRenderDevice(RenderDeviceParams params);
Ref<RenderCommandList> CreateRenderCommandList();
Ref<Swapchain> CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc);
ShaderCompiler* CreateShaderCompiler();
}
