#pragma once
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/Buffer.h"
#include "Prism-Core/Render/BufferView.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/PipelineState.h"
#include "Prism-Core/Render/RenderDevice.h"

namespace Prism::Render::Private
{
void CreateRenderDevice(RenderDeviceParams params);
RenderCommandList* CreateRenderCommandList();
Swapchain* CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc);
Shader* CreateShader(const ShaderCreateInfo& createInfo);
GraphicsPipelineState* CreatePipelineState(const GraphicsPipelineStateDesc& desc);
ComputePipelineState* CreatePipelineState(const ComputePipelineStateDesc& desc);
Buffer* CreateBuffer(const BufferDesc& desc);
BufferView* CreateBufferView(const BufferViewDesc& desc, class Buffer* buffer);
Texture* CreateTexture(const TextureDesc& desc, BarrierLayout initLayout);
Texture* CreateTexture(std::wstring filepath, bool loadAsCubemap);
TextureView* CreateTextureView(const TextureViewDesc& desc, Texture* texture);
}
