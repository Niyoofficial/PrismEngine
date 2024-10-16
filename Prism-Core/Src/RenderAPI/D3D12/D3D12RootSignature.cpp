#include "pcpch.h"
#include "D3D12RootSignature.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12ShaderImpl.h"


namespace Prism::Render::D3D12
{
struct RootSignatureInfo
{
	ID3D12RootSignature* rootSignature = nullptr;
	std::unordered_map<std::wstring, int32_t> indexMap;
	std::unordered_map<int32_t, D3D12_SHADER_INPUT_BIND_DESC> reflectionInfo;
};

template<typename T>
RootSignatureInfo CreateRootSignature(const T& psoDesc) requires
															std::is_same_v<T, GraphicsPipelineStateDesc> ||
															std::is_same_v<T, ComputePipelineStateDesc>
{
	RootSignatureInfo info;

	using ShaderArray = std::conditional_t<std::is_same_v<T, GraphicsPipelineStateDesc>,
										   std::array<D3D12Shader*, 2>,
										   std::array<D3D12Shader*, 1>>;

	ShaderArray shaders;
	if constexpr (std::is_same_v<T, GraphicsPipelineStateDesc>)
	{
		shaders = {
			static_cast<D3D12Shader*>(psoDesc.vs),
			static_cast<D3D12Shader*>(psoDesc.ps)
		};
	}
	else
	{
		shaders = {
			static_cast<D3D12Shader*>(psoDesc.cs)
		};
	}

	std::vector<D3D12_ROOT_PARAMETER> rootParams;
	std::forward_list<D3D12_DESCRIPTOR_RANGE> descriptorRanges; // To keep ranges alive since root parameter takes a pointer

	for (D3D12Shader* shader : shaders)
	{
		ID3D12ShaderReflection* reflection = shader->GetCompilerOutput().reflection.Get();

		D3D12_SHADER_DESC shaderDesc;
		PE_ASSERT_HR(reflection->GetDesc(&shaderDesc));

		// Iterate over all the resources bound in the shader like cbuffers, textures and samplers
		for (UINT resIndex = 0; resIndex < shaderDesc.BoundResources; ++resIndex)
		{
			D3D12_SHADER_INPUT_BIND_DESC resourceDesc = {};
			PE_ASSERT_HR(reflection->GetResourceBindingDesc(resIndex, &resourceDesc));

			std::wstring resourceName = StringToWString(resourceDesc.Name);
			if (!info.indexMap.contains(resourceName))
			{
				if (resourceDesc.Type == D3D_SIT_CBUFFER)
				{
					auto& range = descriptorRanges.emplace_front(D3D12_DESCRIPTOR_RANGE{
						.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
						.NumDescriptors = 1,
						.BaseShaderRegister = resourceDesc.BindPoint,
						.RegisterSpace = resourceDesc.Space,
						.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
						});

					D3D12_ROOT_PARAMETER rootParam = {
						.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
						.DescriptorTable = {
							.NumDescriptorRanges = 1,
							.pDescriptorRanges = &range
						},
						//.ShaderVisibility = 
					};

					rootParams.push_back(rootParam);
				}
				else if (resourceDesc.Type == D3D_SIT_TEXTURE)
				{
					auto& range = descriptorRanges.emplace_front(D3D12_DESCRIPTOR_RANGE{
						.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
						.NumDescriptors = resourceDesc.BindCount,
						.BaseShaderRegister = resourceDesc.BindPoint,
						.RegisterSpace = resourceDesc.Space,
						.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
						});

					D3D12_ROOT_PARAMETER rootParam = {
						.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
						.DescriptorTable = {
							.NumDescriptorRanges = 1,
							.pDescriptorRanges = &range
						},
						//.ShaderVisibility = 
					};

					rootParams.push_back(rootParam);
				}
				else if (resourceDesc.Type == D3D_SIT_UAV_RWTYPED)
				{
					auto& range = descriptorRanges.emplace_front(D3D12_DESCRIPTOR_RANGE{
						.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
						.NumDescriptors = resourceDesc.BindCount,
						.BaseShaderRegister = resourceDesc.BindPoint,
						.RegisterSpace = resourceDesc.Space,
						.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
					});

					D3D12_ROOT_PARAMETER rootParam = {
						.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
						.DescriptorTable = {
							.NumDescriptorRanges = 1,
							.pDescriptorRanges = &range
						},
						//.ShaderVisibility = 
					};

					rootParams.push_back(rootParam);
				}
				else if (resourceDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
				{
					auto& range = descriptorRanges.emplace_front(D3D12_DESCRIPTOR_RANGE{
						.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
						.NumDescriptors = resourceDesc.BindCount,
						.BaseShaderRegister = resourceDesc.BindPoint,
						.RegisterSpace = resourceDesc.Space,
						.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
					});

					D3D12_ROOT_PARAMETER rootParam = {
						.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
						.DescriptorTable = {
							.NumDescriptorRanges = 1,
							.pDescriptorRanges = &range
						},
						//.ShaderVisibility = 
					};

					rootParams.push_back(rootParam);
				}
				else
				{
					continue;
				}

				int32_t rootParamIndex = (int32_t)rootParams.size() - 1;
				// This allows us to refer to the params by name, not an index
				info.indexMap[resourceName] = rootParamIndex;
				info.reflectionInfo[rootParamIndex] = resourceDesc;
			}
		}
	}


	// TODO
	const static CD3DX12_STATIC_SAMPLER_DESC s_pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const static CD3DX12_STATIC_SAMPLER_DESC s_pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const static CD3DX12_STATIC_SAMPLER_DESC s_linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const static CD3DX12_STATIC_SAMPLER_DESC s_linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const static CD3DX12_STATIC_SAMPLER_DESC s_anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressW
		0.0f, // mipLODBias
		8); // maxAnisotropy

	const static CD3DX12_STATIC_SAMPLER_DESC s_anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressW
		0.0f, // mipLODBias
		8); // maxAnisotropy

	const static CD3DX12_STATIC_SAMPLER_DESC s_shadow(
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

	// Create the root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc((UINT)rootParams.size(), rootParams.data(),
		(UINT)samplers.size(), samplers.data(),
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
		IID_PPV_ARGS(&info.rootSignature)));

	return info;
}

D3D12RootSignature::D3D12RootSignature(const GraphicsPipelineStateDesc& psoDesc)
{
	auto [rootSig, indexMap, reflection] = CreateRootSignature(psoDesc);
	m_rootSignature = rootSig;
	m_rootParamsIndexMap = indexMap;
	m_rootParamsReflection = reflection;
}

D3D12RootSignature::D3D12RootSignature(const ComputePipelineStateDesc& psoDesc)
{
	auto [rootSig, indexMap, reflection] = CreateRootSignature(psoDesc);
	m_rootSignature = rootSig;
	m_rootParamsIndexMap = indexMap;
	m_rootParamsReflection = reflection;
}

int32_t D3D12RootSignature::GetParamIndex(const std::wstring& paramName)
{
	auto it = m_rootParamsIndexMap.find(paramName);
	if (it == m_rootParamsIndexMap.end())
		return -1;
	return it->second;
}

D3D12_SHADER_INPUT_BIND_DESC D3D12RootSignature::GetReflectionForRootParam(int32_t paramIndex)
{
	auto it = m_rootParamsReflection.find(paramIndex);
	if (it == m_rootParamsReflection.end())
	{
		PE_ASSERT_NO_ENTRY();
		return {};
	}

	return it->second;
}
}
