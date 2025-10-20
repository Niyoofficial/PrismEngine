#include "pcpch.h"
#include "Components.h"


namespace Prism
{
void Component::InitializeOwnership(Entity* parent)
{
	PE_ASSERT(m_parent == nullptr);
	m_parent = parent;
}

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

void TransformComponent::DrawImGuiInspector() const
{
	Component::DrawImGuiInspector();

	ImGui::BeginTable("##component_table", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV);
	ImGui::TableSetupColumn("label", 0, 100.f);
	ImGui::TableSetupColumn("value", 0, ImGui::GetContentRegionAvail().x - 100.f);

	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	ImGui::Text("Translation");
	ImGui::TableSetColumnIndex(1);
	glm::float3 test = {};
	ImGui::DragFloat3("test", glm::value_ptr(test));

	ImGui::EndTable();
}
}
