#pragma once
#include "Prism/Render/PipelineState.h"
#include "RenderAPI/D3D12/D3D12Base.h"
#include "xxhash.h"

namespace Prism::Render {
class TextureView;
}

struct ID3D12PipelineState;

namespace Prism::Render::D3D12
{
class D3D12PipelineStateCache
{
public:
	ID3D12PipelineState* GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc, std::vector<TextureView*> rtvs, TextureView* dsv);
	ID3D12PipelineState* GetOrCreatePipelineState(const ComputePipelineStateDesc& desc);

private:
	XXH64_hash_t HashPipelineStateDesc(const GraphicsPipelineStateDesc& desc, std::vector<TextureView*> rtvs, TextureView* dsv) const;
	XXH64_hash_t HashPipelineStateDesc(const ComputePipelineStateDesc& desc) const;

private:
	std::unordered_map<XXH64_hash_t, ComPtr<ID3D12PipelineState>> m_graphicsPipelineStates;
	std::unordered_map<XXH64_hash_t, ComPtr<ID3D12PipelineState>> m_computePipelineStates;
};
}
