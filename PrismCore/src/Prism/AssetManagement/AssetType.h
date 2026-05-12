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
	static TextureAssetType* Get();

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

	AssetType* GetAssetTypeForExtension(std::fs::path extension) const;
	template<typename T>
	T* GetAssetType() const requires std::is_base_of_v<AssetType, T>
	{
		auto it = std::ranges::find_if(m_collectedTypes,
			[](AssetType* type)
			{
				return dynamic_cast<T*>(type);
			});
		if (it != m_collectedTypes.end())
			return static_cast<T*>(*it);
		return nullptr;
	}

	void AddAssetTypeToRegistry(AssetType* assetType);

	void BuildAssetTypeAssociations();

private:
	std::vector<AssetType*> m_collectedTypes;
	std::shared_mutex m_collectedTypesMutex;
	std::unordered_map<std::fs::path, AssetType*> m_assetTypes;
	mutable std::shared_mutex m_assetTypesMutex;
};
}

#define REGISTER_ASSET_TYPE(typeClass)			\
	namespace Prism::GeneratedAssetTypes		\
	{											\
	static typeClass g_##typeClass##_generated;	\
	} static_assert(true)

REGISTER_ASSET_TYPE(TextureAssetType);
