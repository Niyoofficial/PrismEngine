#include "Scene.h"

#include "Prism/Scene/Entity.h"
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

	for (auto& entity : m_entities)
	{
		for(auto& comp : entity->m_components)
		{
			if (auto* renderComp = dynamic_cast<MeshRendererComponent*>(comp.Raw()))
				m_renderProxies.emplace_back(renderComp->CreateRenderProxy());
		}
	}
}

void Scene::RenderScene(Render::TextureView* rtv, Render::Camera* camera)
{
	m_renderPipeline
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
	PE_ASSERT(entity->GetOwningScene(), "Entity is already owned by a different scene!");

	entity->InitializeOwnership(this);
	m_entities.emplace_back(entity);
}

void Scene::SetRenderPipeline(Render::SceneRenderPipeline* renderPipeline)
{
	// TODO: Add a safe way to call this during rendering, some kind of separate
	// variable for next pipeline that will actually be changed after frame is finished
	m_renderPipeline = renderPipeline;
}
}
