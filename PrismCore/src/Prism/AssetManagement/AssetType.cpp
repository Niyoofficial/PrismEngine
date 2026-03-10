#include "AssetType.h"

#include "Prism/AssetManagement/AssetManager.h"
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Utilities/LazySingleton.h"

namespace Prism
{
AssetType::AssetType()
{
	AssetTypeRegistry::Get().AddAssetTypeToRegistry(this);
}

std::vector<std::fs::path> TextureAssetType::GetAssociatedExtensions() const
{
	return {
		".jpg", ".jpeg", ".hdr", ".png"
	};
}

Ref<Asset> TextureAssetType::CreateAsset(AssetManager* assetManager, std::fs::path path)
{
	return Ref<TextureAsset>::Create(assetManager, path);
}

AssetTypeRegistry& AssetTypeRegistry::Get()
{
	return LazySingleton<AssetTypeRegistry>::Get();
}

AssetType* AssetTypeRegistry::GetAssetTypeForExtension(std::fs::path extension)
{
	std::shared_lock lock(m_assetTypesMutex);
	auto it = m_assetTypes.find(extension);
	return it != m_assetTypes.end() ? it->second : nullptr;
}

void AssetTypeRegistry::AddAssetTypeToRegistry(AssetType* assetType)
{
	PE_ASSERT(assetType);

	std::unique_lock lock(m_collectedTypesMutex);
	m_collectedTypes.push_back(assetType);
}

void AssetTypeRegistry::BuildAssetTypeAssociations()
{
	std::shared_lock collectedTypesLock(m_collectedTypesMutex);
	std::unique_lock lock(m_assetTypesMutex);
	for (auto* assetType : m_collectedTypes)
	{
		for (auto& ext : assetType->GetAssociatedExtensions())
			m_assetTypes[ext] = assetType;
	}
}
}
