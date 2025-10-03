#include "Scene.h"

#include "Prism/Render/Camera.h"
#include "Prism/Render/TextureView.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/LightRendererComponent.h"
#include "Prism/Scene/MeshRendererComponent.h"

namespace Prism
{
int64_t Scene::GetEntityCount() const
{
	return (int64_t)m_entities.size();
}

Entity* Scene::GetEntityByIndex(int64_t index) const
{
	return m_entities.at(index);
}

void Scene::Update(Duration delta)
{
	m_renderProxies.clear();
	m_dirLights.clear();

	for (auto& entity : m_entities)
	{
		glm::float4x4 transfrom(1.f);
		if (auto* comp = entity->GetComponent<TransformComponent>())
			transfrom = comp->GetTransform();

		if (auto* comp = entity->GetComponent<MeshRendererComponent>())
		{
			if (auto* proxy = comp->CreateRenderProxy())
				m_renderProxies.emplace_back(proxy);
		}
		if (auto* comp = entity->GetComponent<LightRendererComponent>())
			m_dirLights.emplace_back(glm::rotate(glm::quat(transfrom), glm::float3{1.f, 0.f, 0.f}), comp->GetColor() * comp->GetIntensity());
	}
}

void Scene::RenderScene(Render::TextureView* rtv, Render::Camera* camera)
{
	Render::RenderInfo renderInfo = {
		.renderTarget = rtv,

		.view = camera->GetViewMatrix(),
		.invView = camera->GetInvViewMatrix(),
		.proj = camera->GetProjectionMatrix(),
		.invProj = camera->GetInvProjectionMatrix(),
		.viewProj = camera->GetViewProjectionMatrix(),
		.invViewProj = camera->GetInvViewProjectionMatrix(),
	};
	renderInfo.proxies = std::move(m_renderProxies);
	renderInfo.directionalLights = m_dirLights;

	m_renderPipeline->Render(renderInfo);
}

Scene::Scene(const std::wstring& name)
	: m_sceneName(name)
{
}

Scene* Scene::Create(std::wstring name)
{
	return new Scene(name);
}

void Scene::AddEntity(Entity* entity)
{
	PE_ASSERT(entity);
	PE_ASSERT(!entity->GetOwningScene(), "Entity is already owned by a different scene!");

	entity->InitializeOwnership(this);
	m_entities.emplace_back(entity);
}

Entity* Scene::CreateEntityHierarchyForMeshAsset(MeshLoading::MeshAsset* asset)
{
	PE_ASSERT(asset);

	Entity* root = nullptr;
	std::function<void(MeshLoading::MeshNode)> processNode =
		[this, &processNode, asset, &root](MeshLoading::MeshNode node)
		{
			Entity* entity = AddEntity();
			entity->AddComponent<MeshRendererComponent>(asset, node);

			if (!root)
				root = entity;

			for (int32_t i = 0; i < asset->GetNodeChildrenCount(node); ++i)
				processNode(asset->GetNodeChild(node, i));
		};

	processNode(asset->GetRootNode());

	return root;
}

void Scene::SetRenderPipeline(Render::SceneRenderPipeline* renderPipeline)
{
	// TODO: Add a safe way to call this during rendering, some kind of separate
	// variable for next pipeline that will actually be changed after frame is finished
	m_renderPipeline = renderPipeline;
}
}
