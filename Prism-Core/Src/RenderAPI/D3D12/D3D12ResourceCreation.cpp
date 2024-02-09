#include "pcpch.h"

#include "Prism-Core/Render/PipelineStateCache.h"
#include "Prism-Core/Render/Swapchain.h"
#include "RenderAPI/D3D12/D3D12Swapchain.h"

#include "Prism-Core/Render/RenderAPI.h"
#include "Prism-Core/Render/Shader.h"
#include "Prism-Core/Render/ShaderCache.h"
#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12RenderAPI.h"
#include "RenderAPI/D3D12/D3D12ShaderImpl.h"

namespace Prism::Render
{
Swapchain* Swapchain::Create(Core::Window* window, SwapchainDesc swapchainDesc)
{
	return new D3D12::D3D12Swapchain(window, swapchainDesc);
}

RenderAPI* RenderAPI::Create()
{
	return new D3D12::D3D12RenderAPI;
}

Shader* ShaderCache::CreateShader(const ShaderCreateInfo& createInfo)
{
	return new D3D12::D3D12Shader(createInfo);
}

GraphicsPipelineState* PipelineStateCache::CreatePipelineState(const GraphicsPipelineStateDesc& desc)
{
	return new D3D12::D3D12GraphicsPipelineState(desc);
}
}
