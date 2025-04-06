#pragma once
#include "Prism/Render/PipelineState.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render::D3D12
{
class D3D12GraphicsPipelineState : public GraphicsPipelineState
{
public:
	explicit D3D12GraphicsPipelineState(const GraphicsPipelineStateDesc& desc);

	ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState.Get(); }

private:
	ComPtr<ID3D12PipelineState> m_d3d12PipelineState;
};

class D3D12ComputePipelineState : public ComputePipelineState
{
public:
	explicit D3D12ComputePipelineState(const ComputePipelineStateDesc& desc);

	ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState.Get(); }

private:
	ComPtr<ID3D12PipelineState> m_d3d12PipelineState;
};
}
