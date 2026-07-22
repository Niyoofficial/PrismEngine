#include "Scene.h"

#include "Prism/AssetManagement/AssetManager.h"
#include "Prism/AssetManagement/AssetRegistry.h"
#include "Prism/AssetManagement/AssetType.h"
#include "Prism/Render/Camera.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/LightRendererComponent.h"
#include "Prism/Scene/PBRMeshRendererComponent.h"

namespace Prism
{
Scene::Scene(AssetManager* assetManager, AssetHandle handle)
	: Asset(assetManager, handle)
{
	auto path = assetManager->GetPathFromHandle(handle);
	if (!path.empty())
	{
		int64_t maxId = 0;

		auto node = YAML::LoadFile(AssetRegistry::Get().GetAbsPath(path).string());
		for (const auto& entityNode : node["Entities"])
		{
			auto entityId = entityNode.first.as<int64_t>();
			maxId = std::max(maxId, entityId);
			m_entities[entityId] = {
				.name = entityNode.second["Name"].as<std::string>()
			};

			for (const auto& componentNode : entityNode.second["Components"])
			{
				auto comp = ComponentRegistry::Get().CreateComponentFromHash(componentNode.first.as<size_t>());
				comp->FromYAML(componentNode.second);
				auto entity = Entity(entityId, this);
				entity.AddComponent(comp);
			}
		}
		for (const auto& entityNode : node["Entities"])
		{
			auto parentId = entityNode.second["Parent"].as<int64_t>();
			if (parentId >= 0)
			{
				auto entityId = entityNode.first.as<int64_t>();
				auto entity = Entity(entityId, this);
				entity.SetParent(Entity(parentId, this));
			}
		}

		m_nextEntityID = maxId + 1;
	}
}

AssetType* Scene::GetAssetType() const
{
	return SceneAssetType::Get();
}

Entity Scene::CreateEntity(std::string name)
{
	auto id = m_nextEntityID++;
	m_entities[id] = EntityData{
		.name = name
	};

	return {id, this};
}

void Scene::RemoveEntity(Entity entity)
{
	PE_ASSERT(IsEntityValid(entity));

	// This will either be a valid parent invalid entity, both behaviours are valid since
	// passing nothing to SetParent simply makes it a root entity
	Entity parent = entity.GetParent();
	for (auto child : entity.GetChildren())
		child.SetParent(parent);
	entity.SetParent({});

	m_entities.erase(entity.m_ID);
}

bool Scene::IsEntityValid(Entity entity) const
{
	return entity.m_ID >= 0 && entity.m_scene == this && m_entities.contains(entity.m_ID);
}

void Scene::SetEntityName(Entity entity, std::string name)
{
	PE_ASSERT(IsEntityValid(entity));
	m_entities.at(entity.m_ID).name = name;
}

std::string Scene::GetEntityName(Entity entity) const
{
	PE_ASSERT(IsEntityValid(entity));
	return m_entities.at(entity.m_ID).name;
}

void Scene::SetEntityParent(Entity entity, Entity parent)
{
	PE_ASSERT(IsEntityValid(entity));

	auto& entityData = m_entities.at(entity.m_ID);
	if (auto prevParent = entityData.parent)
		std::erase(m_entities.at(prevParent.m_ID).children, entity);

	entityData.parent = parent;

	if (parent)
		m_entities.at(parent.m_ID).children.push_back(entity);
}

Entity Scene::GetEntityParent(Entity entity) const
{
	PE_ASSERT(IsEntityValid(entity));
	return m_entities.at(entity.m_ID).parent;
}

std::vector<Entity> Scene::GetEntityChildren(Entity entity) const
{
	PE_ASSERT(IsEntityValid(entity));
	return m_entities.at(entity.m_ID).children;
}

bool Scene::IsRootEntity(Entity entity) const
{
	PE_ASSERT(IsEntityValid(entity));
	return !m_entities.at(entity.m_ID).parent.IsValid();
}

void Scene::AddComponentToEntity(Entity entity, Component* component)
{
	PE_ASSERT(IsEntityValid(entity));
	PE_ASSERT(component);
	PE_ASSERT(component->GetParentId() == -1, "Component already has a parent!");

	component->InitializeOwnership(entity.m_ID);
	m_entities.at(entity.m_ID).components[typeid(*component).hash_code()] = component;
}

bool Scene::EntityHasComponent(Entity entity, const std::type_info& type) const
{
	PE_ASSERT(IsEntityValid(entity));

	if (m_entities.at(entity.m_ID).components.contains(type.hash_code()))
	{
		return true;
	}
	else
	{
		for (const auto* derived : ComponentRegistry::Get().GetDirectlyDerived(type))
		{
			if (EntityHasComponent(entity, *derived))
				return true;
		}
	}
	return false;
}

Component* Scene::EntityGetComponent(Entity entity, const std::type_info& type) const
{
	PE_ASSERT(IsEntityValid(entity));

	if (m_entities.at(entity.m_ID).components.contains(type.hash_code()))
	{
		return m_entities.at(entity.m_ID).components.at(type.hash_code()).Raw();
	}
	else
	{
		for (const auto* derived : ComponentRegistry::Get().GetDirectlyDerived(type))
		{
			if (auto* comp = EntityGetComponent(entity, *derived))
				return comp;
		}
	}
	return nullptr;
}

Component* Scene::EntityGetComponentChecked(Entity entity, std::type_info* type) const
{
	auto* comp = EntityGetComponent(entity, *type);
	PE_ASSERT(comp);
	return comp;
}

const std::unordered_map<size_t, Ref<Component>>& Scene::EntityGetAllComponents(Entity entity) const
{
	PE_ASSERT(IsEntityValid(entity));
	return m_entities.at(entity.m_ID).components;
}

int64_t Scene::GetEntityComponentCount(Entity entity) const
{
	PE_ASSERT(IsEntityValid(entity));
	return (int64_t)m_entities.at(entity.m_ID).components.size();
}

Entity Scene::CreateEntityHierarchyForMeshAsset(const Ref<MeshAsset>& asset)
{
	PE_ASSERT(asset);

	Entity root;
	std::function<void(MeshNode, Entity)> processNode =
		[this, &processNode, asset, &root](MeshNode node, Entity parent)
	{
		auto entity = CreateEntity(asset->GetNodeName(node));
		if (parent)
			entity.SetParent(parent);

		if (!root)
			root = entity;

		entity.AddComponent<TransformComponent>();

		if (asset->GetNodeChildrenCount(node) == 1)
		{
			auto nodeToRender = asset->GetNodeChild(node, 0);
			if (asset->DoesNodeContainVertices(nodeToRender))
				entity.AddComponent<PBRMeshRendererComponent>(asset, nodeToRender);
		}
		else
		{
			if (asset->DoesNodeContainVertices(node))
				entity.AddComponent<PBRMeshRendererComponent>(asset, node);
			for (int32_t i = 0; i < asset->GetNodeChildrenCount(node); ++i)
				processNode(asset->GetNodeChild(node, i), entity);
		}
	};

	processNode(asset->GetRootNode(), {});

	return root;
}

std::vector<Entity> Scene::GetAllEntities() const
{
	std::vector<Entity> entities;
	for (auto id : m_entities | std::views::keys)
		entities.push_back(Entity(id, const_cast<Scene*>(this)));
	return entities;
}

void Scene::SetSelectedEntity(Entity entity)
{
	m_selectedEntity = entity;
}

Entity Scene::GetSelectedEntity() const
{
	return m_selectedEntity;
}

void Scene::Update(Duration delta)
{
	m_renderProxies.clear();
	m_dirLights.clear();
	m_selectedProxy = nullptr;

	for (auto id : m_entities | std::views::keys)
	{
		auto entity = Entity(id, this);
		if (entity.IsRootEntity())
			PrepareRenderProxiesForEntity(entity, {1.f});
	}
}

void Scene::RenderScene(const Ref<Render::SceneRenderPipeline>& renderPipeline, const Ref<Render::RenderContext>& renderContext,
						const Ref<Render::TextureView>& rtv, const Ref<Render::Camera>& camera)
{
	Render::RenderSceneInfo renderInfo = {
		.renderTargetView = rtv,

		.cameraInfo = camera->GetCameraInfo(),

		.sceneBounds = m_sceneBounds
	};
	for (auto& [proxy, entity] : m_renderProxies)
		renderInfo.proxies.emplace_back(proxy);
	renderInfo.selectedProxy = m_selectedProxy.Raw();
	renderInfo.directionalLights = m_dirLights;

	renderPipeline->Render(renderContext, renderInfo);
}

std::vector<Entity> Scene::RenderHitProxies(const Ref<Render::SceneRenderPipeline>& renderPipeline,
											const Ref<Render::RenderContext>& renderContext,
											const Ref<Render::TextureView>& rtv, const Ref<Render::Camera>& camera)
{
	Render::RenderHitProxiesInfo renderInfo = {
		.renderTargetView = rtv,
		.cameraInfo = camera->GetCameraInfo(),
	};

	std::vector<Entity> hitProxyEntities;
	for (auto& [proxy, entity] : m_renderProxies)
	{
		PE_ASSERT(entity.IsValid());
		renderInfo.proxies.emplace_back(proxy);
		hitProxyEntities.emplace_back(entity);
	}

	renderPipeline->RenderHitProxies(renderContext, renderInfo);

	return hitProxyEntities;
}

void Scene::PrepareRenderProxiesForEntity(Entity entity, glm::float4x4 parentTransform)
{
	glm::float4x4 transform = parentTransform;
	if (auto* comp = entity.GetComponent<TransformComponent>())
		transform *= comp->GetTransform();

	if (auto* comp = entity.GetComponent<MeshRendererComponent>())
	{
		if (Ref proxy = comp->CreateRenderProxy(transform))
		{
			m_renderProxies.try_emplace(proxy, entity);
			m_sceneBounds += proxy->GetBounds();

			if (m_selectedEntity.IsValid() && m_selectedEntity == entity)
				m_selectedProxy = proxy;
		}
	}
	if (auto* comp = entity.GetComponent<LightRendererComponent>())
	{
		m_dirLights.emplace_back(glm::rotate(glm::quat(transform), glm::float3{1.f, 0.f, 0.f}), comp->GetColor() * comp->GetIntensity());
	}

	for (auto& child : entity.GetChildren())
		PrepareRenderProxiesForEntity(child, transform);
}

void Scene::SaveAsset(const std::fs::path& path, YAML::Emitter& metadataEmitter)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Entities";

	out << YAML::BeginMap;

	for (const auto& [id, entityData] : m_entities)
	{
		auto entity = Entity(id, this);

		out << YAML::Key << id;
		out << YAML::Value;

		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << entityData.name;
		out << YAML::Key << "Parent" << YAML::Value << entityData.parent.m_ID;
		out << YAML::Key << "Components";
		out << YAML::Value;

		out << YAML::BeginMap;
		for (const auto& [typeHash, component] : entity.GetAllComponents())
		{
			out << YAML::Key << typeHash;
			out << YAML::Value << component->ToYAML();
		}
		out << YAML::EndMap; // Component type hash

		out << YAML::EndMap; // "Components"
	}
	out << YAML::EndMap; // Entity id

	out << YAML::EndMap; // "Entities"

	auto savePath = path;
	savePath.replace_extension(".pscene");
	std::ofstream file(AssetRegistry::Get().GetAbsPath(savePath), std::ios::out);
	file.write(out.c_str(), (std::streamsize)out.size());
	file.close();
}
}
