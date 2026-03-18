#pragma once
#include <shared_mutex>

#include "Prism/AssetManagement/Asset.h"

namespace Prism
{
class AssetType
{
public:
	AssetType();
	virtual ~AssetType() = default;

	virtual std::vector<std::fs::path> GetAssociatedExtensions() const = 0;
	virtual Ref<Asset> CreateAsset(AssetManager* assetManager, std::fs::path path) = 0;
	virtual glm::float4 GetAssetIndicatorColor() const = 0;
	virtual std::string GetFileTypeName() const = 0;
};

class TextureAssetType : public AssetType
{
public:
	virtual std::vector<std::fs::path> GetAssociatedExtensions() const override;
	virtual Ref<Asset> CreateAsset(AssetManager* assetManager, std::fs::path path) override;
	glm::float4 GetAssetIndicatorColor() const override;
	std::string GetFileTypeName() const override;
};

// This class is internally synchronized and therefore thread-safe
class AssetTypeRegistry
{
public:
	static AssetTypeRegistry& Get();

	AssetType* GetAssetTypeForExtension(std::fs::path extension);

	void AddAssetTypeToRegistry(AssetType* assetType);

	void BuildAssetTypeAssociations();

private:
	std::vector<AssetType*> m_collectedTypes;
	std::shared_mutex m_collectedTypesMutex;
	std::unordered_map<std::fs::path, AssetType*> m_assetTypes;
	std::shared_mutex m_assetTypesMutex;
};
}

#define REGISTER_ASSET_TYPE(typeClass)			\
	namespace Prism::GeneratedAssetTypes		\
	{											\
	static typeClass g_##typeClass##_generated;	\
	} static_assert(true)

REGISTER_ASSET_TYPE(TextureAssetType);
