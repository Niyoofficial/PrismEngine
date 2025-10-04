#include "Entity.h"

namespace Prism
{
Entity::Entity(const std::wstring& name)
	: m_name(name)
{
}

void Entity::AddComponent(Component* component)
{
	PE_ASSERT(component);
	PE_ASSERT(component->GetParent() == nullptr, "Component already has a parent!");

	component->InitializeOwnership(this);
	m_components[typeid(*component).hash_code()] = component;
}

int64_t Entity::GetComponentCount() const
{
	return (int64_t)m_components.size();
}

void Entity::InitializeOwnership(Scene* scene)
{
	m_scene = scene;
}
}
