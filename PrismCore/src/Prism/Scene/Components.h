#pragma once
#include "Prism/Render/VertexFactory.h"

namespace Prism
{
namespace MeshLoading
{
class MeshAsset;
}

struct TransformComponent
{
public:
	void SetTranslation(glm::float3 translation);
	void SetRotation(glm::quat rotation);
	void SetRotation(glm::float3 eulerRotation);
	void SetScale(glm::float3 scale);

	glm::float4x4 GetTransform() const;
	glm::float3 GetTranslation() const { return m_translation; }
	glm::quat GetRotation() const { return m_rotation; }
	glm::float3 GetScale() const { return m_scale; }

private:
	glm::float3 m_translation = {0.f, 0.f, 0.f};
	glm::quat m_rotation = {};
	glm::float3 m_scale = {1.f, 1.f, 1.f};
};
}
