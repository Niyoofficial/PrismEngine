#include "pcpch.h"
#include "D3D12ShaderCompiler.h"

#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Render/RenderAPI.h"


namespace Prism::Render::D3D12
{
D3D12ShaderCompiler::D3D12ShaderCompiler()
{
	PE_ASSERT_HR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_dxcUtils)));
	PE_ASSERT_HR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_dxcCompiler)));

	PE_ASSERT_HR(m_dxcUtils->CreateDefaultIncludeHandler(&m_dxcIncludeHandler));
}

D3D12ShaderCompilerOutput D3D12ShaderCompiler::CompileShader(const ShaderCreateInfo& createInfo)
{
	// dxc <shaderName>.hlsl -E <entryPoint> -T <shaderType>_6_0 -Zi -Od -Fo <shaderName>.bin -Fd <shaderName>.pdb -Qstrip_reflect

	ComPtr<IDxcBlobEncoding> source = nullptr;
	PE_ASSERT_HR(m_dxcUtils->LoadFile(createInfo.filepath.c_str(), nullptr, &source));

	// Open file
	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = source->GetBufferPointer();
	sourceBuffer.Size = source->GetBufferSize();
	sourceBuffer.Encoding = DXC_CP_ACP;

	std::wstring filename = createInfo.filepath.substr(createInfo.filepath.find_last_of('/', 1));
	std::wstring filenameNoExt = filename.substr(0, filename.find_first_of('.'));
	std::wstring target = GetTargetStringForShader(createInfo.shaderType, 6, 0);
	std::wstring outputFilename = filenameNoExt + L".bin";
	std::wstring pdbFilename = filenameNoExt + L".pdb";

	const wchar_t* arguments[] = {
		filename.c_str(),						// Shader filename
		L"-E", createInfo.entryName.c_str(),	// Entry point
		L"-T", target.c_str(),					// Target
		L"-Zi", L"-Od",							// Enable debug information, Disable optimization
		L"-Fo", outputFilename.c_str(),			// Bin output file
		L"-Fd", pdbFilename.c_str(),			// PDB output file
		L"-Qstrip_reflect"						// Strip reflection into a separate blob
	};

	// Compile
	ComPtr<IDxcResult> results;
	PE_ASSERT_HR(m_dxcCompiler->Compile(&sourceBuffer, arguments, _countof(arguments), m_dxcIncludeHandler.Get(), IID_PPV_ARGS(&results)));

	// Check for errors
	{
		ComPtr<IDxcBlobUtf8> errors;
		PE_ASSERT_HR(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
		if (errors != nullptr && errors->GetStringLength() != 0)
		{
			PE_RENDER_LOG(Error, "{}", errors->GetStringPointer());
			return {};
		}
	}

	D3D12ShaderCompilerOutput output;

	// Get shader bytecode
	{
		ComPtr<IDxcBlob> shader;
		ComPtr<IDxcBlobUtf16> shaderName;
		PE_ASSERT_HR(results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), &shaderName));
		if (shader != nullptr)
		{
			output.bytecode.resize(shader->GetBufferSize());
			memcpy_s(output.bytecode.data(), output.bytecode.size(), shader->GetBufferPointer(), shader->GetBufferSize());
		}
	}

	// Get shader reflection
	{
		ComPtr<IDxcBlob> reflectionBlob;
		PE_ASSERT_HR(results->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr));
		if (reflectionBlob != nullptr)
		{
			DxcBuffer reflectionBuffer;
			reflectionBuffer.Encoding = DXC_CP_ACP;
			reflectionBuffer.Ptr = reflectionBlob->GetBufferPointer();
			reflectionBuffer.Size = reflectionBlob->GetBufferSize();

			PE_ASSERT_HR(m_dxcUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&output.reflection)));
		}
	}

	return output;
}

std::wstring D3D12ShaderCompiler::GetTargetStringForShader(ShaderType shaderType, int32_t major, int32_t minor)
{
	std::wstring shader;
	switch (shaderType)
	{
	case ShaderType::VS:
		shader = L"vs";
		break;
	case ShaderType::PS:
		shader = L"ps";
		break;
	case ShaderType::CS:
		shader = L"cs";
		break;
	case ShaderType::GS:
		shader = L"gs";
		break;
	case ShaderType::HS:
		shader = L"hs";
		break;
	case ShaderType::DS:
		shader = L"ds";
		break;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}

	return std::format(L"{}_{}_{}", shader, major, minor);
}
}
