#pragma once
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/GraphicsPipelineState.h"

namespace Prism::Render::Private
{
void CreateRenderDevice();
class RenderContext* CreateRenderContext();
Swapchain* CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc);
Shader* CreateShader(const ShaderCreateInfo& createInfo);
GraphicsPipelineState* CreatePipelineState(const GraphicsPipelineStateDesc& desc);
Texture* CreateTexture(const TextureDesc& desc, const std::vector<TextureInitData>& initData);
TextureView* CreateTextureView(const TextureViewDesc& desc, Texture* texture);
}
