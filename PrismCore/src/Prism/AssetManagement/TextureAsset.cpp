#include "TextureAsset.h"

#include "Prism/AssetManagement/AssetManager.h"

namespace Prism
{
TextureAsset::TextureAsset(AssetManager* assetManager, std::fs::path path)
	: Asset(assetManager, path), m_renderTexture(Render::Texture::CreateFromFile(assetManager->GetAbsolutePath(path)))
{
}
}
