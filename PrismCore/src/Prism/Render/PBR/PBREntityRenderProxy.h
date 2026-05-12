#pragma once
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Render/EntityRenderProxy.h"

namespace Prism::Render
{
struct PBRRenderProxyInitInfo
{
	RenderProxyInitInfo renderProxyInitInfo;
	Ref<Texture> albedo;
	Ref<Texture> normals;
	Ref<Texture> metallic;
	Ref<Texture> roughness;
	Ref<Texture> emissive;
};

class PBREntityRenderProxy : public EntityRenderProxy
{
public:
	explicit PBREntityRenderProxy(const PBRRenderProxyInitInfo& initInfo);

	Ref<Texture> GetTexture(MeshLoading::TextureType type) const;

private:
	Ref<Texture> m_albedo;
	Ref<Texture> m_normals;
	Ref<Texture> m_metallic;
	Ref<Texture> m_roughness;
	Ref<Texture> m_emissive;
};
}
