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

	void AddComponent(Component* component);

	template<typename T, typename... Args> requires std::is_base_of_v<Component, T>
	T* AddComponent(Args&&... args)
	{
		T* comp = new T(std::forward<Args>(args)...);
		AddComponent(comp);
		return comp;
	}

	Scene* GetOwningScene() const { return m_scene; }
	int64_t GetComponentCount() const;
	template<typename T>
	bool HasComponent() const
	{
		return m_components.contains(typeid(T).hash_code());
	}
	template<typename T>
	T* GetComponent() const
	{
		if (m_components.contains(typeid(T).hash_code()))
			return static_cast<T*>(m_components.at(typeid(T).hash_code()).Raw());
		return nullptr;
	}
	template<typename T>
	T* GetComponentChecked() const
	{
		return static_cast<T*>(m_components.at(typeid(T).hash_code()).Raw());
	}
	const std::unordered_map<size_t, Ref<Component>>& GetAllComponents() const { return m_components; }

	void SetName(const std::wstring& name) { m_name = name; }
	std::wstring GetName() const { return m_name; }

protected:
	// Used by the Scene class to initialize the parent
	void InitializeOwnership(Scene* scene);

protected:
	std::wstring m_name;
	Ref<Scene> m_scene;

	// TODO: Replace ref with unique_ptr equivalent
	std::unordered_map<size_t, Ref<Component>> m_components;
};
}
