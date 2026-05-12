#pragma once

#include "pcpch.h"

namespace Prism
{
// Assets are created by the asset manager
class Asset : public RefCounted
{
public:
	std::fs::path GetAssetPath() const { return m_assetPath; }

protected:
	Asset(class AssetManager* assetManager, std::fs::path path);

protected:
	AssetManager* m_assetManager = nullptr;
	std::fs::path m_assetPath;
};
}
