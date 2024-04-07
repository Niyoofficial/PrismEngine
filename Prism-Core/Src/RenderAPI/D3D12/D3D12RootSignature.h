#pragma once
#include "Prism-Core/Render/GraphicsPipelineState.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render::D3D12
{
class D3D12RootSignature
{
public:
	explicit D3D12RootSignature(const GraphicsPipelineStateDesc& psoDesc);

	ID3D12RootSignature* GetD3D12RootSignature() const { return m_rootSignature.Get(); }
	int32_t GetParamIndex(const std::wstring& paramName);

private:
	std::unordered_map<std::wstring, int32_t> m_rootParamsIndexMap;

	ComPtr<ID3D12RootSignature> m_rootSignature;
};
}
