#include "pcpch.h"
#include "Components.h"
#include "Prism/Utilities/MeshLoading.h"

namespace Prism
{
void TransformComponent::SetTranslation(glm::float3 translation)
{
	m_translation = translation;
}

void TransformComponent::SetRotation(glm::quat rotation)
{
	m_rotation = rotation;
}

void TransformComponent::SetRotation(glm::float3 eulerRotation)
{
	m_rotation = glm::quat(eulerRotation);
}

void TransformComponent::SetScale(glm::float3 scale)
{
	m_scale = scale;
}

glm::float4x4 TransformComponent::GetTransform() const
{
	return
		glm::translate(glm::float4x4(1.f), m_translation) *
		glm::toMat4(m_rotation) *
		glm::scale(glm::float4x4(1.f), m_scale);
}
}
