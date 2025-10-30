#pragma once
#include "Prism/Render/PipelineState.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render {
class TextureView;
}

struct ID3D12PipelineState;

namespace Prism::Render::D3D12
{
class D3D12PipelineStateCache
{
public:
	ID3D12PipelineState* GetOrCreatePipelineState(const GraphicsPipelineStateDesc& desc, const std::vector<Ref<TextureView>>& rtvs, TextureView* dsv);
	ID3D12PipelineState* GetOrCreatePipelineState(const ComputePipelineStateDesc& desc);

private:
	uint64_t HashPipelineStateDesc(const GraphicsPipelineStateDesc& desc, const std::vector<Ref<TextureView>>& rtvs, TextureView* dsv) const;
	uint64_t HashPipelineStateDesc(const ComputePipelineStateDesc& desc) const;

private:
	std::unordered_map<uint64_t, ComPtr<ID3D12PipelineState>> m_graphicsPipelineStates;
	std::unordered_map<uint64_t, ComPtr<ID3D12PipelineState>> m_computePipelineStates;
};
}
