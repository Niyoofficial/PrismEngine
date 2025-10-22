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
	m_eulerRotation = eulerRotation;
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

void TransformComponent::DrawImGuiInspector()
{
	Component::DrawImGuiInspector();

	ImGui::BeginTable("##component_table", 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_NoClip);

	auto drawControlFloat3 =
		[](const char* label, glm::float3& value, float speed)
		{
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text(label);
			ImGui::TableSetColumnIndex(1);

			float valueColumnWidth = ImGui::GetContentRegionAvail().x;
			constexpr int32_t NUMBER_OF_CONTROLS = 3;
			constexpr float CONTROL_SPACING = 3.f;

			auto drawTranslateControl =
				[valueColumnWidth, label](const char* controlLabel, float& value, float speed)
				{
					glm::float2 cursorStartPos = ImGui::GetCursorScreenPos();

					ImGui::AlignTextToFramePadding();
					ImGui::Text(controlLabel);

					float textWidth = ImGui::CalcTextSize(controlLabel).x + CONTROL_SPACING;
					ImGui::SetCursorScreenPos(cursorStartPos + glm::float2{textWidth, 0.f});

					float controlSpacing = (NUMBER_OF_CONTROLS - 1) * CONTROL_SPACING / NUMBER_OF_CONTROLS;
					ImGui::SetNextItemWidth(valueColumnWidth / NUMBER_OF_CONTROLS - textWidth - controlSpacing);

					ImGui::PushID(fmt::format("{}_control_{}", label, controlLabel).c_str());
					bool modified = ImGui::DragFloat("", &value, speed, 0, 0, "%.2f");
					ImGui::PopID();

					return modified;
				};

			bool modified = false;
			modified |= drawTranslateControl("X", value.x, speed);
			ImGui::SameLine(0.f, CONTROL_SPACING);
			modified |= drawTranslateControl("Y", value.y, speed);
			ImGui::SameLine(0.f, CONTROL_SPACING);
			modified |= drawTranslateControl("Z", value.z, speed);

			return modified;
		};

	drawControlFloat3("Translation", m_translation, 0.1f);

	auto rotation = glm::degrees(m_eulerRotation);
	if (drawControlFloat3("Rotation", rotation, 0.1f))
		SetRotation(glm::radians(rotation));

	drawControlFloat3("Scale", m_scale, 0.01f);

	ImGui::EndTable();
}
}
