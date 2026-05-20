#pragma once
#include "Prism/AssetManagement/MeshAsset.h"
#include "Prism/Scene/Components.h"


namespace Prism
{
namespace Render
{
class EntityRenderProxy;
}

DECLARE_COMPONENT(MeshRendererComponent, Component)
{
public:
	MeshRendererComponent() = default;
	MeshRendererComponent(Ref<MeshAsset> mesh, MeshNode meshNode);

	virtual std::wstring GetComponentName() const override { return L"Mesh Renderer Component"; }

	void SetPrimitive(Ref<MeshAsset> mesh, MeshNode meshNode);

	virtual Ref<Render::EntityRenderProxy> CreateRenderProxy(glm::float4x4 transform) const;

	virtual void DrawImGuiInspector() override;

protected:
	Ref<MeshAsset> m_meshAsset;
	MeshNode m_meshNode = -1;
};
}
