#include "Asset.h"

namespace Prism
{
Asset::Asset(AssetManager* assetManager, AssetHandle handle)
	: m_assetManager(assetManager), m_handle(handle)
{
}
}

namespace YAML
{
Node convert<Prism::AssetHandle>::encode(const Prism::AssetHandle& rhs)
{
    return Node(std::string(rhs));
}

bool convert<Prism::AssetHandle>::decode(const Node& node, Prism::AssetHandle& rhs)
{
    rhs = Prism::AssetHandle(node.as<std::string>());
    return true;
}
}
