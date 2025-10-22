#pragma once
#include "Prism/Render/EntityRenderProxy.h"
#include "Prism/Render/SceneRenderPipeline.h"
#include "Prism/Utilities/Duration.h"

namespace Prism
{
namespace Render
{
class RenderContext;
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

	const std::vector<Ref<Entity>>& GetAllEntities() const;

	Render::SceneRenderPipeline* GetCurrentRenderPipeline() const { return m_renderPipeline; }

	void SetSelectedEntity(Entity* entity);
	Entity* GetSelectedEntity() const { return m_selectedEntity; }


	void Update(Duration delta);

	void RenderScene(Render::RenderContext* renderContext, Render::TextureView* rtv, Render::Camera* camera);
	std::vector<Entity*> RenderHitProxies(Render::RenderContext* renderContext, Render::TextureView* rtv, Render::Camera* camera);

private:
	explicit Scene(const std::wstring& name);

	void PrepareRenderProxiesForEntity(Entity* entity, glm::float4x4 parentTransform);

private:
	std::wstring m_sceneName;

	Ref<Render::SceneRenderPipeline> m_renderPipeline;

	// TODO: Replace ref with unique_ptr equivalent
	std::vector<Ref<Entity>> m_entities;
	// TODO: Remove this and add something like mesh processors to collect meshes for each pass
	Entity* m_selectedEntity = nullptr;

	std::unordered_map<Ref<Render::EntityRenderProxy>, Entity*> m_renderProxies;
	// TODO: Remove this
	Render::EntityRenderProxy* m_selectedProxy = nullptr;
	Bounds3f m_sceneBounds;
	std::vector<Render::DirectionalLight> m_dirLights;
};
}
