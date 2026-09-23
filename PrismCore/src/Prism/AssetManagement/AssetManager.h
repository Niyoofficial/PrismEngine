#pragma once

#include "pcpch.h"
#include "Prism/AssetManagement/AssetType.h"

namespace Prism
{
class AssetType;

struct AssetMeta
{
	std::fs::path filepath;
};

// This class is internally synchronized and thread-safe
class AssetManager
{
public:
	static AssetManager& Get();

	AssetManager();

	Ref<Asset> FindAsset(AssetHandle handle) const;
	Ref<Asset> FindAsset(std::fs::path path) const;
	Ref<Asset> LoadAsset(AssetHandle handle);
	Ref<Asset> LoadAsset(std::fs::path path);
	std::future<Ref<Asset>> LoadAssetAsync(AssetHandle handle);
	std::future<Ref<Asset>> LoadAssetAsync(std::fs::path path);

	template<typename T>
	Ref<T> FindAsset(AssetHandle handle) const requires std::is_base_of_v<Asset, T>
	{
		if (Ref<T> asset = dynamic_cast<T*>(FindAsset(handle).Raw()))
			return asset;
		return {};
	}
	template<typename T>
	Ref<T> FindAsset(std::fs::path path) const requires std::is_base_of_v<Asset, T>
	{
		if (Ref<T> asset = dynamic_cast<T*>(FindAsset(path).Raw()))
			return asset;
		return {};
	}
	template<typename T>
	Ref<T> LoadAsset(AssetHandle handle) requires std::is_base_of_v<Asset, T>
	{
		if (Ref<T> asset = dynamic_cast<T*>(LoadAsset(handle).Raw()))
			return asset;
		return {};
	}
	template<typename T>
	Ref<T> LoadAsset(std::fs::path path) requires std::is_base_of_v<Asset, T>
	{
		if (Ref<T> asset = dynamic_cast<T*>(LoadAsset(path).Raw()))
			return asset;
		return {};
	}
	template<typename T>
	std::future<Ref<T>> LoadAssetAsync(AssetHandle handle) requires std::is_base_of_v<Asset, T>
	{
		return LoadAssetAsync(handle);
	}
	template<typename T>
	std::future<Ref<Asset>> LoadAssetAsync(std::fs::path path) requires std::is_base_of_v<Asset, T>
	{
		return LoadAssetAsync(path);
	}

	// Create asset without assigning it to any path, this is useful for assets that are generated at runtime and don't have a file representation
	Ref<Asset> CreateAsset(AssetType* assetType);
	template<typename T>
	Ref<T> CreateAsset() requires std::is_base_of_v<Asset, T>
	{
		return dynamic_cast<T*>(CreateAsset(AssetTypeRegistry::Get().GetAssetTypeForAsset<T>()).Raw());
	}

	/**
	 * @param asset Asset you want to save
	 * @param filePath File path to use to save the asset, if no path is specified it will be taken from the asset itself
	 * (if the asset doesn't have a path i.e. it is a runtime generated asset an assert will be triggered)
	 */
	void SaveAsset(const Ref<Asset>& asset, std::fs::path filePath = "");

	// Searches the registry to find if there is any handle associated with this path
	AssetHandle GetHandleFromPath(std::fs::path path) const;
	std::fs::path GetPathFromHandle(AssetHandle handle) const;

	YAML::Node GetMetadata(std::fs::path path);
	YAML::Node GetMetadata(AssetHandle handle);

	std::fs::path NormalizePath(std::fs::path path) const;

private:
	void SaveMetadata(AssetHandle handle, const YAML::Emitter& emitter);
	void CreateDefaultMetadata(YAML::Emitter& emitter, AssetHandle handle, const std::fs::path& path);

	Ref<Asset> CreateLoadedAsset(std::fs::path path, AssetHandle handle);

private:
	std::unordered_map<AssetHandle, AssetMeta> m_registry;
	mutable std::shared_mutex m_registryMutex;
	std::unordered_map<AssetHandle, WeakRef<Asset>> m_loadedAssets;
	mutable std::shared_mutex m_loadedAssetsMutex;
};
}
