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
	explicit Entity(const std::wstring& name = L"");

	void SetParent(Entity* parent);
	Entity* GetParent() const;
	const std::vector<WeakRef<Entity>>& GetChildren() const;
	bool IsRootEntity() const;

	void AddComponent(Component* component);

	template<typename T, typename... Args> requires std::is_base_of_v<Component, T>
	T* AddComponent(Args&&... args) requires std::is_base_of_v<Component, T>
	{
		auto comp = Ref<T>::Create(std::forward<Args>(args)...);
		AddComponent(comp);
		return comp;
	}

	Scene* GetOwningScene() const { return m_scene; }
	int64_t GetComponentCount() const;
	template<typename T>
	bool HasComponent() const requires std::is_base_of_v<Component, T>
	{
		return HasComponent(typeid(T));
	}
	bool HasComponent(const std::type_info& type) const;
	template<typename T>
	T* GetComponent() const requires std::is_base_of_v<Component, T>
	{
		return static_cast<T*>(GetComponent(typeid(T)));
	}
	Component* GetComponent(const std::type_info& type) const;
	template<typename T>
	T* GetComponentChecked() const requires std::is_base_of_v<Component, T>
	{
		return static_cast<T*>(GetComponentChecked(typeid(T)));
	}
	Component* GetComponentChecked(std::type_info* type) const;
	const std::unordered_map<size_t, Ref<Component>>& GetAllComponents() const { return m_components; }

	void SetName(const std::wstring& name) { m_name = name; }
	std::wstring GetName() const { return m_name; }

protected:
	// Used by the Scene class to initialize the parent scene
	void InitializeOwnership(Scene* scene);

protected:
	std::wstring m_name;
	Ref<Scene> m_scene;

	std::unordered_map<size_t, Ref<Component>> m_components;

	// TODO: Maybe the scene should be storing this info?
	WeakRef<Entity> m_parent;
	std::vector<WeakRef<Entity>> m_children;
};
}
