#include "AssetManager.h"

#include "Prism/AssetManagement/AssetType.h"
#include "Prism/AssetManagement/MeshAsset.h"
#include "Prism/Base/Application.h"
#include "Prism/Base/Paths.h"

namespace Prism
{
AssetManager& AssetManager::Get()
{
	return Core::Application::Get().GetAssetManager();
}

AssetManager::AssetManager()
{
	InitMeshLoading();
	AssetTypeRegistry::Get().BuildAssetTypeAssociations();
}

Ref<Asset> AssetManager::FindAsset(AssetHandle handle) const
{
	PE_ASSERT(handle.isValid());

	std::shared_lock lock(m_loadedAssetsMutex);

	auto it = m_loadedAssets.find(handle);
	if (it != m_loadedAssets.end() && it->second.IsValid())
		return it->second.IsValid() ? it->second.Raw() : nullptr;

	return {};
}

Ref<Asset> AssetManager::FindAsset(std::fs::path path) const
{
	auto handle = GetHandleFromPath(path);
	if (handle.isValid())
		return FindAsset(handle);

	return {};
}

Ref<Asset> AssetManager::LoadAsset(AssetHandle handle)
{
	PE_ASSERT(handle.isValid());

	std::fs::path assetPath;
	{
		std::shared_lock lock(m_registryMutex);

		PE_ASSERT(m_registry.contains(handle), "Registry doesn't contain handle, where did this handle come from?");
		assetPath = m_registry.at(handle).filepath;
	}

	return CreateLoadedAsset(assetPath, handle);
}

Ref<Asset> AssetManager::LoadAsset(std::fs::path path)
{
	auto normalizedPath = NormalizePath(path);

	auto handle = GetHandleFromPath(normalizedPath);
	if (!handle.isValid())
		// Register the asset if we cannot find handle to it
		handle = RegisterAsset(normalizedPath);

	return CreateLoadedAsset(normalizedPath, handle);
}

std::future<Ref<Asset>> AssetManager::LoadAssetAsync(AssetHandle handle)
{
	return std::async(std::launch::async,
		[this, handle]()
		{
			return LoadAsset(handle);
		});
}

std::future<Ref<Asset>> AssetManager::LoadAssetAsync(std::fs::path path)
{
	return std::async(std::launch::async,
		[this, path]()
		{
			return LoadAsset(path);
		});
}

AssetHandle AssetManager::GetHandleFromPath(std::fs::path path) const
{
	std::shared_lock lock(m_registryMutex);

	auto it = std::ranges::find_if(m_registry,
								   [pathToTest = NormalizePath(path)](auto pair)
								   {
									   return pair.second.filepath == pathToTest;
								   });

	return it != m_registry.end() ? it->first : AssetHandle{};
}

std::fs::path AssetManager::NormalizePath(std::fs::path path) const
{
	if (path.is_absolute())
	{
		PE_ASSERT(false, "Path has to be relative, you can use engine/ prefix to refer to engine assets");
		return {};
	}

	return path.lexically_normal();
}

std::fs::path AssetManager::GetAbsolutePath(std::fs::path path) const
{
	path = NormalizePath(path);

	auto enginePath = path.lexically_relative("engine");
	if (!enginePath.empty() && !enginePath.string().starts_with(".."))
		return Core::Paths::Get().GetEngineAssetsDir() / enginePath;
	else
		return Core::Paths::Get().GetProjectAssetsDir() / path;
}

AssetHandle AssetManager::RegisterAsset(std::fs::path path)
{
	path = NormalizePath(path);

	std::fs::path absPath = GetAbsolutePath(path);

	PE_ASSERT(std::fs::is_regular_file(absPath));

	auto metaPath = absPath.replace_extension(".meta");
	AssetHandle handle;
	if (!std::fs::exists(metaPath))
	{
		handle = xg::newGuid();

		PE_CORE_LOG(Info, "First time registration of asset {} generated handle: {}", path.string(), handle.str());

		std::ofstream file(metaPath, std::ios::out);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Filepath" << YAML::Value << path.string();
		out << YAML::Key << "Handle" << YAML::Value << handle;
		out << YAML::EndMap;

		file.write(out.c_str(), (std::streamsize)out.size());
		file.close();
	}
	else
	{
		auto node = YAML::LoadFile(metaPath.string());
		handle = AssetHandle(node["Handle"].as<std::string>());
	}

	{
		std::unique_lock lock(m_registryMutex);

		PE_ASSERT(!m_registry.contains(handle));

		m_registry[handle] = {
			.filepath = path
		};
	}

	return handle;
}

Ref<Asset> AssetManager::CreateLoadedAsset(std::fs::path path, AssetHandle handle)
{
	{
		std::shared_lock lock(m_loadedAssetsMutex);
		auto it = m_loadedAssets.find(handle);
		if (it != m_loadedAssets.end() && it->second.IsValid())
			return it->second.Raw();
	}

	AssetType* assetType = AssetTypeRegistry::Get().GetAssetTypeForExtension(path.extension());
	Ref asset = assetType->CreateAsset(this, path);

	{
		std::unique_lock lock(m_loadedAssetsMutex);
		m_loadedAssets[handle] = asset;
	}

	return asset;
}
}
