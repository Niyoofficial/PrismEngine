#pragma once
#include "EntityRenderProxy.h"

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
	Ref<TextureView> renderTarget;

	glm::float4x4 view;
	glm::float4x4 invView;
	glm::float4x4 proj;
	glm::float4x4 invProj;
	glm::float4x4 viewProj;
	glm::float4x4 invViewProj;

	std::vector<Ref<EntityRenderProxy>> proxies;
	Bounds3f sceneBounds;

	std::vector<DirectionalLight> directionalLights;
	std::vector<PointLight> pointLights;
};

class SceneRenderPipeline : public RefCounted
{
public:
	virtual ~SceneRenderPipeline() = default;

	virtual void Render(RenderInfo renderInfo) = 0;
};
}
