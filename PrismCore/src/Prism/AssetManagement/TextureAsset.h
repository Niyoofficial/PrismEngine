#pragma once
#include "Prism/AssetManagement/Asset.h"
#include "Prism/Render/Texture.h"

namespace Prism
{
class TextureAsset : public Asset
{
public:
	TextureAsset(AssetManager* assetManager, AssetHandle handle);

	Ref<Render::Texture> GetRenderResource() const { return m_renderTexture; }

    AssetType* GetAssetType() const override;

private:
    void SaveAsset(const std::fs::path& path, YAML::Emitter& emitter) override { PE_ASSERT_NO_ENTRY(); }

    Ref<Render::Texture> m_renderTexture;
};
}
