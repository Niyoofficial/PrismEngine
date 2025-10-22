#pragma once
#include "EntityRenderProxy.h"
#include "Prism/Render/Camera.h"
#include "Prism/Render/TextureView.h"

namespace Prism {
class Entity;
}

namespace Prism::Render
{
class RenderContext;

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

struct RenderSceneInfo
{
	Ref<TextureView> renderTargetView;

	CameraInfo cameraInfo;

	std::vector<Ref<EntityRenderProxy>> proxies;
	EntityRenderProxy* selectedProxy = nullptr;

	Bounds3f sceneBounds;

	std::vector<DirectionalLight> directionalLights;
	std::vector<PointLight> pointLights;

	float bloomThreshold = 1.f;
	float bloomKnee = 0.1f;
};

struct RenderHitProxiesInfo
{
	Ref<TextureView> renderTargetView;

	CameraInfo cameraInfo;

	std::vector<Ref<EntityRenderProxy>> proxies;
};

class SceneRenderPipeline : public RefCounted
{
public:
	virtual ~SceneRenderPipeline() = default;

	virtual void Render(RenderContext* renderContext, const RenderSceneInfo& renderInfo) = 0;
	virtual void RenderHitProxies(RenderContext* renderContext, const RenderHitProxiesInfo& renderInfo) = 0;
};
}
