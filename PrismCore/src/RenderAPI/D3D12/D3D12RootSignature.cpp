#include "pcpch.h"
#include "D3D12RootSignature.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12ShaderImpl.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
D3D12RootSignature::D3D12RootSignature(PipelineStateType type)
	: m_rootSignatureType(type)
{
	// TODO
	const static CD3DX12_STATIC_SAMPLER_DESC1 s_pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const static CD3DX12_STATIC_SAMPLER_DESC1 s_pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const static CD3DX12_STATIC_SAMPLER_DESC1 s_linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const static CD3DX12_STATIC_SAMPLER_DESC1 s_linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const static CD3DX12_STATIC_SAMPLER_DESC1 s_anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressW
		0.0f, // mipLODBias
		8); // maxAnisotropy

	const static CD3DX12_STATIC_SAMPLER_DESC1 s_anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressW
		0.0f, // mipLODBias
		8); // maxAnisotropy

	const static CD3DX12_STATIC_SAMPLER_DESC1 s_shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER, // addressW
		0.0f, // mipLODBias
		16, // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL, // comparision function
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK); // border color

	std::array samplers = {
		s_pointWrap, s_pointClamp,
		s_linearWrap, s_linearClamp,
		s_anisotropicWrap, s_anisotropicClamp,
		s_shadow
	};

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParams;

	if (type == PipelineStateType::Graphics)
	{
		// TODO: Figure out a way to have separate parameters for each shader in the same PSO
		/*std::array<D3D12_ROOT_PARAMETER1, (size_t)D3D12_SHADER_VISIBILITY_MESH - D3D12_SHADER_VISIBILITY_VERTEX + 1> rootParams;
		for (std::underlying_type_t<D3D12_SHADER_VISIBILITY> i = D3D12_SHADER_VISIBILITY_VERTEX; i <= D3D12_SHADER_VISIBILITY_MESH; ++i)
		{
			CD3DX12_ROOT_PARAMETER1 rootParam;
			rootParam.InitAsConstants(64, 0, 0, (D3D12_SHADER_VISIBILITY)i);
			rootParams[i - D3D12_SHADER_VISIBILITY_VERTEX] = rootParam;
		}

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC::Init_1_2(rootSigDesc,
			(UINT)rootParams.size(), rootParams.data(), (UINT)samplers.size(), samplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);*/

		CD3DX12_ROOT_PARAMETER1 rootParam;
		rootParam.InitAsConstants(64, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

		rootParams.push_back(rootParam);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC::Init_1_2(rootSigDesc,
			1, rootParams.data(), (UINT)samplers.size(), samplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);
	}
	else
	{
		CD3DX12_ROOT_PARAMETER1 rootParam;
		rootParam.InitAsConstants(64, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

		rootParams.push_back(rootParam);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC::Init_1_2(rootSigDesc,
			1, rootParams.data(), (UINT)samplers.size(), samplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);
	}

	ComPtr<ID3DBlob> serializedRootSig;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT result = D3D12SerializeVersionedRootSignature(
		&rootSigDesc,
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

int32_t D3D12RootSignature::GetRootParamIndexForShader(ShaderType shaderType)
{
	// TODO
	/*D3D12_SHADER_VISIBILITY visibility = GetD3D12ShaderVisibility(shaderType);
	return visibility;*/
	return 0;
}
}
