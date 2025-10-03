#pragma once
#include "Prism/Render/RenderConstants.h"
#include "Prism/Render/SceneRenderPipeline.h"
#include "Prism/Render/Texture.h"
#include "Prism/Render/VertexBufferCache.h"

namespace Prism::Render
{
class RenderContext;

struct CameraInfo
{
	glm::float4x4 view;
	glm::float4x4 proj;
	glm::float4x4 viewProj;
	glm::float4x4 invView;
	glm::float4x4 invProj;
	glm::float4x4 invViewProj;

	glm::float3 camPos;
};

struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) SceneUniformBuffer
{
	alignas(16)
	CameraInfo camera;
};

class PBRSceneRenderPipeline : public SceneRenderPipeline
{
public:
	PBRSceneRenderPipeline();

	void Render(RenderInfo renderInfo) override;

private:
	void ConvertSkyboxToCubemap(RenderContext* renderContext);
	void GenerateEnvDiffuseIrradiance(RenderContext* renderContext);
	void GenerateEnvSpecularIrradiance(RenderContext* renderContext);
	void GenerateBRDFIntegrationLUT(RenderContext* renderContext);

	void RenderShadowPass(const RenderInfo& renderInfo, RenderContext* renderContext);

	template<typename Res, typename Desc> requires (std::is_same_v<Res, Texture> && std::is_same_v<Desc, TextureDesc>) || (std::is_same_v<Res, Buffer>&& std::is_same_v <Desc, BufferDesc>)
	void ResizeResourceArrayIfNeeded(std::vector<Ref<Res>>& resArray, int32_t sizeToFit, Desc resDesc);

private:
	VertexAttributeList m_defaultVertexAttributeList = {
		VertexAttribute::Position,
		VertexAttribute::Normal,
		VertexAttribute::TexCoord,
		VertexAttribute::Tangent,
		VertexAttribute::Bitangent
	};


	Ref<Texture> m_skybox;
	Ref<TextureView> m_skyboxCubeSRV;
	Ref<TextureView> m_skyboxArraySRV;
	Ref<TextureView> m_skyboxUAV;
	Ref<Texture> m_environmentTexture;
	Ref<TextureView> m_environmentTextureSRV;

	Ref<Texture> m_prefilteredSkybox;
	Ref<TextureView> m_prefilteredEnvMapCubeSRV;
	Ref<Buffer> m_irradianceSHBuffer;
	Ref<BufferView> m_irradianceSHBufferView;

	Ref<Texture> m_BRDFLUT;

	std::vector<Ref<Texture>> m_dirLightShadowMaps;

	Ref<Buffer> m_sceneShadowPassBuffer;
	Ref<BufferView> m_sceneShadowPassBufferView;
	std::vector<Ref<Buffer>> m_primitiveShadowPassBuffers;
};
}
