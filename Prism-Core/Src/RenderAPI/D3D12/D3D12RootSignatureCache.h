#pragma once
#include "Prism-Core/Render/GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12RootSignature.h"

namespace Prism::Render::D3D12
{
class D3D12RootSignatureCache
{
public:
	D3D12RootSignature* GetOrCreateRootSignature(const GraphicsPipelineStateDesc& psoDesc);

private:
	std::unordered_map<PipelineStateHash, D3D12RootSignature> m_rootSignatures;
};
}
