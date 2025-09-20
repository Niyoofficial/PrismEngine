#pragma once
#include "entt/entt.hpp"
#include "SceneRenderPipeline.h"

namespace Prism
{
class Scene : public RefCounted
{
public:
	static Scene* Create(std::wstring name);

	entt::handle AddEntity(std::wstring name);

	entt::registry& GetRegistry() { return m_registry; }
	const entt::registry& GetRegistry() const { return m_registry; }

	void SetRenderPipeline(SceneRenderPipeline* renderPipeline);




private:
	explicit Scene(const std::wstring& name);

	void OnEntityAdded(entt::entity entity);

private:
	entt::registry m_registry;
	Ref<SceneRenderPipeline> m_renderPipeline;
};
}
