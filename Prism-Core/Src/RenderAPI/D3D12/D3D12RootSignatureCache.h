#pragma once
#include "Prism-Core/Render/GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12RootSignature.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::D3D12
{
class D3D12RootSignatureCache
{
public:
	D3D12RootSignature* GetOrCreateRootSignature(const Render::GraphicsPipelineStateDesc& psoDesc);

private:
	std::unordered_map<Render::PipelineStateHash, D3D12RootSignature> m_rootSignatures;
};
}
