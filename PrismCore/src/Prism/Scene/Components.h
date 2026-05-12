#pragma once

namespace Prism
{
class Entity;

class Component : public RefCounted
{
	friend Entity;
public:
	Component() = default;
	virtual ~Component() = default;

	virtual std::wstring GetComponentName() const = 0;

	Entity* GetParent() const { return m_parent; }

	virtual void DrawImGuiInspector() {}

protected:
	// Used by the Entity class to initialize the parent
	void InitializeOwnership(Entity* parent);

protected:
	Entity* m_parent = nullptr;
};

class ComponentRegistry final
{
public:
	static ComponentRegistry& Get();

	void RegisterComponent(const std::type_info& derived, const std::type_info& base);
	template<typename Derived, typename Base>
	void RegisterComponent()
	{
		RegisterComponent(typeid(Derived), typeid(Base));
	}

	std::vector<const std::type_info*> GetAllDerived(const std::type_info& base) const;
	template<typename T>
	std::vector<std::type_info*> GetAllDerived() const
	{
		return GetAllDerived(*typeid(T));
	}

	std::vector<const std::type_info*> GetDirectlyDerived(const std::type_info& base) const;
	template<typename T>
	std::vector<std::type_info*> GetDirectlyDerived() const
	{
		return GetDirectlyDerived(*typeid(T));
	}

private:
	std::map<const std::type_info*, std::vector<const std::type_info*>> m_registry;
};

template<typename Derived, typename Base> requires std::is_base_of_v<Component, Base>
struct ComponentRegistrar
{
	friend Derived;
private:
	ComponentRegistrar() = default;

	static inline bool s_registered = []()
		{
			ComponentRegistry::Get().RegisterComponent<Derived, Base>();
			return true;
		}();
};

#define DECLARE_COMPONENT(name, parent) \
	class name : public parent, public ComponentRegistrar<name, parent>

DECLARE_COMPONENT(TransformComponent, Component)
{
public:
	virtual std::wstring GetComponentName() const override { return L"Transform Component"; }

	void SetTranslation(glm::float3 translation);
	void SetRotation(glm::quat rotation);
	void SetRotation(glm::float3 eulerRotation);
	void SetScale(glm::float3 scale);

	glm::float4x4 GetTransform() const;
	glm::float3 GetTranslation() const { return m_translation; }
	glm::quat GetRotation() const { return m_rotation; }
	glm::float3 GetScale() const { return m_scale; }

	virtual void DrawImGuiInspector() override;

private:
	glm::float3 m_translation = {0.f, 0.f, 0.f};
	glm::quat m_rotation = {};
	// Euler rotation kept for displaying information in editor
	glm::float3 m_eulerRotation = {};
	glm::float3 m_scale = {1.f, 1.f, 1.f};
};
}
