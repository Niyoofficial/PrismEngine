#include "MeshRendererComponent.h"

#include "Prism/AssetManagement/AssetManager.h"
#include "Prism/Render/EntityRenderProxy.h"

namespace Prism
{
MeshRendererComponent::MeshRendererComponent(Ref<MeshAsset> mesh, MeshNode meshNode)
	: m_meshAsset(mesh), m_meshNode(meshNode)
{
	PE_ASSERT(mesh && meshNode != -1);
}

void MeshRendererComponent::SetPrimitive(Ref<MeshAsset> mesh, MeshNode meshNode)
{
	PE_ASSERT(mesh && meshNode != -1);

	m_meshAsset = mesh;
	m_meshNode = meshNode;
}

Ref<Render::EntityRenderProxy> MeshRendererComponent::CreateRenderProxy(glm::float4x4 transform) const
{
	if (m_meshAsset && m_meshNode != -1 && m_meshAsset->DoesNodeContainVertices(m_meshNode))
	{
		Render::RenderProxyInitInfo initInfo = {
			.wordTransform = transform,
			.bounds = m_meshAsset->GetBoundingBox(m_meshNode),
			.meshAsset = m_meshAsset,
			.meshNode = m_meshNode
		};
		return Ref<Render::EntityRenderProxy>::Create(initInfo);
	}

	return nullptr;
}

void MeshRendererComponent::DrawImGuiInspector()
{
	Component::DrawImGuiInspector();

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("Filepath");

	ImGui::TableNextColumn();
	ImGui::TextWrapped("%s", WStringToString(m_meshAsset->GetLoadedFilepath()).c_str());

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("Node Index");

	ImGui::TableNextColumn();
	ImGui::PushID("node_index");
	ImGui::DragInt("", &m_meshNode, 0.25f, 0, m_meshAsset->GetTotalNodeCount() - 1, "%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::PopID();
}

YAML::Node MeshRendererComponent::ToYAML() const
{
	YAML::Node node;
    node["MeshAssetHandle"] = m_meshAsset->GetHandle();
    node["MeshNode"] = m_meshNode;
    return node;
}

void MeshRendererComponent::FromYAML(const YAML::Node& node)
{
    auto handle = node["MeshAssetHandle"].as<AssetHandle>();
	m_meshAsset = AssetManager::Get().LoadAsset<MeshAsset>(handle);
	m_meshNode = node["MeshNode"].as<MeshNode>();
}
}
