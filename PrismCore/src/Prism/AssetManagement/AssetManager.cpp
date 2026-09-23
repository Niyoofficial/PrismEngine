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

	std::vector<std::fs::path> metaFiles;
	for (const auto& entry : std::fs::recursive_directory_iterator(Core::Paths::Get().GetEngineAssetsDir()))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".meta")
			metaFiles.emplace_back(entry);
	}
	for (const auto& entry : std::fs::recursive_directory_iterator(Core::Paths::Get().GetProjectAssetsDir()))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".meta")
			metaFiles.emplace_back(entry);
	}

	{
		std::unique_lock lock(m_registryMutex);

		for (const auto& file : metaFiles)
		{
			auto node = YAML::LoadFile(file.string());
			auto filepath = node["Filepath"].as<std::string>();
			auto handle = AssetHandle(node["Handle"].as<std::string>());

			m_registry[handle] = {.filepath = filepath};
		}
	}
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
	if (!handle.isValid())
		return {};

	{
		std::shared_lock lock(m_loadedAssetsMutex);

		if (m_loadedAssets.contains(handle))
			return m_loadedAssets.at(handle).Raw();
	}

	std::fs::path assetPath;
	{
		std::shared_lock lock(m_registryMutex);

		PE_ASSERT(m_registry.contains(handle), "Registry doesn't contain handle, where did this handle come from?");
		assetPath = m_registry.at(handle).filepath;
	}

	PE_ASSERT(!assetPath.empty(), "This asset doesn't have a file path assigned, how did this happen?");

	return CreateLoadedAsset(assetPath, handle);
}

Ref<Asset> AssetManager::LoadAsset(std::fs::path path)
{
	auto normalizedPath = NormalizePath(path);

	auto handle = GetHandleFromPath(normalizedPath);
	if (!handle.isValid())
	{
		if (auto node = GetMetadata(path))
		{
			handle = AssetHandle(node["Handle"].as<std::string>());

			{
				std::unique_lock lock(m_registryMutex);

				PE_ASSERT(!m_registry.contains(handle));

				m_registry[handle] = {
					.filepath = path
				};
			}
		}
		else
		{
			handle = xg::newGuid();

			PE_CORE_LOG(Info, "First time registration of asset {} generated handle: {}", path.string(), handle.str());

			{
				std::unique_lock lock(m_registryMutex);

				PE_ASSERT(!m_registry.contains(handle));

				m_registry[handle] = {
					.filepath = path
				};
			}

			YAML::Emitter out;
			CreateDefaultMetadata(out, handle, path);

			SaveMetadata(handle, out);
		}
	}

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

Ref<Asset> AssetManager::CreateAsset(AssetType* assetType)
{
	AssetHandle handle = xg::newGuid();

	{
		std::unique_lock lock(m_registryMutex);

		m_registry[handle] = {
			.filepath = ""
		};
	}

	Ref asset = assetType->CreateAsset(this, handle);

	{
		std::unique_lock lock(m_loadedAssetsMutex);
		m_loadedAssets[handle] = asset;
	}

	return asset;
}

void AssetManager::SaveAsset(const Ref<Asset>& asset, std::fs::path filePath)
{
	PE_ASSERT(asset);

	if (filePath.empty())
		filePath = GetPathFromHandle(asset->GetHandle());
	else
		filePath = AssetRegistry::Get().GetRelPath(filePath);

	auto allowedExtensions = asset->GetAssetType()->GetAssociatedExtensions();
	if (!allowedExtensions.contains(filePath.extension()))
		filePath.replace_extension(*allowedExtensions.begin());
	
	filePath = NormalizePath(filePath);

	{
		std::unique_lock lock(m_registryMutex);

		m_registry.at(asset->GetHandle()) = {
			.filepath = filePath
		};
	}

	YAML::Emitter emitter;
	CreateDefaultMetadata(emitter, asset->GetHandle(), filePath);
	asset->SaveAsset(filePath, emitter);
	SaveMetadata(asset->GetHandle(), emitter);
}

AssetHandle AssetManager::GetHandleFromPath(std::fs::path path) const
{
	PE_ASSERT(!path.empty());

	std::shared_lock lock(m_registryMutex);

	auto it = std::ranges::find_if(m_registry,
								   [pathToTest = NormalizePath(path)](auto pair)
								   {
									   return pair.second.filepath == pathToTest;
								   });

	return it != m_registry.end() ? it->first : AssetHandle{};
}

std::fs::path AssetManager::GetPathFromHandle(AssetHandle handle) const
{
	PE_ASSERT(handle.isValid());
	PE_ASSERT(m_registry.contains(handle), "Registry doesn't contain handle, where did this handle come from?");
	return m_registry.at(handle).filepath;
}

YAML::Node AssetManager::GetMetadata(std::fs::path path)
{
	auto metaPath = AssetRegistry::Get().GetAbsPath(path.replace_extension(".meta"));
	try
	{
		return YAML::LoadFile(metaPath.string());
	}
	catch (const YAML::BadFile&)
	{
		return YAML::Node(YAML::NodeType::Undefined);
	}
}

YAML::Node AssetManager::GetMetadata(AssetHandle handle)
{
	return GetMetadata(GetPathFromHandle(handle));
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

void AssetManager::SaveMetadata(AssetHandle handle, const YAML::Emitter& emitter)
{
	std::fs::path metaPath = AssetRegistry::Get().GetAbsPath(m_registry.at(handle).filepath.replace_extension(".meta"));
	std::ofstream file(metaPath, std::ios::out);
	file.write(emitter.c_str(), (std::streamsize)emitter.size());
	file.close();
}

void AssetManager::CreateDefaultMetadata(YAML::Emitter& emitter, AssetHandle handle, const std::fs::path& path)
{
	emitter << YAML::BeginMap;
	emitter << YAML::Key << "Filepath" << YAML::Value << path.string();
	emitter << YAML::Key << "Handle" << YAML::Value << handle;
	emitter << YAML::EndMap;
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
	Ref asset = assetType->CreateAsset(this, handle);

	{
		std::unique_lock lock(m_loadedAssetsMutex);
		m_loadedAssets[handle] = asset;
	}

	return asset;
}
}
