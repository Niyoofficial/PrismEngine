#pragma once
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/RenderAPI.h"
#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12RenderAPI.h"
#include "RenderAPI/D3D12/D3D12ShaderImpl.h"
#include "RenderAPI/D3D12/D3D12Swapchain.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"

namespace Prism::Render::Private
{
Swapchain* CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc);
RenderAPI* CreateRenderAPI();
Shader* CreateShader(const ShaderCreateInfo& createInfo);
GraphicsPipelineState* CreatePipelineState(const GraphicsPipelineStateDesc& desc);
Texture* CreateTexture(const TextureDesc& desc, const std::vector<TextureInitData>& initData);
TextureView* CreateTextureView(const TextureViewDesc& desc, Texture* texture);
}
