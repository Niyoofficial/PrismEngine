#pragma once
#include "Prism/Scene/Components.h"

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
	void SetPrimitive(MeshLoading::MeshAsset* mesh, int32_t primitiveIndex);

	virtual Render::EntityRenderProxy* CreateRenderProxy() const;

private:
	Ref<MeshLoading::MeshAsset> m_meshAsset;
	int32_t m_primitiveIndex = -1;
};
}
