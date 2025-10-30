#include "LightRendererComponent.h"

namespace Prism
{
void LightRendererComponent::SetColor(glm::float3 color)
{
	m_color = color;
}

void LightRendererComponent::SetIntensity(float intensity)
{
	m_intensity = intensity;
}

void LightRendererComponent::DrawImGuiInspector()
{
	Component::DrawImGuiInspector();

	ImGui::BeginTable("##component_table", 2,
		ImGuiTableFlags_SizingStretchSame |
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_ContextMenuInBody |
		ImGuiTableFlags_NoClip);

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("Intensity");

	ImGui::TableNextColumn();
	ImGui::PushID("intensity");
	ImGui::DragFloat("", &m_intensity, 0.01f, 0, FLT_MAX, "%.2f");
	ImGui::PopID();

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("Color");

	ImGui::TableNextColumn();
	ImGui::PushID("color");
	ImGui::ColorEdit3("", glm::value_ptr(m_color), ImGuiColorEditFlags_Float);
	ImGui::PopID();

	ImGui::EndTable();
}
}
