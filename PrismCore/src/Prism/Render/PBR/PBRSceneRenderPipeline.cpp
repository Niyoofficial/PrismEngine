#include "PBRSceneRenderPipeline.h"

#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderDevice.h"
#include "Prism/Render/RenderUtils.h"

namespace Prism::Render
{
PBRSceneRenderPipeline::PBRSceneRenderPipeline()
{
	auto renderContext = RenderDevice::Get().AllocateContext(L"PBRSceneRenderPipeline_Initialization");


	ConvertSkyboxToCubemap(renderContext);
	GenerateEnvDiffuseIrradiance(renderContext);
	GenerateEnvSpecularIrradiance(renderContext);
	GenerateBRDFIntegrationLUT(renderContext);

	// Shadow scene uniform buffer
	m_sceneShadowPassBuffer = Buffer::Create({
		.bufferName = L"SceneShadowPassInfo_UniformBuffer",
		.size = sizeof(SceneUniformBuffer),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});
	m_sceneShadowPassBufferView = m_sceneShadowPassBuffer->CreateDefaultUniformBufferView();


	RenderDevice::Get().SubmitContext(renderContext);
	RenderDevice::Get().GetRenderCommandQueue()->Flush();
}

void PBRSceneRenderPipeline::Render(RenderInfo renderInfo)
{
	Ref<RenderContext> renderContext = RenderDevice::Get().AllocateContext(L"PBRSceneRenderPipeline_Render");

	RenderShadowPass(renderInfo, renderContext);
}

void PBRSceneRenderPipeline::ConvertSkyboxToCubemap(RenderContext* renderContext)
{
	m_skybox = Texture::Create({
		.textureName = L"Skybox",
		.width = 2048,
		.height = 2048,
		.depthOrArraySize = 6,
		.mipLevels = (int32_t)std::log2f(2048) + 1,
		.dimension = ResourceDimension::TexCube,
		.format = TextureFormat::RGBA32_Float,
		.bindFlags = Flags(BindFlags::ShaderResource) | Flags(BindFlags::UnorderedAccess),
		.optimizedClearValue = {}
	});
	m_skyboxCubeSRV = m_skybox->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::TexCube,
		.subresourceRange = {
			.firstArraySlice = 0,
			.numArraySlices = 6
		}
	});
	m_skyboxArraySRV = m_skybox->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::Tex2D,
		.subresourceRange = {
			.firstMipLevel = 0,
			.numMipLevels = 1,
			.firstArraySlice = 0,
			.numArraySlices = 6
		}
	});
	m_skyboxUAV = m_skybox->CreateView({
		.type = TextureViewType::UAV,
		.dimension = ResourceDimension::Tex2D,
		.subresourceRange = {
			.firstMipLevel = 0,
			.numMipLevels = 1,
			.firstArraySlice = 0,
			.numArraySlices = 6
		}
	});

	// TODO: Remove hardcoded skybox texture
	m_environmentTexture = Texture::Create(L"textures/pisa.hdr");
	m_environmentTextureSRV = m_environmentTexture->CreateView({.type = TextureViewType::SRV});

	renderContext->SetPSO({
		.cs = {
			.filepath = L"shaders/EquirectToCubemap.hlsl",
			.entryName = L"main",
			.shaderType = ShaderType::CS
		},
	});

	renderContext->SetTexture(m_environmentTextureSRV, L"g_environment");
	renderContext->SetTexture(m_skyboxUAV, L"g_skybox");

	renderContext->Dispatch({ 64, 64, 6 });

	renderContext->Barrier(TextureBarrier{
		.texture = m_skybox,
		.syncBefore = BarrierSync::ComputeShading,
		.syncAfter = BarrierSync::ComputeShading,
		.accessBefore = BarrierAccess::Common,
		.accessAfter = BarrierAccess::Common,
		.layoutBefore = BarrierLayout::Common,
		.layoutAfter = BarrierLayout::Common
	});

	m_skybox->GenerateMipMaps(renderContext);
}

