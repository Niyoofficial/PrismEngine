#include "pcpch.h"
#include "D3D12ShaderCompiler.h"

#include "Prism/Base/Base.h"
#include "Prism/Render/RenderDevice.h"

#include "Prism/Base/Paths.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"

#include "xxhash.h"
#include "yaml-cpp/yaml.h"

namespace Prism::Render::D3D12
{
D3D12ShaderCompiler::D3D12ShaderCompiler()
{
	PE_ASSERT_HR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_dxcUtils)));
	PE_ASSERT_HR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_dxcCompiler)));

	PE_ASSERT_HR(m_dxcUtils->CreateDefaultIncludeHandler(&m_dxcIncludeHandler));

	std::error_code error;
	for (const auto& file : std::fs::directory_iterator(Core::Paths::Get().GetIntermediateDir(), error))
	{
		if (file.is_regular_file())
		{
			if (file.path().extension().compare(".bin") == 0)
			{
				std::fs::path metaFile = file.path();
				metaFile.replace_extension(".meta");
				if (!exists(metaFile) || !is_regular_file(metaFile))
				{
					PE_RENDER_LOG(Warn, "Shader binary file {} doesn't have the corresponding .meta file", file.path().filename().generic_string());
					continue;
				}

				auto stem = file.path().stem().generic_wstring();
				XXH64_hash_t hash;
				try
				{
					hash = std::stoull(stem);
				}
				catch (const std::invalid_argument& ex)
				{
					PE_RENDER_LOG(Warn, "Could not read the hash value from the name of the shader binary file {}", file.path().filename().generic_string());
					continue;
				}

				D3D12ShaderCompilerOutput output;

				ComPtr<IDxcBlobEncoding> blob;
				PE_ASSERT_HR(m_dxcUtils->LoadFile(file.path().generic_wstring().c_str(), nullptr, &blob));
				output.bytecode = blob;

				DxcBuffer buffer = {
					.Ptr = blob->GetBufferPointer(),
					.Size = blob->GetBufferSize(),
					.Encoding = 0
				};
				PE_ASSERT_HR(m_dxcUtils->CreateReflection(&buffer, IID_PPV_ARGS(&output.reflection)));

				YAML::Node metadata = YAML::LoadFile(metaFile.generic_string());

				ShaderDesc desc = {
					.filepath = StringToWString(metadata["ShaderDesc"]["filepath"].as<std::string>()),
					.entryName = StringToWString(metadata["ShaderDesc"]["entryName"].as<std::string>()),
					.shaderType = (ShaderType)metadata["ShaderDesc"]["shaderType"].as<int32_t>()
				};

				std::fs::path shaderEnginePath = Core::Paths::Get().GetEngineDir() + L"/" + desc.filepath;
				if ((std::fs::exists(desc.filepath.c_str()) && std::fs::is_regular_file(desc.filepath.c_str())) ||
					(std::fs::exists(shaderEnginePath) && std::fs::is_regular_file(shaderEnginePath)))
				{
					m_shaderCache[desc] = {hash, output};

					PE_D3D12_LOG(Info, "Loaded cached shader binary file {} for shader {}, Entryname: {}, Shader code hash: {}",
						file.path().generic_string(), WStringToString(desc.filepath), WStringToString(desc.entryName), hash);

					// Try to compile the shader anyway in case the cached file is outdated
					D3D12ShaderCompiler::CompileShader(desc);
				}
				else
				{
					PE_D3D12_LOG(Info, "Removing outdated cache for shader {}, Entryname: {}, Shader code hash: {}",
						WStringToString(desc.filepath), WStringToString(desc.entryName), hash);

					RemoveShaderCache(hash);
				}
			}
		}
	}
}

D3D12ShaderCompilerOutput D3D12ShaderCompiler::GetOrCreateShader(const ShaderDesc& desc)
{
	if (!m_shaderCache.contains(desc))
		CompileShader(desc);
	PE_ASSERT(m_shaderCache.contains(desc), "Shader \"{}\" Entryname {} not found in cache", WStringToString(desc.filepath), WStringToString(desc.entryName));

	return m_shaderCache.at(desc).output;
}

