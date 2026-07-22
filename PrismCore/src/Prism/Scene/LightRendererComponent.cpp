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
}

YAML::Node LightRendererComponent::ToYAML() const
{
	YAML::Node node;
    node["Color"] = m_color;
    node["Intensity"] = m_intensity;
	return node;
}

void LightRendererComponent::FromYAML(const YAML::Node& node)
{
    m_color = node["Color"].as<glm::float3>();
    m_intensity = node["Intensity"].as<float>();
}
}
