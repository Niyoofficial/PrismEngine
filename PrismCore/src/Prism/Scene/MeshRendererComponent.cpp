#include "MeshRendererComponent.h"

#include "Prism/Render/EntityRenderProxy.h"
#include "Prism/Scene/Entity.h"

namespace Prism
{
MeshRendererComponent::MeshRendererComponent(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode)
	: m_meshAsset(mesh), m_meshNode(meshNode)
{
	PE_ASSERT(mesh && meshNode != -1);
}

void MeshRendererComponent::SetPrimitive(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode)
{
	PE_ASSERT(mesh && meshNode != -1);

	m_meshAsset = mesh;
	m_meshNode = meshNode;
}

Render::EntityRenderProxy* MeshRendererComponent::CreateRenderProxy(glm::float4x4 transform) const
{
	if (m_meshAsset && m_meshNode != -1 && m_meshAsset->DoesNodeContainVertices(m_meshNode))
	{
		Render::RenderProxyInitInfo initInfo = {
			.wordTransform = transform,
			.bounds = m_meshAsset->GetBoundingBox(m_meshNode),
			.meshAsset = m_meshAsset,
			.meshNode = m_meshNode
		};
		return new Render::EntityRenderProxy(initInfo);
	}

	return nullptr;
}
}
