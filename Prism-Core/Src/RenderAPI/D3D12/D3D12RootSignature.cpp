#include "pcpch.h"
#include "D3D12RootSignature.h"

#include "RenderAPI/D3D12/D3D12RenderAPI.h"
#include "RenderAPI/D3D12/D3D12ShaderImpl.h"

namespace Prism::Render::D3D12
{
D3D12RootSignature::D3D12RootSignature(const GraphicsPipelineStateDesc& psoDesc)
{
	ID3D12ShaderReflection* vsReflection = static_cast<D3D12Shader*>(psoDesc.vs)->GetCompilerOutput().reflection.Get();

	D3D12_SHADER_DESC vsShaderDesc;
	PE_ASSERT_HR(vsReflection->GetDesc(&vsShaderDesc));

	std::vector<D3D12_ROOT_PARAMETER> rootParams;

	// Iterate over all the resources bound in the shader like cbuffers, textures and samplers
	for (UINT resIndex = 0; resIndex < vsShaderDesc.BoundResources; ++resIndex)
	{
		D3D12_SHADER_INPUT_BIND_DESC bindingDesc = {};
		PE_ASSERT_HR(vsReflection->GetResourceBindingDesc(resIndex, &bindingDesc));

		// This allows us to refer to the params by name, not an index
		m_rootParamsIndexMap[bindingDesc.Name] = (int32_t)rootParams.size();

		if (bindingDesc.Type == D3D_SIT_CBUFFER)
		{
			auto* constBufferRef = vsReflection->GetConstantBufferByIndex(resIndex);
			D3D12_SHADER_BUFFER_DESC constBufferDesc = {};
			PE_ASSERT_HR(constBufferRef->GetDesc(&constBufferDesc));

			D3D12_ROOT_PARAMETER rootParam = {
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
				.Descriptor = {
					.ShaderRegister = bindingDesc.BindPoint,
					.RegisterSpace = bindingDesc.Space,
				},
				//.ShaderVisibility = 
			};

			rootParams.push_back(rootParam);
		}
	}


	// Create the root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc((UINT)rootParams.size(), rootParams.data());

	ComPtr<ID3DBlob> serializedRootSig;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT result = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&serializedRootSig,
		&errorBlob);

	if (FAILED(result))
	{
		PE_RENDER_LOG(Error, "Failed to serialize Root Signature");

		if (errorBlob.Get() && errorBlob->GetBufferPointer())
			PE_RENDER_LOG(Error, "{}", static_cast<char*>(errorBlob->GetBufferPointer()));

		PE_ASSERT(false);
	}

	PE_ASSERT_HR(D3D12RenderAPI::Get()->GetD3DDevice()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}
}
