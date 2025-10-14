#pragma once
#include "Prism/Base/AppEvents.h"
#include "Prism/Base/AppEvents.h"
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
		Stencil,
		Color,
		Normal,
		Roughness_Metal_AO,
		Count
	};

	void CreateResources(glm::int2 windowSize);

	Texture* GetTexture(Type type) const
	{
		return entries[(size_t)type].texture.Raw();
	}
	TextureView* GetView(Type type, TextureViewType view) const
	{
		return entries[(size_t)type].views.at(view).Raw();
	}

	struct Entry
	{
		Ref<Texture> texture;
		std::unordered_map<TextureViewType, Ref<TextureView>> views;
	};

	std::array<Entry, (size_t)Type::Count> entries;
};

class PBRSceneRenderPipeline : public SceneRenderPipeline
{
public:
	PBRSceneRenderPipeline();

	virtual void Render(RenderContext* renderContext, const RenderSceneInfo& renderInfo) override;
	virtual void RenderHitProxies(RenderContext* renderContext, const RenderHitProxiesInfo& renderInfo) override;

	const GBuffer& GetGBuffer() const { return m_gbuffer; }

private:
	void CreateInitialResources();
	void CheckForScreenResize(glm::int2 newScreenSize);
	void CreateScreenSizeDependentResources(glm::int2 newScreenSize);

	void ConvertSkyboxToCubemap(RenderContext* renderContext);
	void GenerateEnvDiffuseIrradiance(RenderContext* renderContext);
	void GenerateEnvSpecularIrradiance(RenderContext* renderContext);
	void GenerateBRDFIntegrationLUT(RenderContext* renderContext);

	void RenderShadowPass(RenderContext* renderContext, const RenderSceneInfo& renderInfo);
	void RenderBasePass(RenderContext* renderContext, const RenderSceneInfo& renderInfo);
	void RenderLightingPass(RenderContext* renderContext, const RenderSceneInfo& renderInfo);
	void RenderBloomPass(RenderContext* renderContext, const RenderSceneInfo& renderInfo);
	void RenderSelectionOutlinePass(RenderContext* renderContext, const RenderSceneInfo& renderInfo);
	void RenderFinalCompositionPass(RenderContext* renderContext, const RenderSceneInfo& renderInfo);

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

	Ref<Texture> m_sceneColor;

	// Shadow pass
	std::vector<Ref<Buffer>> m_sceneShadowPassBuffers;
	std::vector<Ref<Buffer>> m_primitiveShadowPassBuffers;
	std::vector<Ref<Texture>> m_dirLightShadowMaps;

	// Base pass
	Ref<Buffer> m_sceneBasePassBuffer;
	std::vector<Ref<Buffer>> m_primitiveBasePassBuffers;

	// Lighting pass
	Ref<Buffer> m_dirLightingPassBuffer;

	// Bloom pass
	Ref<Buffer> m_bloomPassSettingsBuffer;
	Ref<Texture> m_bloomDownsampleA;
	Ref<Texture> m_bloomDownsampleB;
	Ref<Texture> m_bloomUpsampleTexture;

	// Selection outline
	int32_t m_outlineWidth = 1.f;
	Ref<Buffer> m_primitiveSelectionOutlinePassBuffer;
	Ref<Buffer> m_sceneSelectionOutlinePassBuffer;
	Ref<Texture> m_outlineMask;
	Ref<Texture> m_jumpFloodTextureA;
	Ref<Texture> m_jumpFloodTextureB;
	Texture* m_outlineOutput = nullptr;
	Ref<Buffer> m_jumpFloodSettingsBuffer;

	// Final composition
	Ref<Buffer> m_outlineSettingsBuffer;

	// Hit proxies
	Ref<Buffer> m_sceneHitProxiesBuffer;
};
}
