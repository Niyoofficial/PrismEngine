#include "PBREntityRenderProxy.h"

namespace Prism::Render
{
PBREntityRenderProxy::PBREntityRenderProxy(const PBRRenderProxyInitInfo& initInfo)
	: EntityRenderProxy(initInfo.renderProxyInitInfo),
	  m_albedo(initInfo.albedo),
	  m_albedoTexture(initInfo.albedoTexture),
	  m_normalTexture(initInfo.normalTexture),
	  m_metallic(initInfo.metallic),
	  m_metallicTexture(initInfo.metallicTexture),
	  m_roughness(initInfo.roughness),
	  m_roughnessTexture(initInfo.roughnessTexture),
	  m_emissiveTexture(initInfo.emissiveTexture)
{
}

Ref<Texture> PBREntityRenderProxy::GetTexture(TextureType type) const
{
	switch (type)
	{
	case TextureType::Albedo:
		return m_albedoTexture;
	case TextureType::Normals:
		return m_normalTexture;
	case TextureType::Metallic:
		return m_metallicTexture;
	case TextureType::Roughness:
		return m_roughnessTexture;
	case TextureType::Emissive:
		return m_emissiveTexture;
	}

	return {};
}
}
