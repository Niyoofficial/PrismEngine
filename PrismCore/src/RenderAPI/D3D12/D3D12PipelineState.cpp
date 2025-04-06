#include "pcpch.h"
#include "D3D12PipelineState.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
D3D12GraphicsPipelineState::D3D12GraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
	: GraphicsPipelineState(desc)
{
	auto d3d12PipelineStateDesc = GetD3D12PipelineStateDesc(desc);
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateGraphicsPipelineState(
		&d3d12PipelineStateDesc.psoDesc, IID_PPV_ARGS(&m_d3d12PipelineState)));
}

D3D12ComputePipelineState::D3D12ComputePipelineState(const ComputePipelineStateDesc& desc)
	: ComputePipelineState(desc)
{
	auto d3d12PipelineStateDesc = GetD3D12PipelineStateDesc(desc);
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateComputePipelineState(
		&d3d12PipelineStateDesc, IID_PPV_ARGS(&m_d3d12PipelineState)));
}
}
