#pragma once
#include "Prism/Scene/Components.h"
#include "Prism/Scene/Scene.h"

namespace Prism
{
// TODO: Maybe this class could the replaced with an index,
// similar to what EnTT does
class Entity : public RefCounted
{
	friend Scene;
public:
	Entity() = default;

	void AddComponent(Component* component);

	template<typename T, typename... Args> requires std::is_base_of_v<T, Component>
	T* AddComponent(Args&&... args)
	{
		T* comp = new T(std::forward<Args>(args)...);
		AddComponent(comp);
		return comp;
	}

	Scene* GetOwningScene() const { return m_scene; }
	int64_t GetComponentCount() const;
	Component* GetComponentByIndex(int64_t index) const;

protected:
	// Used by the Scene class to initialize the parent
	void InitializeOwnership(Scene* scene);

protected:
	Ref<Scene> m_scene;

	// TODO: Replace ref with unique_ptr equivalent
	std::vector<Ref<Component>> m_components;
};
}
