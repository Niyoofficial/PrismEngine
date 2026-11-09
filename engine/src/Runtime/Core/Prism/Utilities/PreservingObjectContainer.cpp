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

void PreservingObjectContainer::MoveObjects(PreservingObjectContainer& containerToMoveFrom)
{
	m_preservedObjects.insert(m_preservedObjects.end(),
							  std::make_move_iterator(containerToMoveFrom.m_preservedObjects.begin()),
							  std::make_move_iterator(containerToMoveFrom.m_preservedObjects.end()));

	containerToMoveFrom.m_preservedObjects.erase(containerToMoveFrom.m_preservedObjects.begin(),
												 containerToMoveFrom.m_preservedObjects.end());
}
}