void PBRSceneRenderPipeline::GenerateEnvDiffuseIrradiance(RenderContext* renderContext)
{
	m_prefilteredSkybox = Texture::Create({
		.textureName = L"PrefilteredSkybox",
		.width = 2048,
		.height = 2048,
		.depthOrArraySize = 6,
		.mipLevels = 6,
		.dimension = ResourceDimension::TexCube,
		.format = TextureFormat::RGBA32_Float,
		.bindFlags = Flags(BindFlags::ShaderResource) | Flags(BindFlags::UnorderedAccess),
		.optimizedClearValue = {}
	});
	m_prefilteredEnvMapCubeSRV = m_prefilteredSkybox->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::TexCube,
		.subresourceRange = {
			.firstMipLevel = 0,
			.numMipLevels = 6,
			.firstArraySlice = 0,
			.numArraySlices = 6
		}
	});

	auto coeffGenerationBuffer = Buffer::Create({
		.bufferName = L"SHCoefficientsGeneration",
		.size = sizeof(glm::float4) * 9, // RGB channels + one for padding * numOfCoefficients
		.bindFlags = BindFlags::UnorderedAccess,
		.usage = ResourceUsage::Default
	});
	auto coeffBufferView = coeffGenerationBuffer->CreateDefaultUAVView(sizeof(glm::float4)); // Single element

	renderContext->SetPSO({
		.cs = {
			.filepath = L"shaders/DiffuseIrradianceIntegration.hlsl",
			.entryName = L"main",
			.shaderType = ShaderType::CS
		},
	});

	renderContext->Barrier(TextureBarrier{
		.texture = m_skybox,
		.syncBefore = BarrierSync::Copy,
		.syncAfter = BarrierSync::ComputeShading,
		.accessBefore = BarrierAccess::CopyDest,
		.accessAfter = BarrierAccess::ShaderResource,
		.layoutBefore = BarrierLayout::CopyDest,
		.layoutAfter = BarrierLayout::ShaderResource
	});

	renderContext->SetTexture(m_skyboxCubeSRV, L"g_skybox");
	renderContext->SetBuffer(coeffBufferView, L"g_coefficients");

	renderContext->Dispatch({ 1, 1, 1 });

	m_irradianceSHBuffer = Buffer::Create({
		.bufferName = L"IrradianceSH",
		.size = sizeof(glm::float3) * 9, // RGB channels * numOfCoefficients
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Default
	});
	m_irradianceSHBufferView = m_irradianceSHBuffer->CreateDefaultUniformBufferView();

	renderContext->Barrier(BufferBarrier{
		.buffer = coeffGenerationBuffer,
		.syncBefore = BarrierSync::ComputeShading,
		.syncAfter = BarrierSync::Copy,
		.accessBefore = BarrierAccess::UnorderedAccess,
		.accessAfter = BarrierAccess::CopySource
	});

	renderContext->CopyBufferRegion(m_irradianceSHBuffer, 0, coeffGenerationBuffer, 0, coeffGenerationBuffer->GetBufferDesc().size);
}

