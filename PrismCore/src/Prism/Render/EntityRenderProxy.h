#pragma once
#include "Prism/Utilities/MeshLoading.h"

namespace Prism::Render
{
struct RenderProxyInitInfo
{
	glm::float4x4 wordTransform;
	Bounds3f bounds;

	MeshLoading::MeshAsset* meshAsset;
	MeshLoading::MeshNode meshNode;
};

class EntityRenderProxy : public RefCounted
{
public:
	explicit EntityRenderProxy(const RenderProxyInitInfo& initInfo);

	glm::float4x4 GetWorldTransform() const { return m_worldTransform; }
	Bounds3f GetBounds() const { return m_bounds; }
	MeshLoading::MeshAsset* GetMeshAsset() const { return m_meshAsset; }
	MeshLoading::MeshNode GetMeshNode() const { return m_meshNode; }

protected:
	glm::float4x4 m_worldTransform = {1.f};
	Bounds3f m_bounds;
	Ref<MeshLoading::MeshAsset> m_meshAsset;
	MeshLoading::MeshNode m_meshNode;
};
}
