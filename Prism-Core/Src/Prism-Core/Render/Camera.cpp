#include "pcpch.h"
#include "Camera.h"


namespace Prism::Render
{
Camera::Camera(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	UpdateViewMatrix();
	SetPerspective(fovDegrees, aspectRatio, nearZ, farZ);
}

void Camera::SetPosition(glm::float3 position)
{
	m_position = position;
	UpdateViewMatrix();
}

void Camera::SetRotation(glm::float3 eulerRotation)
{
	m_rotation = eulerRotation;
	UpdateViewMatrix();
}

void Camera::AddPosition(glm::float3 position)
{
	m_position += position;
	UpdateViewMatrix();
}

void Camera::AddRotation(glm::float3 eulerRotation)
{
	m_rotation += eulerRotation;
	UpdateViewMatrix();
}

void Camera::SetPerspective(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	m_projectionMatrix = glm::perspectiveFov(glm::radians(fovDegrees), aspectRatio, 1.f, nearZ, farZ);;
	m_invProjectionMatrix = glm::inverse(m_projectionMatrix);
	UpdateViewProjectionMatrix();
}

glm::float3 Camera::GetForwardVector() const
{
	auto ret = glm::float3(0.f, 0.f, 1.f) * glm::quat(m_rotation);
	return ret;
}

glm::float3 Camera::GetRightVector() const
{
	return glm::normalize(glm::cross(GetUpVector(), GetForwardVector()));
}

glm::float3 Camera::GetUpVector() const
{
	return m_up;
}

void Camera::UpdateViewMatrix()
{
	glm::quat quatRot = glm::identity<glm::quat>();
	quatRot = quatRot * glm::quat({0.f, m_rotation.y, 0.f});
	quatRot = glm::quat({m_rotation.x, 0.f, 0.f}) * quatRot;
	m_viewMatrix = glm::toMat4(quatRot) * glm::translate(glm::mat4x4(1.f), -m_position);
	m_invViewMatrix = glm::inverse(m_viewMatrix);
	UpdateViewProjectionMatrix();
}

void Camera::UpdateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	m_invViewProjectionMatrix = glm::inverse(m_viewProjectionMatrix);
}

glm::quat Camera::LimitRotation(glm::quat rotation)
{
	glm::vec3 eulerRotation = glm::eulerAngles(rotation);
	//PE_CORE_LOG(Info, "{}", glm::degrees(eulerRotation.z));
	//eulerRotation.z = glm::clamp(eulerRotation.z, glm::radians(-89.f), glm::radians(89.f));
	return glm::quat(eulerRotation);
}
}
