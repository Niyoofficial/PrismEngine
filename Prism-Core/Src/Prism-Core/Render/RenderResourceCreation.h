#pragma once
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/Buffer.h"
#include "Prism-Core/Render/BufferView.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/GraphicsPipelineState.h"
#include "Prism-Core/Render/RenderDevice.h"

namespace Prism::Render::Private
{
void CreateRenderDevice(RenderDeviceParams params);
RenderContext* CreateRenderContext();
Swapchain* CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc);
Shader* CreateShader(const ShaderCreateInfo& createInfo);
GraphicsPipelineState* CreatePipelineState(const GraphicsPipelineStateDesc& desc);
Buffer* CreateBuffer(const BufferDesc& desc, const std::vector<BufferInitData>& initData);
BufferView* CreateBufferView(const BufferViewDesc& desc, class Buffer* buffer);
Texture* CreateTexture(const TextureDesc& desc, const std::vector<TextureInitData>& initData);
TextureView* CreateTextureView(const TextureViewDesc& desc, Texture* texture);
}
