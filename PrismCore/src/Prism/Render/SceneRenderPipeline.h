#pragma once
#include "EntityRenderProxy.h"
#include "Prism/Render/Camera.h"

namespace Prism::Render
{
struct DirectionalLight
{
	alignas(16)
	glm::float3 direction;
	alignas(16)
	glm::float3 lightColor;
};

struct PointLight
{
	alignas(16)
	glm::float3 position;
	alignas(16)
	glm::float3 lightColor;
};

struct RenderInfo
{
	Ref<TextureView> renderTargetView;

	CameraInfo cameraInfo;

	std::vector<Ref<EntityRenderProxy>> proxies;
	EntityRenderProxy* selectedProxy;
	Bounds3f sceneBounds;

	std::vector<DirectionalLight> directionalLights;
	std::vector<PointLight> pointLights;

	float bloomThreshold = 1.f;
	float bloomKnee = 0.1f;
};

class SceneRenderPipeline : public RefCounted
{
public:
	virtual ~SceneRenderPipeline() = default;

	virtual void Render(RenderInfo renderInfo) = 0;
};
}
