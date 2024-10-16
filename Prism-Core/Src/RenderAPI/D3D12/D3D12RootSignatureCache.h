#pragma once
#include "Prism-Core/Render/PipelineState.h"
#include "RenderAPI/D3D12/D3D12RootSignature.h"

namespace Prism::Render::D3D12
{
class D3D12RootSignatureCache
{
public:
	using PipelineStateDesc = std::variant<GraphicsPipelineStateDesc, ComputePipelineStateDesc>;

public:
	D3D12RootSignature* GetOrCreateRootSignature(const GraphicsPipelineStateDesc& psoDesc);
	D3D12RootSignature* GetOrCreateRootSignature(const ComputePipelineStateDesc& psoDesc);

private:
	std::unordered_map<PipelineStateDesc, D3D12RootSignature> m_rootSignatures;
};
}
