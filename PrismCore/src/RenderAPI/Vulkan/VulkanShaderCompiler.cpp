#include "VulkanShaderCompiler.h"
#include "Prism/Base/Paths.h"
#include "Prism/Render/RenderTypes.h"
#include "xxhash.h"

Prism::Render::Vulkan::VulkanShaderCompiler::VulkanShaderCompiler()
{
	HRESULT dxcUtilsResult = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_dxcUtils));
	PE_ASSERT(SUCCEEDED(dxcUtilsResult), "DxcCreateInstance(DxcUtils) failed: HRESULT = 0x{:08X}",
	          static_cast<uint32_t>(dxcUtilsResult));

	HRESULT dxcCompilerResult = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_dxcCompiler));
	PE_ASSERT(SUCCEEDED(dxcCompilerResult), "DxcCreateInstance(CLSID_DxcCompiler) failed: HRESULT = 0x{:08X}",
	          static_cast<uint32_t>(dxcCompilerResult));

	HRESULT includeHeaderResult = m_dxcUtils->CreateDefaultIncludeHandler(&m_dxcIncludeHandler);
	PE_ASSERT(SUCCEEDED(includeHeaderResult), "CreateDefaultIncludeHandler failed: HRESULT = 0x{:08X}",
	          static_cast<uint32_t>(includeHeaderResult));

	for (std::error_code error;
	     const auto& file : std::filesystem::directory_iterator(Core::Paths::Get().GetIntermediateDir(), error))
	{
		if (!file.is_regular_file())
		{
			continue;
		}

		if (file.path().extension() != ".spv")
		{
			continue;
		}

		auto metaFile = file.path();
		metaFile.replace_extension(".meta");

		if (!exists(metaFile))
		{
			PE_RENDER_LOG(Warn, "Shader {} has no metadata.", file.path().filename().generic_string());
			continue;
		}

		XXH64_hash_t hash;
		try
		{
			hash = std::stoull(file.path().stem().string());
		}
		catch (const std::invalid_argument& ex)
		{
			PE_RENDER_LOG(Warn, "Could not read the hash value from the name of the shader binary file {}",
			              file.path().filename().generic_string());
			continue;
		}

		VulkanShaderCompilerOutput output;

		std::ifstream shaderFile(file.path(), std::ios::binary | std::ios::ate);
		if (!shaderFile)
		{
			continue;
		}

		auto size = shaderFile.tellg();

		shaderFile.seekg(0);

		output.spirv.resize(size / sizeof(uint32_t));

		shaderFile.read(reinterpret_cast<char*>(output.spirv.data()), size);

		if (SpvReflectResult result = output.reflection.Create(output.spirv.size() * sizeof(uint32_t), output.spirv.data());
		    result != SPV_REFLECT_RESULT_SUCCESS)
		{
			PE_RENDER_LOG(Error, "Failed to reflect shader {}", file.path().string());

			continue;
		}

		YAML::Node metadata = YAML::LoadFile(metaFile.generic_string());

		ShaderDesc desc = {
		    .filepath = StringToWString(metadata["ShaderDesc"]["filepath"].as<std::string>()),
		    .entryName = StringToWString(metadata["ShaderDesc"]["entryName"].as<std::string>()),
		    .shaderType = static_cast<ShaderType>(metadata["ShaderDesc"]["shaderType"].as<int32_t>()),
		};

		std::filesystem::path enginePath = Core::Paths::Get().GetEngineDir() / desc.filepath;

		if ((exists(desc.filepath) && is_regular_file(desc.filepath)) || (exists(enginePath) && is_regular_file(enginePath)))
		{
			m_shaderCache[desc] = {hash, std::move(output)};

			PE_RENDER_LOG(Info, "Loaded cached shader {}", WStringToString(desc.filepath));

			CompileShader(desc);
		}
		else
		{
			RemoveShaderCache(hash);
		}
	}
}

const Prism::Render::Vulkan::VulkanShaderCompilerOutput&
Prism::Render::Vulkan::VulkanShaderCompiler::GetOrCreateShader(const ShaderDesc& desc)
{
	if (!m_shaderCache.contains(desc))
	{
		PE_RENDER_LOG(Info, "BEFORE COMPILESHADER PIPELINE VS: file='{}' entry='{}' type={}", WStringToString(desc.filepath),
		              WStringToString(desc.entryName), static_cast<int>(desc.shaderType));

		PE_RENDER_LOG(Info, "BEFORE COMPILESHADER PIPELINE PS: file='{}' entry='{}' type={}", WStringToString(desc.filepath),
		              WStringToString(desc.entryName), static_cast<int>(desc.shaderType));

		CompileShader(desc);
	}

	PE_ASSERT(m_shaderCache.contains(desc), "Shader \"{}\" Entryname {} not found in cache", WStringToString(desc.filepath),
	          WStringToString(desc.entryName));

	return m_shaderCache.at(desc).output;
}

