#include "pcpch.h"

#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12RenderContext.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12ShaderImpl.h"
#include "RenderAPI/D3D12/D3D12Swapchain.h"
#include "Prism-Core/Render/Buffer.h"
#include "Prism-Core/Render/BufferView.h"
#include "RenderAPI/D3D12/D3D12Buffer.h"
#include "RenderAPI/D3D12/D3D12BufferView.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"


namespace Prism::Render::Private
{
void CreateRenderDevice(RenderDeviceParams params)
{
	StaticPointerSingleton<RenderDevice>::Create<D3D12::D3D12RenderDevice>(params);
}

RenderContext* CreateRenderContext()
{
	return new D3D12::D3D12RenderContext;
}

Swapchain* CreateSwapchain(Core::Window* window, SwapchainDesc swapchainDesc)
{
	return new D3D12::D3D12Swapchain(window, swapchainDesc);
}

Shader* CreateShader(const ShaderCreateInfo& createInfo)
{
	return new D3D12::D3D12Shader(createInfo);
}

GraphicsPipelineState* CreatePipelineState(const GraphicsPipelineStateDesc& desc)
{
	return new D3D12::D3D12GraphicsPipelineState(desc);
}

Buffer* CreateBuffer(const BufferDesc& desc, const std::vector<BufferInitData>& initData)
{
	return new D3D12::D3D12Buffer(desc, initData);
}

BufferView* CreateBufferView(const BufferViewDesc& desc, class Buffer* buffer)
{
	return new D3D12::D3D12BufferView(desc, buffer);
}

Texture* CreateTexture(const TextureDesc& desc, const std::vector<TextureInitData>& initData)
{
	return new D3D12::D3D12Texture(desc, initData);
}

TextureView* CreateTextureView(const TextureViewDesc& desc, Texture* texture)
{
	return new D3D12::D3D12TextureView(desc, texture);
}
}
