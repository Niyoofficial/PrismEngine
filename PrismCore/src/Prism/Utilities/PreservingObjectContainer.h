#pragma once

namespace Prism
{
class PreservingObjectContainer final
{
public:
	PreservingObjectContainer() = default;
	PreservingObjectContainer(const PreservingObjectContainer&) = delete;
	PreservingObjectContainer(PreservingObjectContainer&& other) noexcept;
	~PreservingObjectContainer() = default;

	PreservingObjectContainer& operator=(const PreservingObjectContainer&) = delete;
	PreservingObjectContainer& operator=(PreservingObjectContainer&& other) noexcept;

	template<typename T>
	void AddObject(const Ref<T>& object)
	{
		m_preservedObjects.emplace_back(object);
	}

	void MoveObjects(PreservingObjectContainer& containerToMoveFrom);

	std::vector<Ref<RefCounted>>& GetPreservedObjects() { return m_preservedObjects; }
	const std::vector<Ref<RefCounted>>& GetPreservedObjects() const { return m_preservedObjects; }

private:
	std::vector<Ref<RefCounted>> m_preservedObjects;
};
}