void PBRSceneRenderPipeline::GenerateEnvSpecularIrradiance(RenderContext* renderContext)
{
	renderContext->Barrier(TextureBarrier{
		.texture = m_skybox,
		.syncBefore = BarrierSync::ComputeShading,
		.syncAfter = BarrierSync::Copy,
		.accessBefore = BarrierAccess::ShaderResource,
		.accessAfter = BarrierAccess::CopySource,
		.layoutBefore = BarrierLayout::ShaderResource,
		.layoutAfter = BarrierLayout::CopySource
	});

	for (int32_t i = 0; i < 6; ++i)
		renderContext->CopyTextureRegion(m_prefilteredSkybox, {},
										 GetSubresourceIndex(0, m_prefilteredSkybox->GetTextureDesc().GetMipLevels(), i, 6),
										 m_skybox, GetSubresourceIndex(0, m_skybox->GetTextureDesc().GetMipLevels(), i, 6));

	renderContext->Barrier(TextureBarrier{
		.texture = m_skybox,
		.syncBefore = BarrierSync::Copy,
		.syncAfter = BarrierSync::ComputeShading,
		.accessBefore = BarrierAccess::CopySource,
		.accessAfter = BarrierAccess::ShaderResource,
		.layoutBefore = BarrierLayout::CopySource,
		.layoutAfter = BarrierLayout::ShaderResource
	});

	renderContext->Barrier(TextureBarrier{
		.texture = m_prefilteredSkybox,
		.syncBefore = BarrierSync::Copy,
		.syncAfter = BarrierSync::ComputeShading,
		.accessBefore = BarrierAccess::Common,
		.accessAfter = BarrierAccess::UnorderedAccess,
		.layoutBefore = BarrierLayout::Common,
		.layoutAfter = BarrierLayout::UnorderedAccess
	});

	renderContext->SetPSO({
		.cs = {
			.filepath = L"shaders/SpecularIrradianceIntegration.hlsl",
			.entryName = L"main",
			.shaderType = ShaderType::CS
		},
	});

	renderContext->SetTexture(m_skyboxCubeSRV, L"g_skybox");

	struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) PrefilterData
	{
		float roughness = 0.f;
		int32_t totalResolution = 0;
		int32_t mipResolution = 0;
		int32_t sampleCount = 0;
	};

	for (int32_t i = 1; i < m_prefilteredSkybox->GetTextureDesc().GetMipLevels(); ++i)
	{
		int32_t threadGroupSize = m_prefilteredSkybox->GetTextureDesc().GetWidth() / (int32_t)std::pow(2, i) / 32;

		renderContext->SetTexture(m_prefilteredSkybox->CreateView({
									  .type = TextureViewType::UAV,
									  .dimension = ResourceDimension::Tex2D,
									  .subresourceRange = {
										  .firstMipLevel = i,
										  .numMipLevels = 1,
										  .firstArraySlice = 0,
										  .numArraySlices = 6
									  }
								  }), L"g_outputTexture");

		std::array sampleCounts = {
			8, 16, 64, 128, 128
		};

		PrefilterData data = {
			.roughness = (float)i / (float)(m_prefilteredSkybox->GetTextureDesc().GetMipLevels() - 1),
			.totalResolution = m_prefilteredSkybox->GetTextureDesc().GetWidth(),
			.mipResolution = m_prefilteredSkybox->GetTextureDesc().GetWidth() / (int32_t)std::pow(2, i),
			.sampleCount = sampleCounts.size() >= i ? sampleCounts[i - 1] : sampleCounts.back()
		};

		auto prefilterDataBuffer = Buffer::Create({
													  .bufferName = std::wstring(L"PrefilterDataBuffer_") + std::to_wstring(
														  data.mipResolution),
													  .size = sizeof(PrefilterData),
													  .bindFlags = BindFlags::UniformBuffer,
													  .usage = ResourceUsage::Default,
													  .cpuAccess = CPUAccess::None
												  },
												  {
													  .data = &data,
													  .sizeInBytes = sizeof(data)
												  });

		renderContext->SetBuffer(prefilterDataBuffer->CreateDefaultUniformBufferView(), L"g_prefilterData");

		renderContext->Dispatch({ threadGroupSize, threadGroupSize, 6 });
	}
}

