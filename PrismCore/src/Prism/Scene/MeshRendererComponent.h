#pragma once
#include "Prism/Base/AppEvents.h"
#include "Prism/Base/AppEvents.h"
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

	void SetPrimitive(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode);

	virtual Render::EntityRenderProxy* CreateRenderProxy(glm::float4x4 transform) const;

private:
	Ref<MeshLoading::MeshAsset> m_meshAsset;
	MeshLoading::MeshNode m_meshNode = -1;
};
}
