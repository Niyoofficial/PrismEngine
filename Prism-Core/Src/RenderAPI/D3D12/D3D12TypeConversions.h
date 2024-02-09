#pragma once
#include "Prism-Core/Render/GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::D3D12
{
DXGI_FORMAT GetDXGIFormat(Render::TextureFormat format);
DXGI_SAMPLE_DESC GetDXGISampleDesc(Render::SampleDesc sampleDesc);
DXGI_RATIONAL GetDXGIRational(int32_t numerator, int32_t denominator);

D3D12_GRAPHICS_PIPELINE_STATE_DESC GetD3D12PipelineStateDesc(const Render::GraphicsPipelineStateDesc& desc);
CD3DX12_SHADER_BYTECODE GetD3D12ShaderBytecode(Render::Shader* shader);

D3D12_FILL_MODE GetD3D12FillMode(Render::FillMode fillMode);
D3D12_CULL_MODE GetD3D12CullMode(Render::CullMode cullMode);
D3D12_COMPARISON_FUNC GetD3D12ComparisionFunc(Render::ComparisionFunction comparisionFunction);
D3D12_DEPTH_STENCILOP_DESC GetD3D12DepthStencilOpDesc(Render::DepthStencilOperationDesc depthStencilOperationDesc);
D3D12_STENCIL_OP GetD3D12StencilOp(Render::StencilOperation stencilOperation);
D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType(Render::TopologyType topologyType);
D3D12_RENDER_TARGET_BLEND_DESC GetD3D12RenderTargetBlendDesc(Render::RenderTargetBlendDesc renderTargetBlendDesc);
D3D12_BLEND GetD3D12Blend(Render::BlendFactor blendFactor);
D3D12_BLEND_OP GetD3D12BlendOp(Render::BlendOperation blendOperation);
D3D12_LOGIC_OP GetD3D12LogicOp(Render::LogicOperation logicOperation);
std::vector<D3D12_INPUT_ELEMENT_DESC> GetD3D12InputLayoutElements(const std::vector<Render::LayoutElement>& layoutElements);
D3D12_INPUT_CLASSIFICATION GetD3D12InputClassification(Render::LayoutElementFrequency frequency);
DXGI_FORMAT GetD3D12InputElementDescFormat(Render::LayoutValueType valueType, int32_t componentsNum, bool isNormalized);
}
