#include "Scene.h"

#include "Prism/Render/Camera.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/LightRendererComponent.h"
#include "Prism/Scene/MeshRendererComponent.h"

namespace Prism
{
void Scene::SetSelectedEntity(Entity* entity)
{
	if (entity)
		PE_ASSERT(std::ranges::find(m_entities, Ref(entity)) != m_entities.end());
	m_selectedEntity = entity;
}

void Scene::Update(Duration delta)
{
	m_renderProxies.clear();
	m_dirLights.clear();
	m_selectedProxy = nullptr;

	for (auto& entity : m_entities)
	{
		if (entity->IsRootEntity())
			PrepareRenderProxiesForEntity(entity, {1.f});
	}
}

void Scene::RenderScene(Render::RenderContext* renderContext, Render::TextureView* rtv, Render::Camera* camera)
{
	Render::RenderSceneInfo renderInfo = {
		.renderTargetView = rtv,

		.cameraInfo = camera->GetCameraInfo(),

		.sceneBounds = m_sceneBounds
	};
	for (auto& [proxy, entity] : m_renderProxies)
		renderInfo.proxies.emplace_back(proxy);
	renderInfo.selectedProxy = m_selectedProxy;
	renderInfo.directionalLights = m_dirLights;

	m_renderPipeline->Render(renderContext, renderInfo);
}

std::vector<Entity*> Scene::RenderHitProxies(Render::RenderContext* renderContext, Render::TextureView* rtv, Render::Camera* camera)
{
	Render::RenderHitProxiesInfo renderInfo = {
		.renderTargetView = rtv,
		.cameraInfo = camera->GetCameraInfo(),
	};

	std::vector<Entity*> hitProxyEntities;
	for (auto& [proxy, entity] : m_renderProxies)
	{
		renderInfo.proxies.emplace_back(proxy);
		hitProxyEntities.emplace_back(entity);
	}

	m_renderPipeline->RenderHitProxies(renderContext, renderInfo);

	return hitProxyEntities;
}

Scene::Scene(const std::wstring& name)
	: m_sceneName(name)
{
}

void Scene::PrepareRenderProxiesForEntity(Entity* entity, glm::float4x4 parentTransform)
{
	glm::float4x4 transfrom = parentTransform;
	if (auto* comp = entity->GetComponent<TransformComponent>())
		transfrom *= comp->GetTransform();

	if (auto* comp = entity->GetComponent<MeshRendererComponent>())
	{
		if (Ref proxy = comp->CreateRenderProxy(transfrom))
		{
			m_renderProxies.try_emplace(proxy, entity);
			m_sceneBounds += proxy->GetBounds();

			if (m_selectedEntity == entity)
				m_selectedProxy = proxy;
		}
	}
	if (auto* comp = entity->GetComponent<LightRendererComponent>())
	{
		m_dirLights.emplace_back(glm::rotate(glm::quat(transfrom), glm::float3{ 1.f, 0.f, 0.f }), comp->GetColor() * comp->GetIntensity());
	}

	for (auto& child : entity->GetChildren())
		PrepareRenderProxiesForEntity(child.Raw(), transfrom);
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
	std::function<void(MeshLoading::MeshNode, Entity*)> processNode =
		[this, &processNode, asset, &root](MeshLoading::MeshNode node, Entity* parent)
		{
			Entity* entity = AddEntity(asset->GetNodeName(node));
			if (parent)
				entity->SetParent(parent);

			if (!root)
				root = entity;

			entity->AddComponent<TransformComponent>();

			if (asset->GetNodeChildrenCount(node) == 1)
			{
				auto nodeToRender = asset->GetNodeChild(node, 0);
				if (asset->DoesNodeContainVertices(nodeToRender))
					entity->AddComponent<MeshRendererComponent>(asset, nodeToRender);
			}
			else
			{
				if (asset->DoesNodeContainVertices(node))
					entity->AddComponent<MeshRendererComponent>(asset, node);
				for (int32_t i = 0; i < asset->GetNodeChildrenCount(node); ++i)
					processNode(asset->GetNodeChild(node, i), entity);
			}
		};

	processNode(asset->GetRootNode(), nullptr);

	return root;
}

void Scene::SetRenderPipeline(Render::SceneRenderPipeline* renderPipeline)
{
	// TODO: Add a safe way to call this during rendering, some kind of separate
	// variable for next pipeline that will actually be changed after frame is finished
	m_renderPipeline = renderPipeline;
}

const std::vector<Ref<Entity>>& Scene::GetAllEntities() const
{
	return m_entities;
}
}
