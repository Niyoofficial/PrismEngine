#pragma once

#include "pcpch.h"
#include "Prism/AssetManagement/Asset.h"
#include "crossguid/guid.hpp"

namespace Prism
{
using AssetHandle = xg::Guid;

struct AssetMeta
{
	std::fs::path filepath;
};

// This class is internally synchronized and thread-safe
class AssetManager
{
public:
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
		PE_ASSERT(false);
		return {};
	}
	template<typename T>
	Ref<T> FindAsset(std::fs::path path) const requires std::is_base_of_v<Asset, T>
	{
		if (Ref<T> asset = dynamic_cast<T*>(FindAsset(path).Raw()))
			return asset;
		PE_ASSERT(false);
		return {};
	}
	template<typename T>
	Ref<T> LoadAsset(AssetHandle handle) requires std::is_base_of_v<Asset, T>
	{
		if (Ref<T> asset = dynamic_cast<T*>(LoadAsset(handle).Raw()))
			return asset;
		PE_ASSERT(false);
		return {};
	}
	template<typename T>
	Ref<T> LoadAsset(std::fs::path path) requires std::is_base_of_v<Asset, T>
	{
		if (Ref<T> asset = dynamic_cast<T*>(LoadAsset(path).Raw()))
			return asset;
		PE_ASSERT(false);
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

	// Searches the registry to find if there is any handle associated with this path
	AssetHandle GetHandleFromPath(std::fs::path path) const;

	std::fs::path NormalizePath(std::fs::path path) const;
	std::fs::path GetAbsolutePath(std::fs::path path) const;

private:
	AssetHandle RegisterAsset(std::fs::path path);

	Ref<Asset> CreateLoadedAsset(std::fs::path path, AssetHandle handle);

private:
	std::unordered_map<AssetHandle, AssetMeta> m_registry;
	mutable std::shared_mutex m_registryMutex;
	std::unordered_map<AssetHandle, WeakRef<Asset>> m_loadedAssets;
	mutable std::shared_mutex m_loadedAssetsMutex;
};
}
