#pragma once
#include "Prism/Scene/Components.h"

namespace Prism
{
class LightRendererComponent : public Component
{
public:
	void SetColor(glm::float3 color);
	void SetIntensity(float intensity);

	glm::float3 GetColor() const { return m_color; }
	float GetIntensity() const { return m_intensity; }

private:
	glm::float3 m_color = {1.f, 1.f, 1.f};
	float m_intensity = 1.f;
};
}
