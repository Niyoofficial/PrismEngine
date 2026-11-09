#include "Entity.h"

#include "Prism/Scene/Scene.h"

namespace Prism
{
bool Entity::IsValid() const
{
	return m_scene.IsValid() && m_scene->IsEntityValid(*this);
}

void Entity::SetName(std::string name)
{
	PE_ASSERT(IsValid());
	m_scene->SetEntityName(*this, name);
}

std::string Entity::GetName() const
{
	PE_ASSERT(IsValid());
	return m_scene->GetEntityName(*this);
}

void Entity::SetParent(Entity parent)
{
	PE_ASSERT(IsValid());
	m_scene->SetEntityParent(*this, parent);
}

Entity Entity::GetParent() const
{
	PE_ASSERT(IsValid());
	return m_scene->GetEntityParent(*this);
}

std::vector<Entity> Entity::GetChildren() const
{
	PE_ASSERT(IsValid());
	return m_scene->GetEntityChildren(*this);
}

bool Entity::IsRootEntity() const
{
	PE_ASSERT(IsValid());
	return m_scene->IsRootEntity(*this);
}

void Entity::AddComponent(Component* component)
{
	PE_ASSERT(IsValid());
	return m_scene->AddComponentToEntity(*this, component);
}

int64_t Entity::GetComponentCount() const
{
	PE_ASSERT(IsValid());
	return m_scene->GetEntityComponentCount(*this);
}

bool Entity::HasComponent(const std::type_info& type) const
{
	PE_ASSERT(IsValid());
	return m_scene->EntityHasComponent(*this, type);
}

Component* Entity::GetComponent(const std::type_info& type) const
{
	PE_ASSERT(IsValid());
	return m_scene->EntityGetComponent(*this, type);
}

Component* Entity::GetComponentChecked(std::type_info* type) const
{
	PE_ASSERT(IsValid());
	return m_scene->EntityGetComponentChecked(*this, type);
}

const std::unordered_map<size_t, Ref<Component>>& Entity::GetAllComponents() const
{
	PE_ASSERT(IsValid());
	return m_scene->EntityGetAllComponents(*this);
}

Entity::Entity(int64_t id, Scene* scene)
	: m_ID(id), m_scene(scene)
{
}
}
