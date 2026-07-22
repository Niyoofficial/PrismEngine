#include "TextureAsset.h"

#include "Prism/AssetManagement/AssetManager.h"

namespace Prism
{
TextureAsset::TextureAsset(AssetManager* assetManager, AssetHandle handle)
	: Asset(assetManager, handle), m_renderTexture(Render::Texture::CreateFromFile(assetManager->GetPathFromHandle(handle)))
{
}

AssetType* TextureAsset::GetAssetType() const
{
    return TextureAssetType::Get();
}
}
