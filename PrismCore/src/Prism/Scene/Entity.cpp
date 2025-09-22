#include "Entity.h"

namespace Prism
{
void Entity::AddComponent(Component* component)
{
	PE_ASSERT(component);
	PE_ASSERT(component->GetParent() == nullptr, "Component already has a parent!");

	component->InitializeOwnership(this);
	m_components.emplace_back(component);
}

int64_t Entity::GetComponentCount() const
{
	return (int64_t)m_components.size();
}

Component* Entity::GetComponentByIndex(int64_t index) const
{
	return m_components.at(index);
}

void Entity::InitializeOwnership(Scene* scene)
{
	m_scene = scene;
}
}