void D3D12ShaderCompiler::CompileShader(const ShaderDesc& desc)
{
	// dxc <shaderName>.hlsl -E <entryPoint> -T <shaderType>_6_8 -Zi -Od -Fo <outputName>.bin -Fd <outputName>.pdb -I -enable-16bit-types

	std::wstring engineInputPath = Core::Paths::Get().GetEngineDir() + L"/" + desc.filepath;

	ComPtr<IDxcBlobEncoding> source = nullptr;
	HRESULT result = m_dxcUtils->LoadFile(desc.filepath.c_str(), nullptr, &source);
	if (FAILED(result))
	{
		// If shader wasn't found in the project directory, try to load it from the engine directory
		result = m_dxcUtils->LoadFile(engineInputPath.c_str(), nullptr, &source);
	}

	PE_ASSERT(SUCCEEDED(result) && source, "Shader file not found: {}", desc.filepath);

	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = source->GetBufferPointer();
	sourceBuffer.Size = source->GetBufferSize();
	sourceBuffer.Encoding = DXC_CP_ACP;

	std::wstring inputPathNoFile = desc.filepath.substr(0, desc.filepath.find_last_of('/') + 1);
	std::wstring engineInputPathNoFile = engineInputPath.substr(0, engineInputPath.find_last_of('/') + 1);
	std::wstring inputFilename = desc.filepath.substr(desc.filepath.find_last_of('/') + 1);
	std::wstring inputFilenameNoExt = inputFilename.substr(0, inputFilename.find_last_of('.'));
	std::wstring target = GetTargetStringForShader(desc.shaderType, 6, 8);

	const wchar_t* compileArguments[] = {
		inputFilename.c_str(),					// Shader filename
		L"-E", desc.entryName.c_str(),			// Entry point
		L"-T", target.c_str(),					// Target
		L"-Zi",									// Enable debug information
#if PE_BUILD_DEBUG
		L"-Od",									// Disable optimization
#else
		L"-O3",									// Enable optimization
#endif
		//L"-Qstrip_debug",						// Strip debug into a separate blob
		//L"-Qstrip_reflect",					// Strip reflection into a separate blob
		L"-I", inputPathNoFile.c_str(),			// Shader directory will be used as base directory for include handler
		L"-I", engineInputPathNoFile.c_str(),	// Shader directory will be used as base directory for include handler
		L"-enable-16bit-types"
	};

	const wchar_t* preprocessArguments[] = {
		inputFilename.c_str(),					// Shader filename
		L"-P",									// Preprocess
		L"-I", inputPathNoFile.c_str(),			// Shader directory will be used as base directory for include handler
		L"-I", engineInputPathNoFile.c_str(),	// Shader directory will be used as base directory for include handler
	};

	static_assert(std::is_same_v<uint64_t, XXH64_hash_t>);
	uint64_t shaderHash;

	// Shader preprocessing
	{
		ComPtr<IDxcResult> results;
		PE_ASSERT_HR(m_dxcCompiler->Compile(&sourceBuffer, preprocessArguments, _countof(preprocessArguments), m_dxcIncludeHandler.Get(), IID_PPV_ARGS(&results)));

		// Check for errors
		{
			ComPtr<IDxcBlobUtf8> errors;
			PE_ASSERT_HR(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));

			PE_ASSERT(!errors || errors->GetStringLength() == 0, "Shader preprocessing failed! Filename: {} Entryname: {}\n{}", desc.filepath, desc.entryName, errors->GetStringPointer());
		}

		{
			ComPtr<IDxcBlobUtf8> preprocess;
			PE_ASSERT_HR(results->GetOutput(DXC_OUT_HLSL, IID_PPV_ARGS(&preprocess), nullptr));

			auto* hashState = XXH3_createState();

			XXH3_64bits_reset(hashState);

			for (auto& arg : compileArguments)
				XXH3_64bits_update(hashState, arg, wcslen(arg) * sizeof(wchar_t));
			XXH3_64bits_update(hashState, preprocess->GetStringPointer(), preprocess->GetStringLength());

			shaderHash = XXH3_64bits_digest(hashState);

			XXH3_freeState(hashState);

			if (m_shaderCache.contains(desc))
			{
				if (m_shaderCache.at(desc).hash == shaderHash)
				{
					// If the hash is the same, use the cached shader, otherwise compile it
					return;
				}
				else
				{
					// As soon as we know that the shader cached files are outdated, get rid of them, so they won't be mistakenly loaded at the next program load
					std::fs::path cacheFile(Core::Paths::Get().GetIntermediateDir() + L"/" + std::to_wstring(m_shaderCache.at(desc).hash));

					RemoveShaderCache(m_shaderCache.at(desc).hash);
				}
			}
		}
	}

	std::fs::path outputFilepathNoExt = Core::Paths::Get().GetIntermediateDir() + L"/" + std::to_wstring(shaderHash);

	D3D12ShaderCompilerOutput output;

	// Shader compilation
	{
		PE_D3D12_LOG(Info, "Compiling {}, Entryname: {}, Shader code hash: {}", desc.filepath, desc.entryName, shaderHash);

		ComPtr<IDxcResult> results;
		PE_ASSERT_HR(m_dxcCompiler->Compile(&sourceBuffer, compileArguments, _countof(compileArguments), m_dxcIncludeHandler.Get(), IID_PPV_ARGS(&results)));

		// Check for errors
		{
			ComPtr<IDxcBlobUtf8> errors;
			PE_ASSERT_HR(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));

			if (errors && errors->GetStringLength() != 0)
			{
				PE_D3D12_LOG(Error, "Shader compilation failed! Filename: {} Entryname: {}\n{}", desc.filepath, desc.entryName, errors->GetStringPointer());
				return;
			}
		}

		// Get shader bytecode and save binary
		{
			ComPtr<IDxcBlobUtf16> shaderName;
			PE_ASSERT_HR(results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&output.bytecode), &shaderName));

			std::fs::create_directory(Core::Paths::Get().GetIntermediateDir());

			auto outputFilepath = outputFilepathNoExt;
			outputFilepath.replace_extension(".bin");
			std::ofstream file(outputFilepath, std::ios::out | std::ios::binary);
			file.write((char*)output.bytecode->GetBufferPointer(), (int64_t)output.bytecode->GetBufferSize());
			file.close();
		}

		// Save shader metadata
		{
			auto outputFilepath = outputFilepathNoExt;
			outputFilepath.replace_extension(".meta");
			std::ofstream file(outputFilepath, std::ios::out);

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "ShaderDesc";
			out << YAML::Value << desc;
			out << YAML::EndMap;

			file.write(out.c_str(), (std::streamsize)out.size());
			file.close();
		}

		// Get and save pdb
		{
			ComPtr<IDxcBlob> pdb;
			ComPtr<IDxcBlobUtf16> pdbName;
			PE_ASSERT_HR(results->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdb), &pdbName));

			std::fs::create_directory(Core::Paths::Get().GetIntermediateDir());

			auto outputFilepath = outputFilepathNoExt;
			outputFilepath.replace_extension(".pdb");
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

		m_shaderCache[desc] = {shaderHash, output};
	}
}

uint64_t D3D12ShaderCompiler::GetShaderCodeHash(const ShaderDesc& desc)
{
	if (m_shaderCache.contains(desc))
		return m_shaderCache.at(desc).hash;
	else
		return 0;
}

void D3D12ShaderCompiler::RecompileCachedShaders()
{
	for (auto& [desc, shader] : m_shaderCache)
		CompileShader(desc);
}

std::wstring D3D12ShaderCompiler::GetStringForShader(ShaderType shaderType) const
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

std::wstring D3D12ShaderCompiler::GetTargetStringForShader(ShaderType shaderType, int32_t major, int32_t minor) const
{
	return std::format(L"{}_{}_{}", GetStringForShader(shaderType), major, minor);
}

void D3D12ShaderCompiler::RemoveShaderCache(uint64_t shaderHash)
{
	std::fs::path cacheFile(Core::Paths::Get().GetIntermediateDir() + L"/" + std::to_wstring(shaderHash));

	std::fs::remove(cacheFile.replace_extension(L".bin"));
	std::fs::remove(cacheFile.replace_extension(L".pdb"));
	std::fs::remove(cacheFile.replace_extension(L".meta"));
}
}
