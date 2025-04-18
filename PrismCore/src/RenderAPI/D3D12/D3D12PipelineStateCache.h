#pragma once
#include "Prism/Render/PipelineState.h"
#include "RenderAPI/D3D12/D3D12Base.h"
#include "xxhash.h"

struct ID3D12PipelineState;

namespace Prism::Render::D3D12
{
class D3D12PipelineStateCache
{
public:
	ID3D12PipelineState* GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc);
	ID3D12PipelineState* GetOrCreatePipelineState(const ComputePipelineStateDesc& desc);

private:
	template<typename T>
	XXH64_hash_t HashPipelineStateDesc(const T& desc) const;

private:
	std::unordered_map<XXH64_hash_t, ComPtr<ID3D12PipelineState>> m_graphicsPipelineStates;
	std::unordered_map<XXH64_hash_t, ComPtr<ID3D12PipelineState>> m_computePipelineStates;
};
}
