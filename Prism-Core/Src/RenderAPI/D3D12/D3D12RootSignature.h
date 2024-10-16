#pragma once
#include "Prism-Core/Render/PipelineState.h"
#include "RenderAPI/D3D12/D3D12Base.h"
#include <d3d12shader.h>

namespace Prism::Render::D3D12
{
class D3D12RootSignature
{
public:
	explicit D3D12RootSignature(const GraphicsPipelineStateDesc& psoDesc);
	explicit D3D12RootSignature(const ComputePipelineStateDesc& psoDesc);

	ID3D12RootSignature* GetD3D12RootSignature() const { return m_rootSignature.Get(); }
	int32_t GetParamIndex(const std::wstring& paramName);
	const std::unordered_map<std::wstring, int32_t>& GetRootParamsIndexMap() const { return m_rootParamsIndexMap; }
	D3D12_SHADER_INPUT_BIND_DESC GetReflectionForRootParam(int32_t paramIndex);

private:
	std::unordered_map<std::wstring, int32_t> m_rootParamsIndexMap;
	std::unordered_map<int32_t, D3D12_SHADER_INPUT_BIND_DESC> m_rootParamsReflection;

	ComPtr<ID3D12RootSignature> m_rootSignature;
};
}