void PBRSceneRenderPipeline::GenerateBRDFIntegrationLUT(RenderContext* renderContext)
{
	m_BRDFLUT = Texture::Create({
		.textureName = L"BRDFLUT",
		.width = 1024,
		.height = 1024,
		.dimension = ResourceDimension::Tex2D,
		.format = TextureFormat::RG32_Float,
		.bindFlags = Flags(BindFlags::ShaderResource) | Flags(BindFlags::UnorderedAccess),
	});

	renderContext->SetPSO({
		.cs = {
			.filepath = L"shaders/BRDFIntegration.hlsl",
			.entryName = L"main",
			.shaderType = ShaderType::CS
		},
	});

	struct
	{
		int32_t resolution;
	} integrationData;
	integrationData.resolution = m_BRDFLUT->GetTextureDesc().GetWidth();

	auto integrationDataBuffer = Buffer::Create({
													.bufferName = L"IntegrationDataBuffer",
													.size = sizeof(integrationData),
													.bindFlags = BindFlags::UniformBuffer,
													.usage = ResourceUsage::Default,
													.cpuAccess = CPUAccess::None
												},
												{
													.data = &integrationData,
													.sizeInBytes = sizeof(integrationData)
												});

	renderContext->SetBuffer(integrationDataBuffer->CreateDefaultUniformBufferView(), L"g_integrationData");

	renderContext->SetTexture(m_BRDFLUT->CreateView({ .type = TextureViewType::UAV }), L"g_outputTexture");

	renderContext->Dispatch({ integrationData.resolution / 8, integrationData.resolution / 8, 1 });
}

