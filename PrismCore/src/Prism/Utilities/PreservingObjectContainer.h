#pragma once

namespace Prism
{
struct PreservedResourceWrapperBase : public RefCounted {};
template<typename T>
struct PreservedResourceWrapper : public PreservedResourceWrapperBase
{
public:
	explicit PreservedResourceWrapper(T&& specificObject) requires !std::is_lvalue_reference_v<T>
		: m_resource(std::move(specificObject))
	{
	}

private:
	T m_resource = nullptr;
};

class PreservingObjectContainer final
{
public:
	PreservingObjectContainer() = default;
	PreservingObjectContainer(const PreservingObjectContainer&) = delete;
	PreservingObjectContainer(PreservingObjectContainer&& other) noexcept;
	~PreservingObjectContainer() = default;

	PreservingObjectContainer& operator=(const PreservingObjectContainer&) = delete;
	PreservingObjectContainer& operator=(PreservingObjectContainer&& other) noexcept;

	// Objects MUST be std::move'd into this function
	template<typename T>
	void AddObject(T&& object) requires !std::is_lvalue_reference_v<T>
	{
		m_preservedObjects.emplace_back(new PreservedResourceWrapper{std::move(object)});
	}

	void MoveObjects(PreservingObjectContainer& containerToMoveFrom);

	std::vector<Ref<PreservedResourceWrapperBase>>& GetPreservedObjects() { return m_preservedObjects; }
	const std::vector<Ref<PreservedResourceWrapperBase>>& GetPreservedObjects() const { return m_preservedObjects; }

private:
	std::vector<Ref<PreservedResourceWrapperBase>> m_preservedObjects;
};
}
