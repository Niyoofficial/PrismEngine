#include "MeshRendererComponent.h"

#include "Prism/Render/EntityRenderProxy.h"
#include "Prism/Scene/Entity.h"

namespace Prism
{
void MeshRendererComponent::SetPrimitive(MeshLoading::MeshAsset* mesh, int32_t primitiveIndex)
{
	PE_ASSERT(mesh && primitiveIndex > 0);

	m_meshAsset = mesh;
	m_primitiveIndex = primitiveIndex;
}

Render::EntityRenderProxy* MeshRendererComponent::CreateRenderProxy() const
{
	return new Render::EntityRenderProxy(GetParent()->GetOwningScene()->GetCurrentRenderPipeline());
}
}
