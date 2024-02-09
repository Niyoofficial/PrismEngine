#include "pcpch.h"
#include "D3D12RootSignatureCache.h"

#include "RenderAPI/D3D12/D3D12RootSignature.h"

namespace Prism::D3D12
{
D3D12RootSignature* D3D12RootSignatureCache::GetOrCreateRootSignature(const Render::GraphicsPipelineStateDesc& psoDesc)
{
	Render::PipelineStateHash hash(psoDesc);

	auto findIt = m_rootSignatures.find(hash);
	if (findIt != m_rootSignatures.end())
		return &findIt->second;

	auto [it, success] = m_rootSignatures.emplace(hash, D3D12RootSignature(psoDesc));
	return &it->second;
}
}
