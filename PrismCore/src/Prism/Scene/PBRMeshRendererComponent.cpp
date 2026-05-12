#include "PBRMeshRendererComponent.h"

#include "Prism/AssetManagement/AssetManager.h"
#include "Prism/AssetManagement/AssetType.h"
#include "Prism/Render/PBR/PBREntityRenderProxy.h"

namespace Prism
{
PBRMeshRendererComponent::PBRMeshRendererComponent(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode)
	: MeshRendererComponent(mesh, meshNode)
{
}

void PBRMeshRendererComponent::DrawImGuiInspector()
{
	MeshRendererComponent::DrawImGuiInspector();

	auto drawTextureRow =
		[](std::string name, Ref<TextureAsset>& texture)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(name.c_str());

			ImGui::TableNextColumn();

			glm::float2 startPos = ImGui::GetCursorScreenPos();

			// Grey background
			const float buttonSize = 120.f * ImGui::GetIO().FontGlobalScale;
			ImGui::InvisibleButton((name + "_texture_button").c_str(), { buttonSize, buttonSize });
			glm::float2 buttonRectMin = ImGui::GetItemRectMin();
			glm::float2 buttonRectMax = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(buttonRectMin, buttonRectMax, ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_WindowBg] + glm::float4{ 0.04f }));

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
			constexpr float INDICATOR_LINE_PADDING = 4.f;
			constexpr float INDICATOR_LINE_WIDTH = 4.f;
			auto indicatorColor = ImGui::GetColorU32(TextureAssetType::Get()->GetAssetIndicatorColor());
			ImGui::GetWindowDrawList()->AddRectFilled({ buttonRectMin.x + INDICATOR_LINE_PADDING, buttonRectMax.y - INDICATOR_LINE_WIDTH }, { buttonRectMax.x - INDICATOR_LINE_PADDING, buttonRectMax.y }, indicatorColor);
		};

	drawTextureRow("Albedo", m_albedo);
	drawTextureRow("Normals", m_normals);
	drawTextureRow("Metallic", m_metallic);
	drawTextureRow("Roughness", m_roughness);
	drawTextureRow("Emissive", m_emissive);
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
			.albedo = m_albedo ? m_albedo->GetRenderResource() : nullptr
		};
		return Ref<Render::PBREntityRenderProxy>::Create(initInfo);
	}

	return nullptr;
}
}
