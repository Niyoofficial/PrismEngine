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

class AssetManager
{
public:
	AssetManager() = default;

	Ref<Asset> FindAsset(AssetHandle handle) const;
	Ref<Asset> FindAsset(std::fs::path path) const;
	Ref<Asset> LoadAsset(AssetHandle handle);
	Ref<Asset> LoadAsset(std::fs::path path);
	std::future<Ref<Asset>> LoadAssetAsync(AssetHandle handle);
	std::future<Ref<Asset>> LoadAssetAsync(std::fs::path path);

	// Searches the registry to find if there is any handle associated with this path
	AssetHandle GetHandleFromPath(std::fs::path path) const;

private:
	std::unordered_map<AssetHandle, AssetMeta> m_registry;
	std::unordered_map<AssetHandle, WeakRef<Asset>> m_loadedAssets;
};
}
