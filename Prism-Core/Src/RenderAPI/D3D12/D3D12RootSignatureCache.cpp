#include "pcpch.h"
#include "D3D12RootSignatureCache.h"

#include "RenderAPI/D3D12/D3D12RootSignature.h"

namespace Prism::Render::D3D12
{
D3D12RootSignature* D3D12RootSignatureCache::GetOrCreateRootSignature(const GraphicsPipelineStateDesc& psoDesc)
{
	auto findIt = m_rootSignatures.find(psoDesc);
	if (findIt != m_rootSignatures.end())
		return &findIt->second;

	auto [it, success] = m_rootSignatures.emplace(psoDesc, D3D12RootSignature(psoDesc));
	return &it->second;
}
}
