﻿#include "pcpch.h"
#include "D3D12GraphicsPipelineState.h"

#include "RenderAPI/D3D12/D3D12RenderAPI.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

namespace Prism::D3D12
{
D3D12GraphicsPipelineState::D3D12GraphicsPipelineState(const Render::GraphicsPipelineStateDesc& desc)
	: GraphicsPipelineState(desc)
{
	auto d3d12PipelineStateDesc = GetD3D12PipelineStateDesc(desc);
	PE_ASSERT_HR(D3D12RenderAPI::Get()->GetD3DDevice()->CreateGraphicsPipelineState(
		&d3d12PipelineStateDesc, IID_PPV_ARGS(&m_d3d12PipelineState)));
}
}