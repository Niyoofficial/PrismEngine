#pragma once

#include "pcpch.h"

namespace Prism
{
class Asset : public RefCounted
{
public:
	explicit Asset(class AssetManager* assetManager, std::fs::path path);

	std::fs::path GetAssetPath() const { return m_assetPath; }

protected:
	AssetManager* m_assetManager = nullptr;
	std::fs::path m_assetPath;
};
}
