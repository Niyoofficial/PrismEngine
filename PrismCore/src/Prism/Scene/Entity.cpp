#include "Entity.h"

namespace Prism
{
Entity::Entity(const std::wstring& name)
	: m_name(name)
{
}

void Entity::SetParent(Entity* parent)
{
	if (m_parent.IsValid())
		std::erase(m_parent->m_children, WeakRef(this));

	m_parent = parent;

	if (parent)
		parent->m_children.emplace_back(this);
}

Entity* Entity::GetParent() const
{
	if (m_parent.IsValid())
		return m_parent.Raw();
	return nullptr;
}

const std::vector<WeakRef<Entity>>& Entity::GetChildren() const
{
	return m_children;
}

bool Entity::IsRootEntity() const
{
	return GetParent() == nullptr;
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

bool Entity::HasComponent(const std::type_info& type) const
{
	if (m_components.contains(type.hash_code()))
	{
		return true;
	}
	else
	{
		for (const auto* derived : ComponentRegistry::Get().GetDirectlyDerived(type))
		{
			if (HasComponent(*derived))
				return true;
		}
	}
	return false;
}

Component* Entity::GetComponent(const std::type_info& type) const
{
	if (m_components.contains(type.hash_code()))
	{
		return m_components.at(type.hash_code()).Raw();
	}
	else
	{
		for (const auto* derived : ComponentRegistry::Get().GetDirectlyDerived(type))
		{
			if (auto* comp = GetComponent(*derived))
				return comp;
		}
	}
	return nullptr;
}

Component* Entity::GetComponentChecked(std::type_info* type) const
{
	auto* comp = GetComponent(*type);
	PE_ASSERT(comp);
	return comp;
}

void Entity::InitializeOwnership(Scene* scene)
{
	m_scene = scene;
}
}
