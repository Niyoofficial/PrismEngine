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
	m_rotation = LimitRotation(eulerRotation);
	UpdateViewMatrix();
}

void Camera::AddPosition(glm::float3 position)
{
	m_position += position;
	UpdateViewMatrix();
}

void Camera::AddRotation(glm::float3 eulerRotation)
{
	m_rotation = LimitRotation(m_rotation + eulerRotation);
	UpdateViewMatrix();
}

void Camera::SetPerspective(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	m_projectionMatrix = glm::perspectiveFov(glm::radians(fovDegrees), aspectRatio, 1.f, nearZ, farZ);;
	m_invProjectionMatrix = glm::inverse(m_projectionMatrix);
	UpdateViewProjectionMatrix();
}

glm::quat Camera::GetRotationQuat() const
{
	glm::quat quatRot = glm::identity<glm::quat>();
	quatRot = quatRot * glm::quat({ 0.f, m_rotation.y, 0.f }); // Add Yaw locally
	quatRot = glm::quat({ m_rotation.x, 0.f, 0.f }) * quatRot; // Add Pitch globally
	return quatRot;
}

glm::float3 Camera::GetForwardVector() const
{
	return glm::float3(0.f, 0.f, 1.f) * GetRotationQuat();
}

glm::float3 Camera::GetRightVector() const
{
	return glm::normalize(glm::cross(GetUpVector(), GetForwardVector()));
}

glm::float3 Camera::GetUpVector() const
{
	return m_up;
}

CameraInfo Camera::GetCameraInfo() const
{
	return {
		.view = GetViewMatrix(),
		.proj = GetProjectionMatrix(),
		.viewProj = GetViewProjectionMatrix(),
		.invView = GetInvViewMatrix(),
		.invProj = GetInvProjectionMatrix(),
		.invViewProj = GetInvViewProjectionMatrix(),
		.cameraPos = GetPosition(),
	};
}

void Camera::UpdateViewMatrix()
{
	m_viewMatrix = glm::toMat4(GetRotationQuat()) * glm::translate(glm::mat4x4(1.f), -m_position);
	m_invViewMatrix = glm::inverse(m_viewMatrix);
	UpdateViewProjectionMatrix();
}

void Camera::UpdateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	m_invViewProjectionMatrix = glm::inverse(m_viewProjectionMatrix);
}

glm::float3 Camera::LimitRotation(glm::float3 eulerRotation) const
{
	return {
		glm::clamp(eulerRotation.x, glm::radians(-89.9f), glm::radians(89.f)),
		eulerRotation.y,
		eulerRotation.z
	};
}
}
