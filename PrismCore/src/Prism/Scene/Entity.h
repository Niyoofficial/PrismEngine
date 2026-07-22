#pragma once
#include "Prism/Scene/Components.h"

namespace Prism
{
struct Entity final
{
	friend class Scene;
public:
	Entity() = default;

	bool IsValid() const;

	void SetName(std::string name);
	std::string GetName() const;

	void SetParent(Entity parent);
	Entity GetParent() const;
	std::vector<Entity> GetChildren() const;
	bool IsRootEntity() const;

	void AddComponent(Component* component);

	template<typename T, typename... Args> requires std::is_base_of_v<Component, T>
	T* AddComponent(Args&&... args) requires std::is_base_of_v<Component, T>
	{
		auto comp = Ref<T>::Create(std::forward<Args>(args)...);
		AddComponent(comp);
		return comp;
	}

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
	const std::unordered_map<size_t, Ref<Component>>& GetAllComponents() const;

	Scene* GetOwningScene() const { return m_scene.Raw(); }

	int64_t GetID() const { return m_ID; }

	explicit operator int64_t() { return GetID(); } 
	explicit operator bool() const { return IsValid(); }
	auto operator<=>(const Entity&) const = default;

private:
	Entity(int64_t id, Scene* scene);

protected:
	int64_t m_ID = -1;

	WeakRef<Scene> m_scene;
};
}

namespace std
{
template<>
struct hash<Prism::Entity>
{
	size_t operator()(const Prism::Entity& entity) const noexcept
	{
		return std::hash<int64_t>{}(entity.GetID());
	}
};
}
