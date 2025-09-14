#include "Scene.h"

namespace Prism
{
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
