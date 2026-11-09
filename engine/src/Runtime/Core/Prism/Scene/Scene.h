#pragma once
#include "Prism/Render/EntityRenderProxy.h"
#include "Prism/Render/SceneRenderPipeline.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Utilities/Duration.h"

namespace Prism
{
namespace Render
{
class RenderContext;
class Camera;
}

class Scene : public Asset
{
public:
	Scene(AssetManager* assetManager, AssetHandle handle);

	virtual AssetType* GetAssetType() const override;

	Entity CreateEntity(std::string name);
	void RemoveEntity(Entity entity);

	bool IsEntityValid(Entity entity) const;

	void SetEntityName(Entity entity, std::string name);
	std::string GetEntityName(Entity entity) const;

	void SetEntityParent(Entity entity, Entity parent);
	Entity GetEntityParent(Entity entity) const;
	std::vector<Entity> GetEntityChildren(Entity entity) const;
	bool IsRootEntity(Entity entity) const;

	template<typename T, typename... Args> requires std::is_base_of_v<Component, T>
	T* AddComponentToEntity(Args&&... args) requires std::is_base_of_v<Component, T>
	{
		auto comp = Ref<T>::Create(std::forward<Args>(args)...);
		AddComponentToEntity(comp);
		return comp;
	}
	void AddComponentToEntity(Entity entity, Component* component);
	template<typename T>
	bool EntityHasComponent(Entity entity) const requires std::is_base_of_v<Component, T>
	{
		return EntityHasComponent(entity, typeid(T));
	}
	bool EntityHasComponent(Entity entity, const std::type_info& type) const;
	template<typename T>
	T* EntityGetComponent(Entity entity) const requires std::is_base_of_v<Component, T>
	{
		return static_cast<T*>(EntityGetComponent(entity, typeid(T)));
	}
	Component* EntityGetComponent(Entity entity, const std::type_info& type) const;
	template<typename T>
	T* EntityGetComponentChecked(Entity entity) const requires std::is_base_of_v<Component, T>
	{
		return static_cast<T*>(EntityGetComponentChecked(entity, typeid(T)));
	}
	Component* EntityGetComponentChecked(Entity entity, std::type_info* type) const;
	const std::unordered_map<size_t, Ref<Component>>& EntityGetAllComponents(Entity entity) const;
	int64_t GetEntityComponentCount(Entity entity) const;

	// Create an entity hierarchy representing the mesh assset, returns root entity
	Entity CreateEntityHierarchyForMeshAsset(const Ref<MeshAsset>& asset);

	std::vector<Entity> GetAllEntities() const;

	void SetSelectedEntity(Entity entity);
	Entity GetSelectedEntity() const;

	void Update(Duration delta);

	void RenderScene(const Ref<Render::SceneRenderPipeline>& renderPipeline, const Ref<Render::RenderContext>& renderContext, const Ref<Render::
					 TextureView>& rtv, const Ref<Render::Camera>& camera);
	std::vector<Entity> RenderHitProxies(const Ref<Render::SceneRenderPipeline>& renderPipeline,
										 const Ref<Render::RenderContext>& renderContext, const Ref<Render::TextureView>& rtv,
										 const Ref<Render::Camera>& camera);

private:
	void PrepareRenderProxiesForEntity(Entity entity, glm::float4x4 parentTransform);

	virtual void SaveAsset(const std::fs::path& path, YAML::Emitter& metadataEmitter) override;

private:
	int64_t m_nextEntityID = 0;
	struct EntityData
	{
		std::string name;

		Entity parent;
		std::vector<Entity> children;

		std::unordered_map<size_t, Ref<Component>> components;
	};
	std::unordered_map<int64_t, EntityData> m_entities;
	// TODO: Remove this and add something like mesh processors to collect meshes for each pass
	Entity m_selectedEntity;

	std::unordered_map<Ref<Render::EntityRenderProxy>, Entity> m_renderProxies;
	// TODO: Remove this
	WeakRef<Render::EntityRenderProxy> m_selectedProxy;
	Bounds3f m_sceneBounds;
	std::vector<Render::DirectionalLight> m_dirLights;
};
}
