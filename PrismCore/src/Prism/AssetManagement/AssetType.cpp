#include "AssetType.h"

#include "Prism/AssetManagement/AssetManager.h"
#include "Prism/AssetManagement/MeshAsset.h"
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Scene/Scene.h"
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

glm::float4 TextureAssetType::GetAssetIndicatorColor() const
{
	return {0.4f, 0.f, 0.f, 1.f};
}

std::string TextureAssetType::GetFileTypeName() const
{
	return "Texture";
}

size_t TextureAssetType::GetAssetTypeHash() const
{
	return typeid(TextureAsset).hash_code();
}

Ref<Asset> TextureAssetType::CreateAsset(AssetManager* assetManager, AssetHandle handle)
{
	return Ref<TextureAsset>::Create(assetManager, handle);
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

glm::float4 MeshAssetType::GetAssetIndicatorColor() const
{
	return { 0.2f, 0.3f, 0.7f, 1.f };
}

std::string MeshAssetType::GetFileTypeName() const
{
	return "Mesh";
}

size_t MeshAssetType::GetAssetTypeHash() const
{
	return typeid(MeshAsset).hash_code();
}

Ref<Asset> MeshAssetType::CreateAsset(AssetManager* assetManager, AssetHandle handle)
{
	return Ref<MeshAsset>::Create(assetManager, handle);
}

SceneAssetType* SceneAssetType::Get()
{
	return AssetTypeRegistry::Get().GetAssetType<SceneAssetType>();
}

std::unordered_set<std::fs::path> SceneAssetType::GetAssociatedExtensions() const
{
	return {
		".pscene"
	};
}

glm::float4 SceneAssetType::GetAssetIndicatorColor() const
{
	return { 1.f, 0.8f, 0.f, 1.f };
}

std::string SceneAssetType::GetFileTypeName() const
{
	return "Scene";
}

size_t SceneAssetType::GetAssetTypeHash() const
{
	return typeid(Scene).hash_code();
}

void SceneAssetType::OpenAsset(const std::fs::path& assetPath)
{
    auto scene = AssetManager::Get().LoadAsset<Scene>(assetPath);

	if (m_onOpenScene)
        m_onOpenScene(scene);
}

void SceneAssetType::SetOnOpenScene(const std::function<void(Scene*)>& func)
{
    m_onOpenScene = func;
}

Ref<Asset> SceneAssetType::CreateAsset(AssetManager* assetManager, AssetHandle handle)
{
	return Ref<Scene>::Create(assetManager, handle);
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

AssetType* AssetTypeRegistry::GetAssetTypeForAssetHash(size_t typeHash)
{
	for (auto* assetType : m_collectedTypes)
	{
		if (assetType->GetAssetTypeHash() == typeHash)
			return assetType;
	}
	return nullptr;
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
