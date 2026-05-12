#include "PBREntityRenderProxy.h"

namespace Prism::Render
{
PBREntityRenderProxy::PBREntityRenderProxy(const PBRRenderProxyInitInfo& initInfo)
	: EntityRenderProxy(initInfo.renderProxyInitInfo),
	  m_albedo(initInfo.albedo),
	  m_normals(initInfo.normals),
	  m_metallic(initInfo.metallic),
	  m_roughness(initInfo.roughness),
	  m_emissive(initInfo.emissive)
{
}

Ref<Texture> PBREntityRenderProxy::GetTexture(MeshLoading::TextureType type) const
{
	switch (type)
	{
	case MeshLoading::TextureType::Albedo:
		return m_albedo;
	case MeshLoading::TextureType::Normals:
		return m_normals;
	case MeshLoading::TextureType::Metallic:
		return m_metallic;
	case MeshLoading::TextureType::Roughness:
		return m_roughness;
	case MeshLoading::TextureType::Emissive:
		return m_emissive;
	}

	return {};
}
}
