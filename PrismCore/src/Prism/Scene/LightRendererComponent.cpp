#include "LightRendererComponent.h"

namespace Prism
{
void LightRendererComponent::SetColor(glm::float3 color)
{
	m_color = color;
}

void LightRendererComponent::SetIntensity(float intensity)
{
	m_intensity = intensity;
}
}
