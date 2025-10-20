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

class MeshRendererComponent : public Component
{
public:
	MeshRendererComponent() = default;
	MeshRendererComponent(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode);

	virtual std::wstring GetComponentName() const override { return L"Mesh Renderer Component"; }

	void SetPrimitive(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode);

	virtual Render::EntityRenderProxy* CreateRenderProxy(glm::float4x4 transform) const;

private:
	Ref<MeshLoading::MeshAsset> m_meshAsset;
	MeshLoading::MeshNode m_meshNode = -1;
};
}
