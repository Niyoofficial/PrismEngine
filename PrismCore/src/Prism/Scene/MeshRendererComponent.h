#pragma once
#include "Prism/Scene/Components.h"
#include "Prism/Utilities/MeshLoading.h"

namespace Prism
{
namespace Render
{
class EntityRenderProxy;
}
namespace MeshLoading
{
class MeshAsset;
}

DECLARE_COMPONENT(MeshRendererComponent, Component)
{
public:
	MeshRendererComponent() = default;
	MeshRendererComponent(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode);

	virtual std::wstring GetComponentName() const override { return L"Mesh Renderer Component"; }

	void SetPrimitive(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode);

	virtual Ref<Render::EntityRenderProxy> CreateRenderProxy(glm::float4x4 transform) const;

	virtual void DrawImGuiInspector() override;

protected:
	Ref<MeshLoading::MeshAsset> m_meshAsset;
	MeshLoading::MeshNode m_meshNode = -1;
};
}
