#include "PBRMeshRendererComponent.h"

#include "Prism/AssetManagement/AssetManager.h"
#include "Prism/AssetManagement/AssetRegistry.h"
#include "Prism/AssetManagement/AssetType.h"
#include "Prism/Render/PBR/PBREntityRenderProxy.h"

namespace Prism
{
PBRMeshRendererComponent::PBRMeshRendererComponent(MeshAsset* mesh, MeshNode meshNode)
	: MeshRendererComponent(mesh, meshNode)
{
	m_albedoTexture = mesh->GetNodeTexture(meshNode, TextureType::Albedo);
	m_normalTexture = mesh->GetNodeTexture(meshNode, TextureType::Normals);
	m_metallicTexture = mesh->GetNodeTexture(meshNode, TextureType::Metallic);
	m_roughnessTexture = mesh->GetNodeTexture(meshNode, TextureType::Roughness);
	m_emissiveTexture = mesh->GetNodeTexture(meshNode, TextureType::Emissive);
}

void PBRMeshRendererComponent::DrawImGuiInspector()
{
	MeshRendererComponent::DrawImGuiInspector();

	auto drawTextureRow =
		[](std::string name, Ref<TextureAsset>& texture)
		{
			const float buttonSize = 100.f * ImGui::GetIO().FontGlobalScale;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::SetCursorScreenPos({ ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + buttonSize / 2.f - ImGui::GetTextLineHeight() / 2.f });
			ImGui::Text("%s", (name + " Texture").c_str());

			ImGui::TableNextColumn();

			glm::float2 startPos = ImGui::GetCursorScreenPos();

			// Grey background
			ImGui::InvisibleButton((name + "_texture_button").c_str(), { buttonSize, buttonSize });
			glm::float2 buttonRectMin = ImGui::GetItemRectMin();
			glm::float2 buttonRectMax = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(buttonRectMin, buttonRectMax, ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_WindowBg] + glm::float4{ 0.04f }), 1.f);

			// Drag drop target
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(("ASSET_BROWSER_ITEMS_" + TextureAssetType::Get()->GetFileTypeName()).c_str()))
				{
					YAML::Node node = YAML::Load(std::string((char*)payload->Data, (char*)payload->Data + payload->DataSize));
					if (node.IsSequence() && node.size() == 1)
					{
						auto path = node[0].as<std::string>();
						if (!path.empty())
							texture = AssetManager::Get().LoadAsset<TextureAsset>(path);
					}
				}
				ImGui::EndDragDropTarget();
			}

			// Thumbnail
			if (texture && texture->GetRenderResource())
			{
				ImGui::SetCursorScreenPos(startPos);
				ImGui::Image(texture->GetRenderResource(), { buttonSize, buttonSize });
			}

			// Indicator line
			constexpr float INDICATOR_LINE_PADDING = 0.f;
			constexpr float INDICATOR_LINE_WIDTH = 4.f;
			auto indicatorColor = ImGui::GetColorU32(TextureAssetType::Get()->GetAssetIndicatorColor());
			ImGui::GetWindowDrawList()->AddRectFilled({ buttonRectMin.x + INDICATOR_LINE_PADDING, buttonRectMax.y - INDICATOR_LINE_WIDTH }, { buttonRectMax.x - INDICATOR_LINE_PADDING, buttonRectMax.y }, indicatorColor, 1.f);

			ImGui::SameLine();

			// Dropdown
			auto selectedFilename = texture ? texture->GetAssetPath().filename() : "";
			ImGui::PushID((name + "_combo").c_str());
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::BeginCombo("", selectedFilename.string().c_str()))
			{
				for (const auto& file : AssetRegistry::Get().FetchFilesOfType(TextureAssetType::Get()))
				{
					if (ImGui::Selectable(file.filename().string().c_str(), selectedFilename == file.filename()))
						texture = AssetManager::Get().LoadAsset<TextureAsset>(file);
				}

				ImGui::EndCombo();
			}
			ImGui::PopID();

			glm::float2 dropdownRectMin = ImGui::GetItemRectMin();
			glm::float2 dropdownRectMax = ImGui::GetItemRectMax();

			ImGui::SetCursorScreenPos({ dropdownRectMin.x, dropdownRectMax.y + 4.f });
			ImGui::PushID((name + "_RESET_BUTTON").c_str());
			if (ImGui::Button("X"))
				texture = nullptr;
			ImGui::PopID();
		};


	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	ImGui::Text("Albedo");

	ImGui::TableNextColumn();

	ImGui::PushID("ALBEDO_TINT");
	ImGui::ColorEdit3("", &m_albedo.r);
	ImGui::PopID();

	drawTextureRow("Albedo", m_albedoTexture);
	drawTextureRow("Normals", m_normalTexture);

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	ImGui::Text("Metallic");

	ImGui::TableNextColumn();

	ImGui::PushID("METALLIC");
	ImGui::SliderFloat("", &m_metallic, 0.f, 1.f);
	ImGui::PopID();

	drawTextureRow("Metallic", m_metallicTexture);

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	ImGui::Text("Roughness");

	ImGui::TableNextColumn();

	ImGui::PushID("ROUGHNESS");
	ImGui::SliderFloat("", &m_roughness, 0.f, 1.f);
	ImGui::PopID();

	drawTextureRow("Roughness", m_roughnessTexture);
	drawTextureRow("Emissive", m_emissiveTexture);
}

Ref<Render::EntityRenderProxy> PBRMeshRendererComponent::CreateRenderProxy(glm::float4x4 transform) const
{
	if (m_meshAsset && m_meshNode != -1 && m_meshAsset->DoesNodeContainVertices(m_meshNode))
	{
		Render::PBRRenderProxyInitInfo initInfo = {
			.renderProxyInitInfo = {
				.wordTransform = transform,
				.bounds = m_meshAsset->GetBoundingBox(m_meshNode),
				.meshAsset = m_meshAsset,
				.meshNode = m_meshNode
			},
			.albedo = m_albedo,
			.albedoTexture = m_albedoTexture ? m_albedoTexture->GetRenderResource() : nullptr,
			.normalTexture = m_normalTexture ? m_normalTexture->GetRenderResource() : nullptr,
			.metallic = m_metallic,
			.metallicTexture = m_metallicTexture ? m_metallicTexture->GetRenderResource() : nullptr,
			.roughness = m_roughness,
			.roughnessTexture = m_roughnessTexture ? m_roughnessTexture->GetRenderResource() : nullptr,
			.emissiveTexture = m_emissiveTexture ? m_emissiveTexture->GetRenderResource() : nullptr
		};
		return Ref<Render::PBREntityRenderProxy>::Create(initInfo);
	}

	return nullptr;
}
}
