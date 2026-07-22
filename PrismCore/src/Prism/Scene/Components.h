#pragma once

namespace Prism
{
class Component : public RefCounted
{
	friend class Scene;
public:
	Component() = default;
	virtual ~Component() = default;

	virtual std::wstring GetComponentName() const = 0;

	int64_t GetParentId() const { return m_parentId; }

	virtual void DrawImGuiInspector() {}

	virtual YAML::Node ToYAML() const = 0;
	virtual void FromYAML(const YAML::Node& node) = 0;

protected:
	// Used by the Scene class to initialize the parent
	void InitializeOwnership(int64_t parentId);

protected:
	int64_t m_parentId = -1;
};

class ComponentRegistry final
{
public:
	static ComponentRegistry& Get();

	template<typename Derived, typename Base>
	void RegisterComponent()
	{
		m_registry[&typeid(Base)].push_back(&typeid(Derived));
		m_creationFunctions[typeid(Derived).hash_code()] = []() { return Ref<Derived>::Create(); };
	}

	Ref<Component> CreateComponentFromHash(size_t hash) const;

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
	std::map<size_t, std::function<Ref<Component>()>> m_creationFunctions;
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

	virtual YAML::Node ToYAML() const override;
	virtual void FromYAML(const YAML::Node& node) override;

private:
	glm::float3 m_translation = {0.f, 0.f, 0.f};
	glm::quat m_rotation = {};
	// Euler rotation kept for displaying information in editor
	glm::float3 m_eulerRotation = {};
	glm::float3 m_scale = {1.f, 1.f, 1.f};
};
}