void Prism::Render::Vulkan::VulkanShaderCompiler::CompileShader(const ShaderDesc& desc)
{
	const std::filesystem::path engineInputPath = Core::Paths::Get().GetEngineDir() / desc.filepath;

	ComPtr<IDxcBlobEncoding> source;

	HRESULT result = m_dxcUtils->LoadFile(desc.filepath.c_str(), nullptr, &source);

	if (FAILED(result))
	{
		result = m_dxcUtils->LoadFile(engineInputPath.c_str(), nullptr, &source);
	}

	PE_ASSERT(SUCCEEDED(result) && source, "Shader file not found: {}", WStringToString(desc.filepath));

	DxcBuffer sourceBuffer{
	    .Ptr = source->GetBufferPointer(),
	    .Size = source->GetBufferSize(),
	    .Encoding = DXC_CP_ACP,
	};

	std::wstring inputPathNoFile =
	    desc.filepath.lexically_normal().wstring().substr(0, desc.filepath.lexically_normal().wstring().find_last_of(L"/\\") + 1);

	std::wstring engineInputPathNoFile = engineInputPath.lexically_normal().wstring().substr(
	    0, engineInputPath.lexically_normal().wstring().find_last_of(L"/\\") + 1);

	std::wstring inputFilename = desc.filepath.filename().wstring();

	std::wstring target = GetTargetStringForShader(desc.shaderType, 6, 8);

	const wchar_t* preprocessArguments[] = {
	    inputFilename.c_str(), L"-P", L"-I", inputPathNoFile.c_str(), L"-I", engineInputPathNoFile.c_str(), L"-D", L"VULKAN=1",
	};

	const wchar_t* compileArguments[] = {
	    inputFilename.c_str(),
	    L"-E",
	    desc.entryName.c_str(),
	    L"-T",
	    target.c_str(),
	    L"-spirv",
	    L"-fspv-target-env=vulkan1.3",
	    L"-fvk-use-dx-layout",
	    L"-Zi",
#if PE_BUILD_DEBUG
	    L"-Od",
#else
	    L"-O3",
#endif
	    L"-Qembed_debug",
	    L"-Qsource_in_debug_module",
	    L"-I",
	    inputPathNoFile.c_str(),
	    L"-I",
	    engineInputPathNoFile.c_str(),
	    L"-enable-16bit-types",
	    L"-D",
	    L"VULKAN=1",
	};

	static_assert(std::is_same_v<uint64_t, XXH64_hash_t>);

	uint64_t shaderHash{};

	{
		ComPtr<IDxcResult> results;
		PE_ASSERT(SUCCEEDED(m_dxcCompiler->Compile(&sourceBuffer, preprocessArguments, _countof(preprocessArguments),
		                                           m_dxcIncludeHandler.Get(), IID_PPV_ARGS(&results))));

		{
			ComPtr<IDxcBlobUtf8> errors;

			PE_ASSERT(SUCCEEDED(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)));

			PE_ASSERT(!errors || errors->GetStringLength() == 0, "Shader preprocessing failed!\nFile: {}\nEntry: {}\n{}",
			          WStringToString(desc.filepath), WStringToString(desc.entryName), errors ? errors->GetStringPointer() : "");
		}

		{
			ComPtr<IDxcBlobUtf8> preprocess;

			PE_ASSERT(SUCCEEDED(results->GetOutput(DXC_OUT_HLSL, IID_PPV_ARGS(&preprocess), nullptr)));

			auto* hashState = XXH3_createState();

			XXH3_64bits_reset(hashState);

			for (const wchar_t* arg : compileArguments)
			{
				XXH3_64bits_update(hashState, arg, wcslen(arg) * sizeof(wchar_t));
			}

			XXH3_64bits_update(hashState, preprocess->GetStringPointer(), preprocess->GetStringLength());

			shaderHash = XXH3_64bits_digest(hashState);

			XXH3_freeState(hashState);

			if (m_shaderCache.contains(desc))
			{
				if (m_shaderCache.at(desc).hash == shaderHash)
				{
					return;
				}

				RemoveShaderCache(m_shaderCache.at(desc).hash);
			}
		}
	}

	std::filesystem::path outputFilepathNoExt = Core::Paths::Get().GetIntermediateDir() / std::to_string(shaderHash);

	VulkanShaderCompilerOutput output;

	{
		PE_RENDER_LOG(Info, "Compiling {}, Entry: {}, Shader hash: {}", WStringToString(desc.filepath),
		              WStringToString(desc.entryName), shaderHash);

		ComPtr<IDxcResult> results;

		PE_ASSERT(SUCCEEDED(m_dxcCompiler->Compile(&sourceBuffer, compileArguments, _countof(compileArguments),
		                                           m_dxcIncludeHandler.Get(), IID_PPV_ARGS(&results))));

		{
			ComPtr<IDxcBlobUtf8> errors;

			PE_ASSERT(SUCCEEDED(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)));

			if (errors && errors->GetStringLength() != 0)
			{
				PE_RENDER_LOG(Error, "Shader compilation failed!\nFile: {}\nEntry: {}\n{}", WStringToString(desc.filepath),
				              WStringToString(desc.entryName), errors->GetStringPointer());

				return;
			}
		}

		ComPtr<IDxcBlob> spirvBlob;

		PE_ASSERT(SUCCEEDED(results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&spirvBlob), nullptr)));

		output.spirv.resize(spirvBlob->GetBufferSize() / sizeof(uint32_t));

		std::memcpy(output.spirv.data(), spirvBlob->GetBufferPointer(), spirvBlob->GetBufferSize());

		{
			std::filesystem::create_directories(Core::Paths::Get().GetIntermediateDir());

			auto outputFile = outputFilepathNoExt;
			outputFile.replace_extension(".spv");

			std::ofstream file(outputFile, std::ios::binary);

			file.write(reinterpret_cast<char*>(output.spirv.data()),
			           static_cast<std::streamsize>(output.spirv.size() * sizeof(uint32_t)));
		}

		{
			PE_RENDER_LOG(Info, "INSIDE SCOPE PIPELINE VS: file='{}' entry='{}' type={}", WStringToString(desc.filepath),
			              WStringToString(desc.entryName), static_cast<int>(desc.shaderType));

			PE_RENDER_LOG(Info, "INSIDE SCOPE  PIPELINE PS: file='{}' entry='{}' type={}", WStringToString(desc.filepath),
			              WStringToString(desc.entryName), static_cast<int>(desc.shaderType));

			auto outputFile = outputFilepathNoExt;
			outputFile.replace_extension(".meta");

			std::ofstream file(outputFile);

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "ShaderDesc";
			out << YAML::Value << desc;
			out << YAML::EndMap;

			file << out.c_str();
		}

		{
			ComPtr<IDxcBlob> pdb;
			ComPtr<IDxcBlobUtf16> pdbName;

			if (SUCCEEDED(results->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdb), &pdbName)) && pdb)
			{
				auto outputFile = outputFilepathNoExt;
				outputFile.replace_extension(".pdb");

				std::ofstream file(outputFile, std::ios::binary);

				file.write(static_cast<char*>(pdb->GetBufferPointer()), static_cast<std::streamsize>(pdb->GetBufferSize()));
			}
		}

		{
			SpvReflectResult reflectResult =
			    output.reflection.Create(output.spirv.size() * sizeof(uint32_t), output.spirv.data());

			PE_ASSERT(reflectResult == SPV_REFLECT_RESULT_SUCCESS, "Failed to create SPIR-V reflection for {}",
			          WStringToString(desc.filepath));

			for (uint32_t i = 0; i < output.reflection.GetDescriptorSetCount(); i++)
			{
				const auto& set = output.reflection.GetDescriptorSet(i);

				PE_RENDER_LOG(Info, "SET {}", set.set);

				for (uint32_t b = 0; b < set.binding_count; b++)
				{
					const auto& binding = *set.bindings[b];

					PE_RENDER_LOG(Warn, "set={} binding={} name={} type={} count={}", set.set, binding.binding, binding.name,
					              static_cast<uint32_t>(binding.descriptor_type), binding.count);
				}
			}
		}

		m_shaderCache[desc] = {shaderHash, std::move(output)};
	}
}

