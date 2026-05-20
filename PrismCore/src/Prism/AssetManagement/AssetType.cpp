#include "AssetType.h"

#include "Prism/AssetManagement/AssetManager.h"
#include "Prism/AssetManagement/MeshAsset.h"
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Utilities/LazySingleton.h"

namespace Prism
{
AssetType::AssetType()
{
	AssetTypeRegistry::Get().AddAssetTypeToRegistry(this);
}

TextureAssetType* TextureAssetType::Get()
{
	return AssetTypeRegistry::Get().GetAssetType<TextureAssetType>();
}

std::unordered_set<std::fs::path> TextureAssetType::GetAssociatedExtensions() const
{
	return {
		".jpg", ".jpeg", ".hdr", ".png"
	};
}

Ref<Asset> TextureAssetType::CreateAsset(AssetManager* assetManager, std::fs::path path)
{
	return Ref<TextureAsset>::Create(assetManager, path);
}

glm::float4 TextureAssetType::GetAssetIndicatorColor() const
{
	return {0.4f, 0.f, 0.f, 1.f};
}

std::string TextureAssetType::GetFileTypeName() const
{
	return "Texture";
}

MeshAssetType* MeshAssetType::Get()
{
	return AssetTypeRegistry::Get().GetAssetType<MeshAssetType>();
}

std::unordered_set<std::fs::path> MeshAssetType::GetAssociatedExtensions() const
{
	return {
		".gltf", ".fbx"
	};
}

Ref<Asset> MeshAssetType::CreateAsset(AssetManager* assetManager, std::fs::path path)
{
	return Ref<MeshAsset>::Create(assetManager, path);
}

glm::float4 MeshAssetType::GetAssetIndicatorColor() const
{
	return { 0.2f, 0.3f, 0.7f, 1.f };
}

std::string MeshAssetType::GetFileTypeName() const
{
	return "Mesh";
}

AssetTypeRegistry& AssetTypeRegistry::Get()
{
	return LazySingleton<AssetTypeRegistry>::Get();
}

AssetType* AssetTypeRegistry::GetAssetTypeForExtension(std::fs::path extension) const
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
