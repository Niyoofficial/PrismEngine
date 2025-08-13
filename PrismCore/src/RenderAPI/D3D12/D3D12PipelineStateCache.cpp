#include "D3D12PipelineStateCache.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

namespace Prism::Render::D3D12
{
ID3D12PipelineState* D3D12PipelineStateCache::GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc, std::vector<Ref<TextureView>> rtvs, TextureView* dsv)
{
	auto vsOutput = D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetOrCreateShader(desc.vs);
	auto psOutput = D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetOrCreateShader(desc.ps);

	XXH64_hash_t hash = HashPipelineStateDesc(desc, rtvs, dsv);

	if (m_graphicsPipelineStates.contains(hash))
		return m_graphicsPipelineStates.at(hash).Get();

	auto d3d12PipelineStateDesc = GetD3D12PipelineStateDesc(desc, vsOutput, psOutput, rtvs, dsv);
	ComPtr<ID3D12PipelineState> pipelineState;
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateGraphicsPipelineState(
		&d3d12PipelineStateDesc.psoDesc, IID_PPV_ARGS(&pipelineState)));

	PE_ASSERT_HR(pipelineState->SetName(L"Graphics PSO"));
	m_graphicsPipelineStates[hash] = pipelineState;

	return pipelineState.Get();
}

ID3D12PipelineState* D3D12PipelineStateCache::GetOrCreatePipelineState(const ComputePipelineStateDesc& desc)
{
	auto csOutput = D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetOrCreateShader(desc.cs);

	XXH64_hash_t hash = HashPipelineStateDesc(desc);

	if (m_computePipelineStates.contains(hash))
		return m_computePipelineStates.at(hash).Get();

	auto d3d12PipelineStateDesc = GetD3D12PipelineStateDesc(desc, csOutput);
	ComPtr<ID3D12PipelineState> pipelineState;
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateComputePipelineState(
		&d3d12PipelineStateDesc, IID_PPV_ARGS(&pipelineState)));

	PE_ASSERT_HR(pipelineState->SetName(L"Compute PSO"));
	m_computePipelineStates[hash] = pipelineState;

	return pipelineState.Get();
}

XXH64_hash_t D3D12PipelineStateCache::HashPipelineStateDesc(const GraphicsPipelineStateDesc& desc, std::vector<Ref<TextureView>> rtvs, TextureView* dsv) const
{
	auto hash = XXH3_64bits(&desc, sizeof(desc));

	hash ^= D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetShaderCodeHash(desc.vs);
	hash ^= D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetShaderCodeHash(desc.ps);

	for (auto& rtv : rtvs)
	{
		if (rtv)
		{
			auto viewDesc = rtv->GetViewDesc();
			hash ^= XXH3_64bits(&viewDesc, sizeof(viewDesc));
		}
	}
	if (dsv)
	{
		auto viewDesc = dsv->GetViewDesc();
		hash ^= XXH3_64bits(&viewDesc, sizeof(viewDesc));
	}

	return hash;
}

XXH64_hash_t D3D12PipelineStateCache::HashPipelineStateDesc(const ComputePipelineStateDesc& desc) const
{
	auto hash = XXH3_64bits(&desc, sizeof(desc));

	hash ^= D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetShaderCodeHash(desc.cs);

	return hash;
}
}