uint64_t Prism::Render::Vulkan::VulkanShaderCompiler::GetShaderCodeHash(const ShaderDesc& desc)
{
	if (m_shaderCache.contains(desc))
	{
		return m_shaderCache.at(desc).hash;
	}

	return 0;
}

void Prism::Render::Vulkan::VulkanShaderCompiler::RecompileCachedShaders()
{
	for (const auto& desc : m_shaderCache | std::views::keys)
	{
		CompileShader(desc);
	}
}

std::wstring Prism::Render::Vulkan::VulkanShaderCompiler::GetStringForShader(const ShaderType shaderType) const
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

std::wstring Prism::Render::Vulkan::VulkanShaderCompiler::GetTargetStringForShader(const ShaderType shaderType, int32_t major,
                                                                                   int32_t minor) const
{
	return std::format(L"{}_{}_{}", GetStringForShader(shaderType), major, minor);
}

void Prism::Render::Vulkan::VulkanShaderCompiler::RemoveShaderCache(const uint64_t shaderHash)
{
	std::filesystem::path cache = Core::Paths::Get().GetIntermediateDir() / std::to_string(shaderHash);

	std::filesystem::remove(cache.replace_extension(".spv"));
	std::filesystem::remove(cache.replace_extension(".pdb"));
	std::filesystem::remove(cache.replace_extension(".meta"));
}
