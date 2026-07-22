#pragma once

#include "pcpch.h"
#include "crossguid/guid.hpp"

namespace Prism
{
class AssetType;
using AssetHandle = xg::Guid;

// Assets are created by the asset manager
class Asset : public RefCounted
{
friend class AssetManager;
public:
	AssetHandle GetHandle() const { return m_handle; }

	virtual AssetType* GetAssetType() const = 0;

protected:
	Asset(AssetManager* assetManager, AssetHandle handle);

private:

	virtual void SaveAsset(const std::fs::path& path, YAML::Emitter& emitter) = 0;

protected:
	AssetManager* m_assetManager = nullptr;
	const AssetHandle m_handle;
};
}

namespace YAML
{
template<>
struct convert<Prism::AssetHandle>
{
    static Node encode(const Prism::AssetHandle& rhs);
    static bool decode(const Node& node, Prism::AssetHandle& rhs);
};
}
