#pragma once
#include "Prism/AssetManagement/MeshAsset.h"

namespace Prism::Render
{
struct RenderProxyInitInfo
{
	glm::float4x4 wordTransform;
	Bounds3f bounds;

	MeshAsset* meshAsset;
	MeshNode meshNode;
};

class EntityRenderProxy : public RefCounted
{
public:
	explicit EntityRenderProxy(const RenderProxyInitInfo& initInfo);

	glm::float4x4 GetWorldTransform() const { return m_worldTransform; }
	Bounds3f GetBounds() const { return m_bounds; }
	MeshAsset* GetMeshAsset() const { return m_meshAsset; }
	MeshNode GetMeshNode() const { return m_meshNode; }

protected:
	glm::float4x4 m_worldTransform = {1.f};
	Bounds3f m_bounds;
	Ref<MeshAsset> m_meshAsset;
	MeshNode m_meshNode;
};
}
