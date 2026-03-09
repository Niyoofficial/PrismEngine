#include "AssetManager.h"

#include "Prism/Base/Paths.h"

namespace Prism
{
Ref<Asset> AssetManager::FindAsset(AssetHandle handle) const
{
	auto it = m_loadedAssets.find(handle);
	if (it != m_loadedAssets.end() && it->second.IsValid())
		return it->second.Raw();

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
	if (auto asset = FindAsset(handle))
		return asset;

	auto future = LoadAssetAsync(handle);
	return future.get();
}

Ref<Asset> AssetManager::LoadAsset(std::fs::path path)
{
	auto handle = GetHandleFromPath(path);
	if (handle.isValid())
		return LoadAsset(handle);

	return {};
}

std::future<Ref<Asset>> AssetManager::LoadAssetAsync(AssetHandle handle)
{
	return {};
}

std::future<Ref<Asset>> AssetManager::LoadAssetAsync(std::fs::path path)
{
	auto handle = GetHandleFromPath(path);
	if (handle.isValid())
		return LoadAssetAsync(handle);

	return {};
}

AssetHandle AssetManager::GetHandleFromPath(std::fs::path path) const
{
	if (path.is_absolute())
	{
		PE_ASSERT(false, "Path has to be relative, you can use engine/ prefix to refer to engine assets");
		return {};
	}

	auto it = std::ranges::find_if(m_registry,
								   [pathToTest = path.lexically_normal()](auto pair)
								   {
									   return pair.second.filepath == pathToTest;
								   });

	return it != m_registry.end() ? it->first : AssetHandle{};
}
}
