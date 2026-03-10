#include "Asset.h"

namespace Prism
{
Asset::Asset(AssetManager* assetManager, std::fs::path path)
	: m_assetManager(assetManager), m_assetPath(path)
{
}
}
