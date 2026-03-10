#pragma once
#include "Prism/AssetManagement/Asset.h"
#include "Prism/Render/Texture.h"

namespace Prism
{
class TextureAsset : public Asset
{
public:
	TextureAsset(AssetManager* assetManager, std::fs::path path);

	Ref<Render::Texture> GetRenderResource() const { return m_renderTexture; }

private:
	Ref<Render::Texture> m_renderTexture;
};
}
