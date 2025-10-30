#include "MeshRendererComponent.h"

#include "Prism/Render/EntityRenderProxy.h"

namespace Prism
{
MeshRendererComponent::MeshRendererComponent(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode)
	: m_meshAsset(mesh), m_meshNode(meshNode)
{
	PE_ASSERT(mesh && meshNode != -1);
}

void MeshRendererComponent::SetPrimitive(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode meshNode)
{
	PE_ASSERT(mesh && meshNode != -1);

	m_meshAsset = mesh;
	m_meshNode = meshNode;
}

Render::EntityRenderProxy* MeshRendererComponent::CreateRenderProxy(glm::float4x4 transform) const
{
	if (m_meshAsset && m_meshNode != -1 && m_meshAsset->DoesNodeContainVertices(m_meshNode))
	{
		Render::RenderProxyInitInfo initInfo = {
			.wordTransform = transform,
			.bounds = m_meshAsset->GetBoundingBox(m_meshNode),
			.meshAsset = m_meshAsset,
			.meshNode = m_meshNode
		};
		return new Render::EntityRenderProxy(initInfo);
	}

	return nullptr;
}

void MeshRendererComponent::DrawImGuiInspector()
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
	ImGui::Text("Filepath");

	ImGui::TableNextColumn();
	ImGui::Text("%s", WStringToString(m_meshAsset->GetLoadedFilepath()).c_str());

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("Node Index");

	ImGui::TableNextColumn();
	ImGui::PushID("node_index");
	ImGui::DragInt("", &m_meshNode, 0.25f, 0, m_meshAsset->GetTotalNodeCount() - 1, "%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::PopID();

	ImGui::EndTable();
}
}
