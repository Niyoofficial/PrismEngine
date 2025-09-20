#include "Scene.h"

#include "Prism/Scene/MeshRendererComponent.h"

namespace Prism
{
void Scene::SetRenderPipeline(SceneRenderPipeline* renderPipeline)
{
	// TODO: Add a safe way to call this during rendering, some kind of separate
	// variable for next pipeline that will actually be changed after frame is finished
	m_renderPipeline = renderPipeline;
}

void Scene::Update(Duration delta)
{
	for (auto& meshRendererComp : m_registry.storage<MeshRendererComponent>())
	{
		
	}
}

Scene::Scene(const std::wstring& name)
{
	//m_registry.on_construct<entt::entity>().connect<&entt::registry::emplace<>>(this);
	m_registry.on_construct<entt::entity>().connect<&Scene::OnEntityAdded>(this);
}

Scene* Scene::Create(std::wstring name)
{
	return new Scene(name);
}

entt::handle Scene::AddEntity(std::wstring name)
{
	return {m_registry, m_registry.create()};
}

void Scene::OnEntityAdded(entt::entity entity)
{
}
}
