#include "D3D12PipelineStateCache.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

namespace Prism::Render::D3D12
{
ID3D12PipelineState* D3D12PipelineStateCache::GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc, const std::vector<Ref<TextureView>>& rtvs, TextureView* dsv)
{
	uint64_t hash = HashPipelineStateDesc(desc, rtvs, dsv);

	if (m_graphicsPipelineStates.contains(hash))
		return m_graphicsPipelineStates.at(hash).Get();

	auto vsOutput = D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetOrCreateShader(desc.vs);
	auto psOutput = D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetOrCreateShader(desc.ps);

	auto d3d12PipelineStateDesc = GetD3D12PipelineStateDesc(desc, vsOutput, psOutput, rtvs, dsv);
	ComPtr<ID3D12PipelineState> pipelineState;
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateGraphicsPipelineState(
		&d3d12PipelineStateDesc.psoDesc, IID_PPV_ARGS(&pipelineState)));
	PE_ASSERT_HR(pipelineState->SetName(L"Graphics PSO"));

	PE_D3D12_LOG(Trace, "Created new graphics PSO hash: {}\n\tvs: {} entry name: {}\n\tps: {} entry name: {}",
		hash, desc.vs.filepath, desc.vs.entryName, desc.ps.filepath, desc.ps.entryName);

	m_graphicsPipelineStates[hash] = pipelineState;

	return pipelineState.Get();
}

ID3D12PipelineState* D3D12PipelineStateCache::GetOrCreatePipelineState(const ComputePipelineStateDesc& desc)
{
	auto csOutput = D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetOrCreateShader(desc.cs);

	uint64_t hash = HashPipelineStateDesc(desc);

	if (m_computePipelineStates.contains(hash))
		return m_computePipelineStates.at(hash).Get();

	auto d3d12PipelineStateDesc = GetD3D12PipelineStateDesc(desc, csOutput);
	ComPtr<ID3D12PipelineState> pipelineState;
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateComputePipelineState(
		&d3d12PipelineStateDesc, IID_PPV_ARGS(&pipelineState)));
	PE_ASSERT_HR(pipelineState->SetName(L"Compute PSO"));

	PE_D3D12_LOG(Trace, "Created new compute PSO hash: {}\n\tcs: {} entry name: {}",
		hash, desc.cs.filepath, desc.cs.entryName);

	m_computePipelineStates[hash] = pipelineState;

	return pipelineState.Get();
}

uint64_t D3D12PipelineStateCache::HashPipelineStateDesc(const GraphicsPipelineStateDesc& desc, const std::vector<Ref<TextureView>>& rtvs, TextureView* dsv) const
{
	uint64_t hash = std::hash<GraphicsPipelineStateDesc>()(desc);

	hash ^= D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetShaderCodeHash(desc.vs);
	hash ^= D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetShaderCodeHash(desc.ps);

	for (auto& rtv : rtvs)
	{
		if (rtv)
		{
			auto viewDesc = rtv->GetViewDesc();
			hash ^= std::hash<TextureViewDesc>()(viewDesc);
		}
	}
	if (dsv)
	{
		auto viewDesc = dsv->GetViewDesc();
		hash ^= std::hash<TextureViewDesc>()(viewDesc);
	}

	return hash;
}

uint64_t D3D12PipelineStateCache::HashPipelineStateDesc(const ComputePipelineStateDesc& desc) const
{
	return D3D12RenderDevice::Get().GetD3D12ShaderCompiler()->GetShaderCodeHash(desc.cs);
}
}
