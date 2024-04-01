#pragma once
#include <d3d12shader.h>

#include "Prism-Core/Render/Buffer.h"
#include "Prism-Core/Render/GraphicsPipelineState.h"
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/TextureView.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render::D3D12
{
// Prism -> D3D12/DXGI
struct D3D12InputLayout
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	std::vector<std::string> inputLayoutSemanticNames;
};
struct D3D12GraphicsPipelineStateDesc
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

	// Has to be kept alive until the state is created
	D3D12InputLayout inputLayout;
};

DXGI_FORMAT GetDXGIFormat(TextureFormat format);
DXGI_SAMPLE_DESC GetDXGISampleDesc(SampleDesc sampleDesc);
DXGI_RATIONAL GetDXGIRational(int32_t numerator, int32_t denominator);

D3D12GraphicsPipelineStateDesc GetD3D12PipelineStateDesc(const GraphicsPipelineStateDesc& desc);
CD3DX12_SHADER_BYTECODE GetD3D12ShaderBytecode(Shader* shader);

D3D12_FILL_MODE GetD3D12FillMode(FillMode fillMode);
D3D12_CULL_MODE GetD3D12CullMode(CullMode cullMode);
D3D12_COMPARISON_FUNC GetD3D12ComparisionFunc(ComparisionFunction comparisionFunction);
D3D12_DEPTH_STENCILOP_DESC GetD3D12DepthStencilOpDesc(DepthStencilOperationDesc depthStencilOperationDesc);
D3D12_STENCIL_OP GetD3D12StencilOp(StencilOperation stencilOperation);
D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(TopologyType topologyType);
D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType(TopologyType topologyType);
D3D12_RENDER_TARGET_BLEND_DESC GetD3D12RenderTargetBlendDesc(RenderTargetBlendDesc renderTargetBlendDesc);
D3D12_BLEND GetD3D12Blend(BlendFactor blendFactor);
D3D12_BLEND_OP GetD3D12BlendOp(BlendOperation blendOperation);
D3D12_LOGIC_OP GetD3D12LogicOp(LogicOperation logicOperation);
DXGI_FORMAT GetD3D12InputElementDescFormat(const D3D12_SIGNATURE_PARAMETER_DESC& paramDesc);
D3D12InputLayout GetD3D12InputLayoutFromVertexShader(Shader* vertexShader);

D3D12_RESOURCE_DESC GetD3D12ResourceDesc(const TextureDesc& textureDesc);
D3D12_RESOURCE_STATES GetD3D12ResourceStates(Flags<ResourceStateFlags> states);
D3D12_RESOURCE_DIMENSION GetD3D12ResourceDimension(ResourceDimension dimension);
D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(Flags<BindFlags> bindFlags);
CD3DX12_RESOURCE_BARRIER GetD3D12ResourceBarrier(StateTransitionDesc desc);

D3D12_CLEAR_VALUE GetD3D12ClearValue(ClearValue clearValue);

D3D12_SHADER_RESOURCE_VIEW_DESC GetD3D12ShaderResourceViewDesc(TextureViewDesc desc);
D3D12_RENDER_TARGET_VIEW_DESC GetD3D12RenderTargetViewDesc(TextureViewDesc desc);
D3D12_DEPTH_STENCIL_VIEW_DESC GetD3D12DepthStencilViewDesc(TextureViewDesc desc);

D3D12_VIEWPORT GetD3D12Viewport(Viewport viewport);
D3D12_RECT GetD3D12Rect(Scissor scissor);
D3D12_CLEAR_FLAGS GetD3D12ClearFlags(Flags<ClearFlags> clearFlags);

DXGI_FORMAT GetIndexBufferDXGIFormat(IndexBufferFormat format);


// DXGI -> Prism
TextureFormat GetTextureFormat(DXGI_FORMAT dxgiFormat);
SampleDesc GetSampleDesc(DXGI_SAMPLE_DESC dxgiSampleDesc);

// D3D12 -> Prism
Flags<BindFlags> GetBindFlags(D3D12_RESOURCE_FLAGS resourceFlags);
ResourceDimension GetResourceDimension(D3D12_RESOURCE_DIMENSION d3d12Dimension, bool isCube);
BufferDesc GetBufferDesc(const D3D12_RESOURCE_DESC& d3d12ResDesc,
						 const std::wstring& name = {},
						 ResourceUsage usage = ResourceUsage::Default);
TextureDesc GetTextureDesc(const D3D12_RESOURCE_DESC& d3d12ResDesc,
						   const std::wstring& name = {},
						   ResourceUsage usage = ResourceUsage::Default,
						   ClearValue optimizedClearValue = {},
						   bool isCubeTexture = false);
}
