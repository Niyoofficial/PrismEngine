#pragma once
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Scene/MeshRendererComponent.h"

namespace Prism
{
DECLARE_COMPONENT(PBRMeshRendererComponent, MeshRendererComponent)
{
public:
	PBRMeshRendererComponent(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode);

	virtual std::wstring GetComponentName() const override { return L"PBR Mesh Renderer Component"; }
	virtual void DrawImGuiInspector() override;
	virtual Ref<Render::EntityRenderProxy> CreateRenderProxy(glm::float4x4 transform) const override;

protected:
	Ref<TextureAsset> m_albedo;
	Ref<TextureAsset> m_normals;
	Ref<TextureAsset> m_metallic;
	Ref<TextureAsset> m_roughness;
	Ref<TextureAsset> m_emissive;
};
}
