#include "pcpch.h"
#include "D3D12RootSignature.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12ShaderImpl.h"

namespace Prism::Render::D3D12
{
D3D12RootSignature::D3D12RootSignature(const GraphicsPipelineStateDesc& psoDesc)
{
	std::array shaders = {
		static_cast<D3D12Shader*>(psoDesc.vs),
		static_cast<D3D12Shader*>(psoDesc.ps)
	};

	std::vector<D3D12_ROOT_PARAMETER> rootParams;

	for (D3D12Shader* shader : shaders)
	{
		ID3D12ShaderReflection* reflection = shader->GetCompilerOutput().reflection.Get();

		D3D12_SHADER_DESC shaderDesc;
		PE_ASSERT_HR(reflection->GetDesc(&shaderDesc));

		// Iterate over all the resources bound in the shader like cbuffers, textures and samplers
		for (UINT resIndex = 0; resIndex < shaderDesc.BoundResources; ++resIndex)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindingDesc = {};
			PE_ASSERT_HR(reflection->GetResourceBindingDesc(resIndex, &bindingDesc));

			// This allows us to refer to the params by name, not an index
			m_rootParamsIndexMap[StringToWString(bindingDesc.Name)] = (int32_t)rootParams.size();

			if (bindingDesc.Type == D3D_SIT_CBUFFER)
			{
				auto* constBufferRef = reflection->GetConstantBufferByIndex(resIndex);
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
	}


	// Create the root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc((UINT)rootParams.size(), rootParams.data(), 0, nullptr,
											D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

int32_t D3D12RootSignature::GetParamIndex(const std::wstring& paramName)
{
	auto it = m_rootParamsIndexMap.find(paramName);
	if (it == m_rootParamsIndexMap.end())
		return -1;
	return it->second;
}
}
