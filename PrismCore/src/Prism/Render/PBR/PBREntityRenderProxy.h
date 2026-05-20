#pragma once
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Render/EntityRenderProxy.h"

namespace Prism::Render
{
struct PBRRenderProxyInitInfo
{
	RenderProxyInitInfo renderProxyInitInfo;
	glm::float3 albedo = {1.f, 1.f, 1.f};
	Ref<Texture> albedoTexture;
	Ref<Texture> normalTexture;
	float metallic = 1.f;
	Ref<Texture> metallicTexture;
	float roughness = 1.f;
	Ref<Texture> roughnessTexture;
	Ref<Texture> emissiveTexture;
};

class PBREntityRenderProxy : public EntityRenderProxy
{
public:
	explicit PBREntityRenderProxy(const PBRRenderProxyInitInfo& initInfo);

	glm::float3 GetAlbedo() const { return m_albedo; }
	float GetMetallic() const { return m_metallic; }
	float GetRoughness() const { return m_roughness; }
	Ref<Texture> GetTexture(TextureType type) const;

protected:
	glm::float3 m_albedo = {1.f, 1.f, 1.f};
	Ref<Texture> m_albedoTexture;
	Ref<Texture> m_normalTexture;
	float m_metallic = 1.f;
	Ref<Texture> m_metallicTexture;
	float m_roughness = 1.f;
	Ref<Texture> m_roughnessTexture;
	Ref<Texture> m_emissiveTexture;
};
}