void PBRSceneRenderPipeline::RenderShadowPass(const RenderInfo& renderInfo, RenderContext* renderContext)
{
	renderContext->BeginEvent({}, L"Shadows");

	renderContext->SetPSO({
		.vs = {
			.filepath = L"shaders/ShadowMap.hlsl",
			.entryName = L"vsmain",
			.shaderType = ShaderType::VS
		},
		.ps = {
			.filepath = L"shaders/ShadowMap.hlsl",
			.entryName = L"psmain",
			.shaderType = ShaderType::PS
		},
		.rasterizerState = {
			.fillMode = FillMode::Solid,
			.cullMode = CullMode::Back,
			.depthBias = 10000,
			.depthBiasClamp = 0.f,
			.slopeScaledDepthBias = 1.5f,
		},
	});

	int32_t dirLightIndex = 0;
	for (auto& dirLight : renderInfo.directionalLights)
	{
		TextureDesc shadowMapDesc = {
			.textureName = L"SunShadowMap",
			.width = 8192,
			.height = 8192,
			.dimension = ResourceDimension::Tex2D,
			.format = TextureFormat::D32_Float,
			.bindFlags = Flags(BindFlags::DepthStencil) | Flags(BindFlags::ShaderResource),
			.optimizedClearValue = DepthStencilClearValue{
				.format = TextureFormat::D32_Float,
				.depthStencil = {
					.depth = 1.f
				}
			}
		};
		ResizeResourceArrayIfNeeded(m_dirLightShadowMaps, (int32_t)renderInfo.directionalLights.size(), shadowMapDesc);

		auto shadowMap = m_dirLightShadowMaps[dirLightIndex];
		glm::float2 shadowMapSize = { (float)shadowMap->GetTextureDesc().GetWidth(), (float)shadowMap->GetTextureDesc().GetHeight() };
		renderContext->SetViewport({ {0.f, 0.f}, shadowMapSize, {0.f, 1.f} });
		renderContext->SetScissor({ {0.f, 0.f}, shadowMapSize });
		renderContext->SetRenderTarget(nullptr, shadowMap->CreateView({.type = TextureViewType::DSV}));

		renderContext->ClearDepthStencilView(shadowMap->CreateView({.type = TextureViewType::DSV}), ClearFlags::ClearDepth);

		glm::float4x4 lightView = glm::lookAt(renderInfo.sceneBounds.GetRadius() * -dirLight.direction,
											  dirLight.direction, {0.f, 1.f, 0.f});
		glm::float4x4 lightProj = glm::ortho(-renderInfo.sceneBounds.GetRadius(),
											 renderInfo.sceneBounds.GetRadius(),
											 -renderInfo.sceneBounds.GetRadius(),
											 renderInfo.sceneBounds.GetRadius(),
											 0.f, renderInfo.sceneBounds.GetRadius() * 2.f);

		struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) SceneShadowPassInfo
		{
			alignas(16)
			glm::float4x4 lightViewProj;
		};

		SceneShadowPassInfo sceneShadow = {
			.lightViewProj = lightProj * lightView,
		};

		void* sceneBufferData = m_sceneShadowPassBuffer->Map(CPUAccess::Write);
		memcpy_s(sceneBufferData, m_sceneShadowPassBuffer->GetBufferDesc().size, &sceneShadow, sizeof(sceneShadow));
		m_sceneShadowPassBuffer->Unmap();

		renderContext->SetBuffer(m_sceneShadowPassBufferView, L"g_shadowSceneBuffer");

		int32_t proxyIndex = 0;
		for (auto& proxy : renderInfo.proxies)
		{
			struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) PrimitiveShadowPassInfo
			{
				glm::float4x4 world;
			};

			if (m_primitiveShadowPassBuffers.size() <= proxyIndex)
			{
				m_primitiveShadowPassBuffers.emplace_back(Buffer::Create({
					.bufferName = L"PrimitiveShadowPassInfo_UniformBuffer" + std::to_wstring(proxyIndex),
					.size = sizeof(PrimitiveShadowPassInfo),
					.bindFlags = BindFlags::UniformBuffer,
					.usage = ResourceUsage::Dynamic,
					.cpuAccess = CPUAccess::Write
				}));
			}

			PrimitiveShadowPassInfo primitiveShadowPassInfo = {
				.world = proxy->GetWorldTransform()
			};

			void* data = m_primitiveShadowPassBuffers[proxyIndex]->Map(CPUAccess::Write);
			memcpy_s(data, m_primitiveShadowPassBuffers[proxyIndex]->GetBufferDesc().size,
					 &primitiveShadowPassInfo, sizeof(PrimitiveShadowPassInfo));
			m_primitiveShadowPassBuffers[proxyIndex]->Unmap();

			auto [vb, ib] = RenderDevice::Get().GetVertexBufferCache().GetOrCreateMeshBuffers(m_defaultVertexAttributeList, proxy->GetMeshAsset());
			renderContext->SetVertexBuffer(vb, GetVertexSize(m_defaultVertexAttributeList));
			renderContext->SetIndexBuffer(ib, IndexBufferFormat::Uint32);

			auto nodeInfo = RenderDevice::Get().GetVertexBufferCache().GetNodeIndexInfo(proxy->GetMeshAsset(), proxy->GetMeshNode());
			renderContext->DrawIndexed({
				.numIndices = proxy->GetMeshAsset()->GetIndexCount(proxy->GetMeshNode()),
				.numInstances = 1,
				.startIndexLocation = nodeInfo.startIndex,
				.baseVertexLocation = nodeInfo.baseVertex
			});

			++proxyIndex;
		}
	}

	renderContext->EndEvent();
}

template<typename Res, typename Desc>
	requires
	(std::is_same_v<Res, Texture> && std::is_same_v<Desc, TextureDesc>) ||
	(std::is_same_v<Res, Buffer> && std::is_same_v<Desc, BufferDesc>)
void PBRSceneRenderPipeline::ResizeResourceArrayIfNeeded(std::vector<Ref<Res>>& resArray, int32_t sizeToFit, Desc resDesc)
{
	if (sizeToFit > resArray.size())
	{
		resArray.reserve(sizeToFit);

		for (int32_t i = resArray.size(); i < sizeToFit; ++i)
		{
			if constexpr (std::is_same_v<Res, Texture>)
				resArray.push_back(Texture::Create(resDesc));
			else
				resArray.push_back(Buffer::Create(resDesc));
		}
	}
}
}
