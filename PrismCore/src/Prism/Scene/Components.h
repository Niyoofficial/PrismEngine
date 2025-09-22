#pragma once

namespace Prism
{
class Entity;

class Component
{
	friend Entity;
public:
	Component() = default;
	virtual ~Component() = default;

	Entity* GetParent() const { return m_parent; }

protected:
	// Used by the Entity class to initialize the parent
	void InitializeOwnership(Entity* parent);

protected:
	Entity* m_parent = nullptr;
};

class TransformComponent : public Component
{
public:
	void SetTranslation(glm::float3 translation);
	void SetRotation(glm::quat rotation);
	void SetRotation(glm::float3 eulerRotation);
	void SetScale(glm::float3 scale);

	glm::float4x4 GetTransform() const;
	glm::float3 GetTranslation() const { return m_translation; }
	glm::quat GetRotation() const { return m_rotation; }
	glm::float3 GetScale() const { return m_scale; }

private:
	glm::float3 m_translation = {0.f, 0.f, 0.f};
	glm::quat m_rotation = {};
	glm::float3 m_scale = {1.f, 1.f, 1.f};
};
}
