#pragma once


namespace Prism::Render
{
class Camera : public RefCounted
{
public:
	Camera(float fovDegrees, float aspectRatio, float nearZ, float farZ);

	void SetPosition(glm::float3 position);
	void SetRotation(glm::float3 eulerRotation);
	void AddPosition(glm::float3 position);
	void AddRotation(glm::float3 eulerRotation);

	void SetPerspective(float fovDegrees, float aspectRatio, float nearZ, float farZ);


	glm::float3 GetPosition() const { return m_position; }
	glm::quat GetRotation() const { return m_rotation; }
	glm::quat GetRotationQuat() const;
	glm::float3 GetForwardVector() const;
	glm::float3 GetRightVector() const;
	glm::float3 GetUpVector() const;

	const glm::float4x4& GetViewMatrix() const { return m_viewMatrix; }
	const glm::float4x4& GetInvViewMatrix() const { return m_invViewMatrix; }
	const glm::float4x4& GetProjectionMatrix() const { return m_projectionMatrix; }
	const glm::float4x4& GetInvProjectionMatrix() const { return m_invProjectionMatrix; }
	const glm::float4x4& GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }
	const glm::float4x4& GetInvViewProjectionMatrix() const { return m_invViewProjectionMatrix; }

protected:
	void UpdateViewMatrix();
	void UpdateViewProjectionMatrix();

	glm::float3 LimitRotation(glm::float3 eulerRotation) const;

protected:
	glm::float3 m_position = {};
	glm::float3 m_up = {0.f, 1.f, 0.f};
	glm::float3 m_rotation = {};

	glm::float4x4 m_viewMatrix = glm::float4x4(1.f);
	glm::float4x4 m_invViewMatrix = glm::float4x4(1.f);
	glm::float4x4 m_projectionMatrix = glm::float4x4(1.f);
	glm::float4x4 m_invProjectionMatrix = glm::float4x4(1.f);
	glm::float4x4 m_viewProjectionMatrix = glm::float4x4(1.f);
	glm::float4x4 m_invViewProjectionMatrix = glm::float4x4(1.f);
};
}
