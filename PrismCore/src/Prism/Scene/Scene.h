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
	explicit Scene(const std::wstring& name);

	void AddEntity(Ref<Entity>& entity);
	template<typename T = Entity, typename... Args>
	Ref<T> AddEntity(Args&&... args)
	{
		auto entity = Ref<T>::Create(std::forward<Args>(args)...);
		AddEntity(entity);
		return entity;
	}

	void RemoveEntity(const Ref<Entity>& entity);

	// Create an entity hierarchy representing the mesh assset, returns root entity
	Ref<Entity> CreateEntityHierarchyForMeshAsset(const Ref<MeshLoading::MeshAsset>& asset);

	template<typename T, typename... Args> requires std::is_base_of_v<Render::SceneRenderPipeline, T>
	void SetRenderPipeline(Args&&... args)
	{
		SetRenderPipeline(Ref<T>::Create(std::forward<Args>(args)...));
	}
	// Scene will take ownership of the render pipeline
	void SetRenderPipeline(const Ref<Render::SceneRenderPipeline>& renderPipeline);

	const std::wstring& GetSceneName() const { return m_sceneName; }

	const std::vector<Ref<Entity>>& GetAllEntities() const;

	Ref<Render::SceneRenderPipeline> GetCurrentRenderPipeline() const { return m_renderPipeline; }

	void SetSelectedEntity(const Ref<Entity>& entity);
	Entity* GetSelectedEntity() const;


	void Update(Duration delta);

	void RenderScene(Render::RenderContext* renderContext, Render::TextureView* rtv, Render::Camera* camera);
	std::vector<Ref<Entity>> RenderHitProxies(const Ref<Render::RenderContext>& renderContext, const Ref<Render::TextureView>& rtv, const Ref<Render::Camera>& camera);

private:
	void PrepareRenderProxiesForEntity(const Ref<Entity>& entity, glm::float4x4 parentTransform);

private:
	std::wstring m_sceneName;

	Ref<Render::SceneRenderPipeline> m_renderPipeline;

	std::vector<Ref<Entity>> m_entities;
	// TODO: Remove this and add something like mesh processors to collect meshes for each pass
	WeakRef<Entity> m_selectedEntity;

	std::unordered_map<Ref<Render::EntityRenderProxy>, WeakRef<Entity>> m_renderProxies;
	// TODO: Remove this
	WeakRef<Render::EntityRenderProxy> m_selectedProxy;
	Bounds3f m_sceneBounds;
	std::vector<Render::DirectionalLight> m_dirLights;
};
}
