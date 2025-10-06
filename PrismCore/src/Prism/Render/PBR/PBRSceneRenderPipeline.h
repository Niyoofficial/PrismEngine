#pragma once
#include "Prism/Render/RenderConstants.h"
#include "Prism/Render/SceneRenderPipeline.h"
#include "Prism/Render/Texture.h"
#include "Prism/Render/VertexBufferCache.h"

namespace Prism::Render
{
class RenderContext;

struct GBuffer
{
	enum class Type
	{
		Depth,
		Color,
		Normal,
		Roughness_Metal_AO,
		Count
	};

	void CreateResources(glm::int2 windowSize);

	Render::Texture* GetTexture(Type type)
	{
		return entries[(size_t)type].texture.Raw();
	}
	Render::TextureView* GetView(Type type, Render::TextureViewType view)
	{
		return entries[(size_t)type].views.at(view).Raw();
	}

	struct Entry
	{
		Ref<Render::Texture> texture;
		std::unordered_map<Render::TextureViewType, Ref<Render::TextureView>> views;
	};

	std::array<Entry, (size_t)Type::Count> entries;
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
	void RenderBasePass(const RenderInfo& renderInfo, RenderContext* renderContext);

	template<typename Res, typename Desc> requires (std::is_same_v<Res, Texture> && std::is_same_v<Desc, TextureDesc>) || (std::is_same_v<Res, Buffer>&& std::is_same_v <Desc, BufferDesc>)
	void ResizeResourceArrayIfNeeded(std::vector<Ref<Res>>& resArray, int32_t sizeToFit, Desc resDesc, BarrierLayout initLayout = BarrierLayout::Common);

private:
	VertexAttributeList m_defaultVertexAttributeList = {
		VertexAttribute::Position,
		VertexAttribute::Normal,
		VertexAttribute::TexCoord,
		VertexAttribute::Tangent,
		VertexAttribute::Bitangent
	};


	GBuffer m_gbuffer;

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

	// Shadow pass
	std::vector<Ref<Buffer>> m_sceneShadowPassBuffers;
	std::vector<Ref<Buffer>> m_primitiveShadowPassBuffers;
	std::vector<Ref<Texture>> m_dirLightShadowMaps;

	// Base pass
	Ref<Buffer> m_sceneBasePassBuffer;
	Ref<BufferView> m_sceneBasePassBufferView;
	std::vector<Ref<Buffer>> m_primitiveBasePassBuffers;
};
}
