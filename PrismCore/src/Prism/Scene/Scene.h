#pragma once
#include "Prism/Render/EntityRenderProxy.h"
#include "Prism/Render/SceneRenderPipeline.h"
#include "Prism/Utilities/Duration.h"

namespace Prism
{
namespace Render
{
class Camera;
}

class Entity;

class Scene : public RefCounted
{
public:
	static Scene* Create(std::wstring name);

	void AddEntity(Entity* entity);

	template<typename T = Entity, typename... Args>
	T* AddEntity(Args&&... args)
	{
		T* entity = new T(std::forward<Args>(args)...);
		AddEntity(entity);
		return entity;
	}

	// Create an entity hierarchy representing the mesh assset, returns root entity
	Entity* CreateEntityHierarchyForMeshAsset(MeshLoading::MeshAsset* asset);

	// Scene will take ownership of the render pipeline
	void SetRenderPipeline(Render::SceneRenderPipeline* renderPipeline);

	const std::wstring& GetSceneName() const { return m_sceneName; }

	int64_t GetEntityCount() const;
	Entity* GetEntityByIndex(int64_t index) const;

	Render::SceneRenderPipeline* GetCurrentRenderPipeline() const { return m_renderPipeline; }


	void Update(Duration delta);

	void RenderScene(Render::TextureView* rtv, Render::Camera* camera);

private:
	explicit Scene(const std::wstring& name);

private:
	std::wstring m_sceneName;

	Ref<Render::SceneRenderPipeline> m_renderPipeline;

	// TODO: Replace ref with unique_ptr equivalent
	std::vector<Ref<Entity>> m_entities;

	std::vector<Ref<Render::EntityRenderProxy>> m_renderProxies;
	std::vector<Render::DirectionalLight> m_dirLights;
};
}
