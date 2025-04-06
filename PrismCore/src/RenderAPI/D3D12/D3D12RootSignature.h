#pragma once
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render
{
enum class PipelineStateType;
}

namespace Prism::Render::D3D12
{
class D3D12RootSignature : public RefCounted
{
public:
	explicit D3D12RootSignature(PipelineStateType type);

	ID3D12RootSignature* GetD3D12RootSignature() const { return m_rootSignature.Get(); }

	int32_t GetRootParamIndexForShader(ShaderType shaderType);

private:
	ComPtr<ID3D12RootSignature> m_rootSignature;
	PipelineStateType m_rootSignatureType;
};
}
