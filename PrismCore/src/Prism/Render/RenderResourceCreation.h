#pragma once
#include "Prism/Base/Window.h"
#include "Prism/Render/Buffer.h"
#include "Prism/Render/BufferView.h"
#include "Prism/Render/Texture.h"
#include "Prism/Render/RenderDevice.h"

namespace Prism::Render::Private
{
void CreateRenderDevice(RenderDeviceParams params);
RenderCommandList* CreateRenderCommandList();
Swapchain* CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc);
Buffer* CreateBuffer(const BufferDesc& desc);
BufferView* CreateBufferView(const BufferViewDesc& desc, class Buffer* buffer);
Texture* CreateTexture(const TextureDesc& desc, BarrierLayout initLayout);
Texture* CreateTexture(std::wstring filepath, bool loadAsCubemap, bool waitForLoadFinish);
TextureView* CreateTextureView(const TextureViewDesc& desc, Texture* texture);
ShaderCompiler* CreateShaderCompiler();
}
