﻿#include "pcpch.h"
#include "D3D12ShaderCompiler.h"

#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Render/RenderDevice.h"
#include <fstream>
#include <filesystem>


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
	// dxc <shaderName>.hlsl -E <entryPoint> -T <shaderType>_6_8 -Zi -Od -Fo <outputName>.bin -Fd <outputName>.pdb -Qstrip_debug -Qstrip_reflect

	ComPtr<IDxcBlobEncoding> source = nullptr;
	PE_ASSERT_HR(m_dxcUtils->LoadFile(createInfo.filepath.c_str(), nullptr, &source));

	// Open file
	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = source->GetBufferPointer();
	sourceBuffer.Size = source->GetBufferSize();
	sourceBuffer.Encoding = DXC_CP_ACP;

	std::wstring inputPathNoFile = createInfo.filepath.substr(0, createInfo.filepath.find_last_of('/') + 1);
	std::wstring inputFilename = createInfo.filepath.substr(createInfo.filepath.find_last_of('/') + 1);
	std::wstring inputFilenameNoExt = inputFilename.substr(0, inputFilename.find_last_of('.'));
	std::wstring target = GetTargetStringForShader(createInfo.shaderType, 6, 8);
	std::wstring outputFilenameNoExt = inputFilenameNoExt + L"_" + target + L"_" + createInfo.entryName;
	std::wstring binFilename = outputFilenameNoExt + L".bin";
	std::wstring pdbFilename = outputFilenameNoExt + L".pdb";

	const wchar_t* arguments[] = {
		inputFilename.c_str(),					// Shader filename
		L"-E", createInfo.entryName.c_str(),	// Entry point
		L"-T", target.c_str(),					// Target
		L"-Zi", L"-Od",							// Enable debug information, Disable optimization
		L"-Fo", binFilename.c_str(),			// Bin output file
		L"-Fd", pdbFilename.c_str(),			// PDB output file
		L"-Qstrip_debug",						// Strip debug into a separate blob
		L"-Qstrip_reflect",						// Strip reflection into a separate blob
		L"-I", inputPathNoFile.c_str(),			// Shader directory will be used as base director for include handler
		L"-enable-16bit-types"
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
			PE_ASSERT(false, "Shader compilation failed!");
			return {};
		}
	}

	D3D12ShaderCompilerOutput output;

	// Get shader bytecode and save binary
	{
		ComPtr<IDxcBlobUtf16> shaderName;
		PE_ASSERT_HR(results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&output.bytecode), &shaderName));

		std::filesystem::create_directory("Int/");

		std::wstring outputFilepath = L"Int/";
		outputFilepath.append(shaderName->GetStringPointer());
		std::ofstream file(outputFilepath, std::ios::out | std::ios::binary);
		file.write((char*)output.bytecode->GetBufferPointer(), (int64_t)output.bytecode->GetBufferSize());
		file.close();
	}

	// Get and save pdb
	{
		ComPtr<IDxcBlob> pdb;
		ComPtr<IDxcBlobUtf16> pdbName;
		PE_ASSERT_HR(results->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdb), &pdbName));

		std::filesystem::create_directory("Int/");

		std::wstring outputFilepath = L"Int/";
		outputFilepath.append(pdbName->GetStringPointer());
		std::ofstream file(outputFilepath, std::ios::out | std::ios::binary);
		file.write((char*)pdb->GetBufferPointer(), (int64_t)pdb->GetBufferSize());
		file.close();
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

std::wstring D3D12ShaderCompiler::GetStringForShader(ShaderType shaderType)
{
	switch (shaderType)
	{
	case ShaderType::VS:
		return L"vs";
	case ShaderType::PS:
		return L"ps";
	case ShaderType::CS:
		return L"cs";
	case ShaderType::GS:
		return L"gs";
	case ShaderType::HS:
		return L"hs";
	case ShaderType::DS:
		return L"ds";
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

std::wstring D3D12ShaderCompiler::GetTargetStringForShader(ShaderType shaderType, int32_t major, int32_t minor)
{
	return std::format(L"{}_{}_{}", GetStringForShader(shaderType), major, minor);
}
}
