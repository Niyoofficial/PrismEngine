#include "pcpch.h"
#include "PreservingObjectContainer.h"

namespace Prism
{
PreservingObjectContainer::PreservingObjectContainer(PreservingObjectContainer&& other) noexcept
	: m_preservedObjects(std::move(other.m_preservedObjects))
{
	other.m_preservedObjects = {};
}

PreservingObjectContainer& PreservingObjectContainer::operator=(PreservingObjectContainer&& other) noexcept
{
	m_preservedObjects = std::move(other.m_preservedObjects);

	other.m_preservedObjects = {};

	return *this;
}
}
