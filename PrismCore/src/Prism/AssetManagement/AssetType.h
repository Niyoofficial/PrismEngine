#pragma once
#include <shared_mutex>

#include "Prism/AssetManagement/Asset.h"

namespace Prism
{
class AssetType
{
	friend class AssetManager;
public:
	AssetType();
	virtual ~AssetType() = default;

	virtual std::unordered_set<std::fs::path> GetAssociatedExtensions() const = 0;
	virtual glm::float4 GetAssetIndicatorColor() const = 0;
	virtual std::string GetFileTypeName() const = 0;
	virtual size_t GetAssetTypeHash() const = 0;

	virtual void OpenAsset(const std::fs::path& assetPath) = 0;

protected:
	virtual Ref<Asset> CreateAsset(AssetManager* assetManager, AssetHandle handle) = 0;
};

class TextureAssetType : public AssetType
{
public:
	static TextureAssetType* Get();

	virtual std::unordered_set<std::fs::path> GetAssociatedExtensions() const override;
    virtual glm::float4 GetAssetIndicatorColor() const override;
    virtual std::string GetFileTypeName() const override;
    virtual size_t GetAssetTypeHash() const override;

    virtual void OpenAsset(const std::fs::path& assetPath) override {}

protected:
	virtual Ref<Asset> CreateAsset(AssetManager* assetManager, AssetHandle handle) override;
};

class MeshAssetType : public AssetType
{
public:
	static MeshAssetType* Get();

	virtual std::unordered_set<std::fs::path> GetAssociatedExtensions() const override;
    virtual glm::float4 GetAssetIndicatorColor() const override;
    virtual std::string GetFileTypeName() const override;
    virtual size_t GetAssetTypeHash() const override;

	virtual void OpenAsset(const std::fs::path& assetPath) override {}

protected:
	virtual Ref<Asset> CreateAsset(AssetManager* assetManager, AssetHandle handle) override;
};

class SceneAssetType : public AssetType
{
public:
	static SceneAssetType* Get();

	virtual std::unordered_set<std::fs::path> GetAssociatedExtensions() const override;
	virtual glm::float4 GetAssetIndicatorColor() const override;
	virtual std::string GetFileTypeName() const override;
	virtual size_t GetAssetTypeHash() const override;

	virtual void OpenAsset(const std::fs::path& assetPath) override;

	void SetOnOpenScene(const std::function<void(class Scene*)>& func);

protected:
	virtual Ref<Asset> CreateAsset(AssetManager* assetManager, AssetHandle handle) override;

private:
    std::function<void(Scene*)> m_onOpenScene;
};

// This class is internally synchronized and therefore thread-safe
class AssetTypeRegistry
{
public:
	static AssetTypeRegistry& Get();

	AssetType* GetAssetTypeForExtension(std::fs::path extension) const;
	AssetType* GetAssetTypeForAssetHash(size_t typeHash);
	template<typename T>
	AssetType* GetAssetTypeForAsset() requires std::is_base_of_v<Asset, T>
	{
		return GetAssetTypeForAssetHash(typeid(T).hash_code());
	}
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
	inline typeClass g_##typeClass##_generated;	\
	} static_assert(true)

REGISTER_ASSET_TYPE(TextureAssetType);
REGISTER_ASSET_TYPE(MeshAssetType);
REGISTER_ASSET_TYPE(SceneAssetType);
