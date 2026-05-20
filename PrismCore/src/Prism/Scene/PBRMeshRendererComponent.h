#pragma once
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Scene/MeshRendererComponent.h"

namespace Prism
{
DECLARE_COMPONENT(PBRMeshRendererComponent, MeshRendererComponent)
{
public:
	PBRMeshRendererComponent(MeshAsset* mesh, MeshNode meshNode);

	virtual std::wstring GetComponentName() const override { return L"PBR Mesh Renderer Component"; }
	virtual void DrawImGuiInspector() override;
	virtual Ref<Render::EntityRenderProxy> CreateRenderProxy(glm::float4x4 transform) const override;

protected:
	glm::float3 m_albedo = {1.f, 1.f, 1.f};
	Ref<TextureAsset> m_albedoTexture;
	Ref<TextureAsset> m_normalTexture;
	float m_metallic = 1.f;
	Ref<TextureAsset> m_metallicTexture;
	float m_roughness = 1.f;
	Ref<TextureAsset> m_roughnessTexture;
	Ref<TextureAsset> m_emissiveTexture;
};
}
