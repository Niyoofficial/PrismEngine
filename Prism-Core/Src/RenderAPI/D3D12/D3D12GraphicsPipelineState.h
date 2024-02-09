#pragma once
#include "Prism-Core/Render/GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::D3D12
{
class D3D12GraphicsPipelineState : public Render::GraphicsPipelineState
{
public:
	explicit D3D12GraphicsPipelineState(const Render::GraphicsPipelineStateDesc& desc);

	ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState.Get(); }

private:
	ComPtr<ID3D12PipelineState> m_d3d12PipelineState;
};
}
