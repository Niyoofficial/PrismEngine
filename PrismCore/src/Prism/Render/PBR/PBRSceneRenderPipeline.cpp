#include "PBRSceneRenderPipeline.h"

#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderDevice.h"
#include "Prism/Render/RenderUtils.h"

namespace Prism::Render
{
void GBuffer::CreateResources(glm::int2 windowSize)
{
	using namespace Prism::Render;

	auto depthStencil = Texture::Create({
		.textureName = L"GBuffer_DepthStencil",
		.width = windowSize.x,
		.height = windowSize.y,
		.dimension = ResourceDimension::Tex2D,
		.format = TextureFormat::R32G8X24_Typeless,
		.bindFlags = Flags(BindFlags::DepthStencil) | Flags(BindFlags::ShaderResource),
		.optimizedClearValue = DepthStencilClearValue{
			.format = TextureFormat::D32_Float_S8X24_UInt
		}
	},
		BarrierLayout::DepthStencilWrite);
	auto depthStencilDSV = depthStencil->CreateView({
		.type = TextureViewType::DSV, .format = TextureFormat::D32_Float_S8X24_UInt
	});
	auto depthSRV = depthStencil->CreateView({
		.type = TextureViewType::SRV, .format = TextureFormat::R32_Float_X8X24_Typeless
	});
	auto stencilSRV = depthStencil->CreateView({
		.type = TextureViewType::SRV, .format = TextureFormat::X32_Typeless_G8X24_UInt
	});

	auto color = Texture::Create({
		.textureName = L"GBuffer_Color",
		.width = windowSize.x,
		.height = windowSize.y,
		.dimension = ResourceDimension::Tex2D,
		.format = TextureFormat::RGBA16_UNorm,
		.bindFlags = Flags(BindFlags::RenderTarget) | Flags(BindFlags::ShaderResource),
		.optimizedClearValue = RenderTargetClearValue{
			.format = TextureFormat::RGBA16_UNorm,
			.color = {0.f, 0.f, 0.f, 1.f}
		}
		}, BarrierLayout::RenderTarget);
	auto colorRTV = color->CreateView({
		.type = TextureViewType::RTV
		});
	auto colorSRV = color->CreateView({
		.type = TextureViewType::SRV
		});

	auto normals = Texture::Create({
		.textureName = L"GBuffer_Normals",
		.width = windowSize.x,
		.height = windowSize.y,
		.dimension = ResourceDimension::Tex2D,
		.format = TextureFormat::RGBA16_Float,
		.bindFlags = Flags(BindFlags::RenderTarget) | Flags(BindFlags::ShaderResource),
		.optimizedClearValue = RenderTargetClearValue{
			.format = TextureFormat::RGBA16_Float,
			.color = {0.f, 0.f, 0.f, 1.f}
		}
		}, BarrierLayout::RenderTarget);
	auto normalsRTV = normals->CreateView({
		.type = TextureViewType::RTV
		});
	auto normalsSRV = normals->CreateView({
		.type = TextureViewType::SRV
		});

	auto roughnessMetalAO = Texture::Create({
		.textureName = L"GBuffer_RoughnessMetalAO",
		.width = windowSize.x,
		.height = windowSize.y,
		.dimension = ResourceDimension::Tex2D,
		.format = TextureFormat::RGBA16_UNorm,
		.bindFlags = Flags(BindFlags::RenderTarget) | Flags(BindFlags::ShaderResource),
		.optimizedClearValue = RenderTargetClearValue{
			.format = TextureFormat::RGBA16_UNorm,
			.color = {0.f, 0.f, 0.f, 1.f}
		}
		}, BarrierLayout::RenderTarget);
	auto roughnessMetalAORTV = roughnessMetalAO->CreateView({
		.type = TextureViewType::RTV
		});
	auto roughnessMetalAOSRV = roughnessMetalAO->CreateView({
		.type = TextureViewType::SRV
		});

	entries.fill({});

	entries[(size_t)Type::Depth] = {
		.texture = depthStencil,
		.views = {
			{TextureViewType::DSV, depthStencilDSV},
			{TextureViewType::SRV, depthSRV}
		}
	};
	entries[(size_t)Type::Stencil] = {
		.texture = depthStencil,
		.views = {
			{TextureViewType::DSV, depthStencilDSV},
			{TextureViewType::SRV, stencilSRV}
		}
	};
	entries[(size_t)Type::Color] = {
		.texture = color,
		.views = {
			{TextureViewType::RTV, colorRTV},
			{TextureViewType::SRV, colorSRV}
		}
	};
	entries[(size_t)Type::Normal] = {
		.texture = normals,
		.views = {
			{TextureViewType::RTV, normalsRTV},
			{TextureViewType::SRV, normalsSRV}
		}
	};
	entries[(size_t)Type::Roughness_Metal_AO] = {
		.texture = roughnessMetalAO,
		.views = {
			{TextureViewType::RTV, roughnessMetalAORTV},
			{TextureViewType::SRV, roughnessMetalAOSRV}
		}
	};
}

struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) SceneInfo
{
	CameraInfo camera;
};

struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) DirectionalLightingPassInfo
{
	DirectionalLight dirLight;
	glm::float4x4 shadowViewProj;
};

struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) SettingsBloomPass
{
	float threshold = 1.f;
	float knee = 0.1f;
	int32_t lod = 0;
};

struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) JumpFloodSettings
{
	int32_t stepWidth;
};

struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) OutlineSettings
{
	float outlineWidth;
};

struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) PrimitiveSelectionOutlinePassInfo
{
	glm::float4x4 world;
};

struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) PrimitiveHitProxiesInfo
{
	glm::float4x4 world;
	alignas(16)
	uint32_t ID;
};

PBRSceneRenderPipeline::PBRSceneRenderPipeline()
{
	CreateInitialResources();

	auto renderContext = RenderDevice::Get().AllocateContext(L"PBRSceneRenderPipeline_Initialization");

	ConvertSkyboxToCubemap(renderContext);
	GenerateEnvDiffuseIrradiance(renderContext);
	GenerateEnvSpecularIrradiance(renderContext);
	GenerateBRDFIntegrationLUT(renderContext);

	RenderDevice::Get().SubmitContext(renderContext);
	RenderDevice::Get().GetRenderCommandQueue()->Flush();
}

void PBRSceneRenderPipeline::Render(RenderContext* renderContext, const RenderSceneInfo& renderInfo)
{
	renderContext->BeginEvent(L"PBRSceneRenderPipeline_Render");

	CheckForScreenResize(renderInfo.renderTargetView->GetTexture()->GetTextureDesc().GetSize());

	RenderShadowPass(renderContext, renderInfo);
	RenderBasePass(renderContext, renderInfo);
	RenderLightingPass(renderContext, renderInfo);
	RenderBloomPass(renderContext, renderInfo);
	// TODO: This step should only happen in editor and outside of this pipeline
	// or maybe inherit this pipeline and implement EditorPBRPipeline?
	RenderSelectionOutlinePass(renderContext, renderInfo);
	RenderFinalCompositionPass(renderContext, renderInfo);

	renderContext->EndEvent();
}

void PBRSceneRenderPipeline::RenderHitProxies(RenderContext* renderContext, const RenderHitProxiesInfo& renderInfo)
{
	renderContext->BeginEvent(L"PBRSceneRenderPipeline_RenderHitProxies");

	auto renderTargetDesc = renderInfo.renderTargetView->GetTexture()->GetTextureDesc();

	Ref<Texture> depth = Texture::Create({
		.textureName = L"HitProxies_Depth",
		.width = renderInfo.renderTargetView->GetTexture()->GetTextureDesc().GetWidth(),
		.height = renderInfo.renderTargetView->GetTexture()->GetTextureDesc().GetHeight(),
		.format = TextureFormat::D32_Float,
		.bindFlags = Flags(BindFlags::DepthStencil),
		.optimizedClearValue = DepthStencilClearValue{
			.format = TextureFormat::D32_Float
		}
	}, BarrierLayout::DepthStencilWrite);
	auto depthView = depth->CreateView({.type = TextureViewType::DSV, .format = TextureFormat::D32_Float});

	renderContext->SetViewport({{0.f, 0.f}, {renderTargetDesc.GetWidth(), renderTargetDesc.GetHeight()}, {0.f, 1.f}});
	renderContext->SetScissor({{0.f, 0.f}, {renderTargetDesc.GetWidth(), renderTargetDesc.GetHeight()}});

	renderContext->SetRenderTarget(renderInfo.renderTargetView, depthView);
	renderContext->ClearDepthStencilView(depthView, ClearFlags::ClearDepth);

	renderContext->SetPSO(GraphicsPipelineStateDesc{
		.vs = {
			.filepath = L"shaders/HitProxy.hlsl",
			.entryName = L"vsmain",
			.shaderType = ShaderType::VS
		},
		.ps = {
			.filepath = L"shaders/HitProxy.hlsl",
			.entryName = L"psmain",
			.shaderType = ShaderType::PS
		}
	});

	SceneInfo sceneBasePassInfo = {
		.camera = renderInfo.cameraInfo
	};
	void* sceneData = m_sceneBasePassBuffer->Map(CPUAccess::Write);
	memcpy_s(sceneData, m_sceneBasePassBuffer->GetBufferDesc().size, &sceneBasePassInfo, sizeof(sceneBasePassInfo));
	m_sceneBasePassBuffer->Unmap();
	renderContext->SetBuffer(m_sceneBasePassBuffer->CreateDefaultUniformBufferView(), L"g_sceneBuffer");


	int32_t proxyIndex = 0;
	for (auto& proxy : renderInfo.proxies)
	{
		auto [vb, ib] = RenderDevice::Get().GetVertexBufferCache().GetOrCreateMeshBuffers(m_defaultVertexAttributeList, proxy->GetMeshAsset());
		renderContext->SetVertexBuffer(vb, GetVertexSize(m_defaultVertexAttributeList));
		renderContext->SetIndexBuffer(ib, IndexBufferFormat::Uint32);

		PrimitiveHitProxiesInfo primitiveBasePassInfo = {
			.world = proxy->GetWorldTransform(),
			.ID = (uint32_t)proxyIndex
		};
		void* primitiveData = m_primitiveBasePassBuffers[proxyIndex]->Map(CPUAccess::Write);
		memcpy_s(primitiveData, m_primitiveBasePassBuffers[proxyIndex]->GetBufferDesc().size, &primitiveBasePassInfo, sizeof(primitiveBasePassInfo));
		m_primitiveBasePassBuffers[proxyIndex]->Unmap();
		renderContext->SetBuffer(m_primitiveBasePassBuffers[proxyIndex]->CreateDefaultUniformBufferView(), L"g_primitiveBuffer");

		auto nodeInfo = RenderDevice::Get().GetVertexBufferCache().GetNodeIndexInfo(proxy->GetMeshAsset(), proxy->GetMeshNode());
		renderContext->DrawIndexed({
			.numIndices = proxy->GetMeshAsset()->GetIndexCount(proxy->GetMeshNode()),
			.numInstances = 1,
			.startIndexLocation = nodeInfo.startIndex,
			.baseVertexLocation = nodeInfo.baseVertex
		});

		++proxyIndex;
	}

	renderContext->EndEvent();
}

void PBRSceneRenderPipeline::CreateInitialResources()
{
	m_sceneBasePassBuffer = Buffer::Create({
		.bufferName = L"SceneBasePassInfo_UniformBuffer",
		.size = sizeof(SceneInfo),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});

	m_dirLightingPassBuffer = Buffer::Create({
		.bufferName = L"DirectionalLightingPassInfo_UniformBuffer",
		.size = sizeof(DirectionalLightingPassInfo),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});

	m_bloomPassSettingsBuffer = Buffer::Create({
		.bufferName = L"SettingsBloomPass_UniformBuffer",
		.size = sizeof(SettingsBloomPass),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});

	m_primitiveSelectionOutlinePassBuffer = Buffer::Create({
		.bufferName = L"PrimitiveSelectionOutlineInfo_UniformBuffer",
		.size = sizeof(PrimitiveSelectionOutlinePassInfo),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});
	m_sceneSelectionOutlinePassBuffer = Buffer::Create({
		.bufferName = L"SceneSelectionOutlineInfo_UniformBuffer",
		.size = sizeof(SceneInfo),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});

	m_jumpFloodSettingsBuffer = Buffer::Create({
		.bufferName = L"JumpFloodSettings_UniformBuffer",
		.size = sizeof(JumpFloodSettings),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});

	m_outlineSettingsBuffer = Buffer::Create({
		.bufferName = L"OutlineSettings_UniformBuffer",
		.size = sizeof(OutlineSettings),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});


	m_sceneHitProxiesBuffer = Buffer::Create({
		.bufferName = L"SceneHitProxiesInfo_UniformBuffer",
		.size = sizeof(SceneInfo),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});
}

void PBRSceneRenderPipeline::CheckForScreenResize(glm::int2 newScreenSize)
{
	auto gbufferSize = m_gbuffer.GetTexture(GBuffer::Type::Color)
						   ? m_gbuffer.GetTexture(GBuffer::Type::Color)->GetTextureDesc().GetSize()
						   : glm::int2{0, 0};
	if (gbufferSize != newScreenSize)
		CreateScreenSizeDependentResources(newScreenSize);
}

void PBRSceneRenderPipeline::CreateScreenSizeDependentResources(glm::int2 newScreenSize)
{
	m_gbuffer.CreateResources(newScreenSize);

	m_sceneColor = Texture::Create({
									   .textureName = L"SceneColor",
									   .width = newScreenSize.x,
									   .height = newScreenSize.y,
									   .format = TextureFormat::R11G11B10_Float,
									   .bindFlags = Flags(BindFlags::ShaderResource) | Flags(BindFlags::RenderTarget),
									   .optimizedClearValue = RenderTargetClearValue{
										   .format = TextureFormat::R11G11B10_Float,
										   .color = {0.f, 0.f, 0.f, 1.f}
									   }
								   }, BarrierLayout::RenderTarget);

	m_bloomDownsampleA = Texture::Create({
											 .textureName = L"DownsampleBloomTextureA",
											 .width = newScreenSize.x,
											 .height = newScreenSize.y,
											 .mipLevels = 7,
											 .dimension = ResourceDimension::Tex2D,
											 .format = TextureFormat::R11G11B10_Float,
											 .bindFlags = Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::ShaderResource),
										 }, BarrierLayout::UnorderedAccess);
	m_bloomDownsampleB = Texture::Create({
											 .textureName = L"DownsampleBloomTextureB",
											 .width = newScreenSize.x,
											 .height = newScreenSize.y,
											 .mipLevels = 7,
											 .dimension = ResourceDimension::Tex2D,
											 .format = TextureFormat::R11G11B10_Float,
											 .bindFlags = Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::ShaderResource),
										 }, BarrierLayout::UnorderedAccess);
	m_bloomUpsampleTexture = Texture::Create({
												 .textureName = L"UpsampleBloomTexture",
												 .width = newScreenSize.x,
												 .height = newScreenSize.y,
												 .mipLevels = 6,
												 .dimension = ResourceDimension::Tex2D,
												 .format = TextureFormat::R11G11B10_Float,
												 .bindFlags = Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::ShaderResource),
											 }, BarrierLayout::UnorderedAccess);

	m_outlineMask = Texture::Create({
										.textureName = L"OutlineInitMask",
										.width = newScreenSize.x,
										.height = newScreenSize.y,
										.format = TextureFormat::R32_Float,
										.bindFlags = Flags(BindFlags::RenderTarget) | Flags(BindFlags::ShaderResource),
										.optimizedClearValue = RenderTargetClearValue{
											.format = TextureFormat::R32_Float,
											.color = {0.f, 0.f, 0.f, 0.f}
										}
									}, BarrierLayout::RenderTarget);

	m_jumpFloodTextureA = Texture::Create({
											  .textureName = L"JumpFloodA",
											  .width = newScreenSize.x,
											  .height = newScreenSize.y,
											  .format = TextureFormat::RG32_Float,
											  .bindFlags = Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::ShaderResource),
										  }, BarrierLayout::UnorderedAccess);

	m_jumpFloodTextureB = Texture::Create({
											  .textureName = L"JumpFloodB",
											  .width = newScreenSize.x,
											  .height = newScreenSize.y,
											  .format = TextureFormat::RG32_Float,
											  .bindFlags = Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::ShaderResource),
										  }, BarrierLayout::UnorderedAccess);
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
	m_environmentTexture = Texture::CreateFromFile(L"textures/pisa.hdr");
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

	renderContext->Dispatch({64, 64, 6});

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
	auto coeffBufferView = coeffGenerationBuffer->CreateDefaultUAV(sizeof(glm::float4)); // Single element

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

	renderContext->SetTexture(m_BRDFLUT->CreateView({ .type = TextureViewType::UAV}), L"g_outputTexture");

	renderContext->Dispatch({integrationData.resolution / 8, integrationData.resolution / 8, 1});
}

void PBRSceneRenderPipeline::RenderShadowPass(RenderContext* renderContext, const RenderSceneInfo& renderInfo)
{
	renderContext->BeginEvent(L"Shadows");

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

	TextureDesc shadowMapDesc = {
		.textureName = L"ShadowMap",
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
	ResizeResourceArrayIfNeeded(m_dirLightShadowMaps, (int32_t)renderInfo.directionalLights.size(), shadowMapDesc, BarrierLayout::DepthStencilWrite);

	struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) PrimitiveShadowPassInfo
	{
		glm::float4x4 world;
	};
	BufferDesc primitiveShadowPassBufDesc = {
		.bufferName = L"PrimitiveShadowPassInfo_UniformBuffer",
		.size = sizeof(PrimitiveShadowPassInfo),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	};
	ResizeResourceArrayIfNeeded(m_primitiveShadowPassBuffers, (int32_t)renderInfo.proxies.size(), primitiveShadowPassBufDesc);

	struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) SceneShadowPassInfo
	{
		glm::float4x4 lightViewProj;
	};
	BufferDesc sceneShadowPassBufDesc = {
		.bufferName = L"SceneShadowPassInfo_UniformBuffer",
		.size = sizeof(SceneShadowPassInfo),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	};
	ResizeResourceArrayIfNeeded(m_sceneShadowPassBuffers, renderInfo.directionalLights.size() + renderInfo.pointLights.size(), sceneShadowPassBufDesc);

	int32_t dirLightIndex = 0;
	for (auto& dirLight : renderInfo.directionalLights)
	{
		auto shadowMap = m_dirLightShadowMaps[dirLightIndex];
		glm::float2 shadowMapSize = {(float)shadowMap->GetTextureDesc().GetWidth(), (float)shadowMap->GetTextureDesc().GetHeight()};
		renderContext->SetViewport({{0.f, 0.f}, shadowMapSize, {0.f, 1.f}});
		renderContext->SetScissor({{0.f, 0.f}, shadowMapSize});
		renderContext->SetRenderTarget(nullptr, shadowMap->CreateView({.type = TextureViewType::DSV}));

		renderContext->ClearDepthStencilView(shadowMap->CreateView({.type = TextureViewType::DSV}), ClearFlags::ClearDepth);

		glm::float4x4 lightView = glm::lookAt(renderInfo.sceneBounds.GetRadius() * -dirLight.direction,
											  dirLight.direction, {0.f, 1.f, 0.f});
		glm::float4x4 lightProj = glm::ortho(-renderInfo.sceneBounds.GetRadius(),
											 renderInfo.sceneBounds.GetRadius(),
											 -renderInfo.sceneBounds.GetRadius(),
											 renderInfo.sceneBounds.GetRadius(),
											 0.f, renderInfo.sceneBounds.GetRadius() * 2.f);

		SceneShadowPassInfo sceneShadow = {
			.lightViewProj = lightProj * lightView,
		};

		void* sceneBufferData = m_sceneShadowPassBuffers[dirLightIndex]->Map(CPUAccess::Write);
		memcpy_s(sceneBufferData, m_sceneShadowPassBuffers[dirLightIndex]->GetBufferDesc().size, &sceneShadow, sizeof(sceneShadow));
		m_sceneShadowPassBuffers[dirLightIndex]->Unmap();

		renderContext->SetBuffer(m_sceneShadowPassBuffers[dirLightIndex]->CreateDefaultUniformBufferView(), L"g_shadowSceneBuffer");
		
		int32_t proxyIndex = 0;
		for (auto& proxy : renderInfo.proxies)
		{
			PrimitiveShadowPassInfo primitiveShadowPassInfo = {
				.world = proxy->GetWorldTransform()
			};

			// TODO: This can be done once in advance, not for every light
			void* data = m_primitiveShadowPassBuffers[proxyIndex]->Map(CPUAccess::Write);
			memcpy_s(data, m_primitiveShadowPassBuffers[proxyIndex]->GetBufferDesc().size,
					 &primitiveShadowPassInfo, sizeof(PrimitiveShadowPassInfo));
			m_primitiveShadowPassBuffers[proxyIndex]->Unmap();

			renderContext->SetBuffer(m_primitiveShadowPassBuffers[proxyIndex]->CreateDefaultUniformBufferView(), L"g_primitiveBuffer");

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

void PBRSceneRenderPipeline::RenderBasePass(RenderContext* renderContext, const RenderSceneInfo& renderInfo)
{
	renderContext->BeginEvent(L"BasePass", {});

	auto renderTargetDesc = renderInfo.renderTargetView->GetTexture()->GetTextureDesc();
	renderContext->SetViewport({{0.f, 0.f}, {renderTargetDesc.GetWidth(), renderTargetDesc.GetHeight()}, {0.f, 1.f}});
	renderContext->SetScissor({{0.f, 0.f}, {renderTargetDesc.GetWidth(), renderTargetDesc.GetHeight()}});

	renderContext->SetRenderTargets({
										m_gbuffer.GetView(GBuffer::Type::Color, TextureViewType::RTV),
										m_gbuffer.GetView(GBuffer::Type::Normal, TextureViewType::RTV),
										m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::RTV)
									}, m_gbuffer.GetView(GBuffer::Type::Depth, TextureViewType::DSV));

	glm::float4 clearColor = {0.f, 0.f, 0.f, 1.f};
	renderContext->ClearRenderTargetView(m_gbuffer.GetView(GBuffer::Type::Color, TextureViewType::RTV), &clearColor);
	renderContext->ClearRenderTargetView(m_gbuffer.GetView(GBuffer::Type::Normal, TextureViewType::RTV), &clearColor);
	renderContext->ClearRenderTargetView(m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::RTV), &clearColor);
	renderContext->ClearDepthStencilView(m_gbuffer.GetView(GBuffer::Type::Depth, TextureViewType::DSV),
										 Flags(ClearFlags::ClearDepth) | Flags(ClearFlags::ClearStencil));

	renderContext->SetPSO(GraphicsPipelineStateDesc{
		.vs = {
			.filepath = L"shaders/DeferredBasePass.hlsl",
			.entryName = L"vsmain",
			.shaderType = ShaderType::VS
		},
		.ps = {
			.filepath = L"shaders/DeferredBasePass.hlsl",
			.entryName = L"psmain",
			.shaderType = ShaderType::PS
		},
	});


	struct Material
	{
		glm::float3 albedo;
		float metallic = 0.f;
		float roughness = 0.f;
		float ao = 0.f;
	};
	struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) PrimitiveBasePassInfo
	{
		glm::float4x4 world;
		alignas(16)
		glm::float4x4 normalMatrix;

		alignas(16)
		Material material;
	};
	BufferDesc primitiveBasePassBufDesc = {
		.bufferName = L"PrimitiveBasePassInfo_UniformBuffer",
		.size = sizeof(PrimitiveBasePassInfo),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	};
	ResizeResourceArrayIfNeeded(m_primitiveBasePassBuffers, (int32_t)renderInfo.proxies.size(), primitiveBasePassBufDesc);

	SceneInfo sceneBasePassInfo = {
		.camera = renderInfo.cameraInfo
	};
	void* sceneData = m_sceneBasePassBuffer->Map(CPUAccess::Write);
	memcpy_s(sceneData, m_sceneBasePassBuffer->GetBufferDesc().size, &sceneBasePassInfo, sizeof(sceneBasePassInfo));
	m_sceneBasePassBuffer->Unmap();
	renderContext->SetBuffer(m_sceneBasePassBuffer->CreateDefaultUniformBufferView(), L"g_sceneBuffer");

	int32_t proxyIndex = 0;
	for (auto& proxy : renderInfo.proxies)
	{
		auto [vb, ib] = RenderDevice::Get().GetVertexBufferCache().GetOrCreateMeshBuffers(m_defaultVertexAttributeList, proxy->GetMeshAsset());
		renderContext->SetVertexBuffer(vb, GetVertexSize(m_defaultVertexAttributeList));
		renderContext->SetIndexBuffer(ib, IndexBufferFormat::Uint32);

		PrimitiveBasePassInfo primitiveBasePassInfo = {
			.world = proxy->GetWorldTransform(),
			.normalMatrix = glm::transpose(glm::inverse(primitiveBasePassInfo.world)),
			.material = {
				.albedo = glm::float3(1.f, 1.f, 1.f),
				.metallic = 1.f,
				.roughness = 1.f,
				.ao = 1.f
			}
		};
		void* primitiveData = m_primitiveBasePassBuffers[proxyIndex]->Map(CPUAccess::Write);
		memcpy_s(primitiveData, m_primitiveBasePassBuffers[proxyIndex]->GetBufferDesc().size, &primitiveBasePassInfo, sizeof(primitiveBasePassInfo));
		m_primitiveBasePassBuffers[proxyIndex]->Unmap();
		renderContext->SetBuffer(m_primitiveBasePassBuffers[proxyIndex]->CreateDefaultUniformBufferView(), L"g_primitiveBuffer");

		if (auto albedo = proxy->GetMeshAsset()->GetNodeTexture(proxy->GetMeshNode(), MeshLoading::TextureType::Albedo))
			renderContext->SetTexture(albedo->CreateDefaultSRV(), L"g_albedoTexture");
		if (auto metallic = proxy->GetMeshAsset()->GetNodeTexture(proxy->GetMeshNode(), MeshLoading::TextureType::Metallic))
			renderContext->SetTexture(metallic->CreateDefaultSRV(), L"g_metallicTexture");
		if (auto roughness = proxy->GetMeshAsset()->GetNodeTexture(proxy->GetMeshNode(), MeshLoading::TextureType::Roughness))
			renderContext->SetTexture(roughness->CreateDefaultSRV(), L"g_roughnessTexture");
		if (auto normal = proxy->GetMeshAsset()->GetNodeTexture(proxy->GetMeshNode(), MeshLoading::TextureType::Normals))
			renderContext->SetTexture(normal->CreateDefaultSRV(), L"g_normalTexture");

		auto nodeInfo = RenderDevice::Get().GetVertexBufferCache().GetNodeIndexInfo(proxy->GetMeshAsset(), proxy->GetMeshNode());
		renderContext->DrawIndexed({
			.numIndices = proxy->GetMeshAsset()->GetIndexCount(proxy->GetMeshNode()),
			.numInstances = 1,
			.startIndexLocation = nodeInfo.startIndex,
			.baseVertexLocation = nodeInfo.baseVertex
		});

		++proxyIndex;
	}

	renderContext->EndEvent();
}

void PBRSceneRenderPipeline::RenderLightingPass(RenderContext* renderContext, const RenderSceneInfo& renderInfo)
{
	renderContext->BeginEvent(L"LightingPass", {});

	renderContext->SetRenderTarget(m_sceneColor->CreateDefaultRTV(), nullptr);

	glm::float4 clearColor = {0.f, 0.f, 0.f, 1.f};
	renderContext->ClearRenderTargetView(m_sceneColor->CreateDefaultRTV(), &clearColor);

	renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Color, TextureViewType::SRV), L"g_colorTexture");
	renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Normal, TextureViewType::SRV), L"g_normalTexture");
	renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::SRV), L"g_roughnessMetalAOTexture");
	renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Depth, TextureViewType::SRV), L"g_depthTexture");

	{
		renderContext->BeginEvent(L"IndirectLighting");

		renderContext->SetBuffer(m_irradianceSHBufferView, L"g_irradiance");
		renderContext->SetTexture(m_BRDFLUT->CreateView({ .type = TextureViewType::SRV }), L"g_brdfLUT");
		renderContext->SetTexture(m_prefilteredEnvMapCubeSRV, L"g_envMap");

		glm::int2 screenSize = renderInfo.renderTargetView->GetTexture()->GetTextureDesc().GetSize();
		ShaderDesc pixelShader = {
			.filepath = L"shaders/DeferredLightingIndirect.hlsl",
			.entryName = L"psmain",
			.shaderType = ShaderType::PS
		};
		BlendStateDesc blendState = {
			.renderTargetBlendDesc = RenderTargetBlendDesc{.blendEnable = true, .destBlend = BlendFactor::One, .destBlendAlpha = BlendFactor::One}
		};
		DrawFullscreenPixelShader(renderContext, screenSize, pixelShader, &blendState);

		renderContext->EndEvent();
	}

	{
		renderContext->BeginEvent(L"DirectLighting");

		int32_t dirLightIndex = 0;
		for (auto& dirLight : renderInfo.directionalLights)
		{
			glm::float4x4 lightView = glm::lookAt(renderInfo.sceneBounds.GetRadius() * -dirLight.direction,
												  dirLight.direction, {0.f, 1.f, 0.f});
			glm::float4x4 lightProj = glm::ortho(-renderInfo.sceneBounds.GetRadius(),
												 renderInfo.sceneBounds.GetRadius(),
												 -renderInfo.sceneBounds.GetRadius(),
												 renderInfo.sceneBounds.GetRadius(),
												 0.f, renderInfo.sceneBounds.GetRadius() * 2.f);

			DirectionalLightingPassInfo dirLightingPassInfo = {
				.dirLight = dirLight,
				.shadowViewProj = lightProj * lightView
			};

			void* perLightData = m_dirLightingPassBuffer->Map(CPUAccess::Write);
			memcpy_s(perLightData, m_dirLightingPassBuffer->GetBufferDesc().size, &dirLightingPassInfo, sizeof(dirLightingPassInfo));
			m_dirLightingPassBuffer->Unmap();

			renderContext->SetBuffer(m_dirLightingPassBuffer->CreateDefaultUniformBufferView(), L"g_dirLightPassBuffer");
			renderContext->SetBuffer(m_sceneBasePassBuffer->CreateDefaultUniformBufferView(), L"g_sceneBuffer");

			Ref<TextureView> shadowMapSRV = m_dirLightShadowMaps[dirLightIndex]->CreateView({
				.type = TextureViewType::SRV,
				.format = TextureFormat::R32_Float
			});
			renderContext->SetTexture(shadowMapSRV, L"g_shadowMap");

			glm::int2 screenSize = renderInfo.renderTargetView->GetTexture()->GetTextureDesc().GetSize();
			ShaderDesc pixelShader = {
				.filepath = L"shaders/DeferredLightingDirect.hlsl",
				.entryName = L"psmain",
				.shaderType = ShaderType::PS
			};
			BlendStateDesc blendState = {
				.renderTargetBlendDesc = RenderTargetBlendDesc{.blendEnable = true, .destBlend = BlendFactor::One, .destBlendAlpha = BlendFactor::One}
			};
			DrawFullscreenPixelShader(renderContext,  screenSize, pixelShader, &blendState);

			++dirLightIndex;
		}

		renderContext->EndEvent();
	}

	renderContext->EndEvent();
}

void PBRSceneRenderPipeline::RenderBloomPass(RenderContext* renderContext, const RenderSceneInfo& renderInfo)
{
	renderContext->BeginEvent(L"Bloom", {});

	SettingsBloomPass bloomPassSettings = {
		.threshold = renderInfo.bloomThreshold,
		.knee = renderInfo.bloomKnee,
		.lod = 0,
	};

	renderContext->SetBuffer(m_bloomPassSettingsBuffer->CreateDefaultUniformBufferView(), L"g_bloomSettings");

	// Prefilter
	{
		renderContext->BeginEvent(L"Prefilter", {});

		renderContext->SetTexture(m_sceneColor->CreateDefaultSRV(), L"g_inputTexture");
		renderContext->SetTexture(m_bloomDownsampleA->CreateView({ .type = TextureViewType::UAV }), L"g_outputTexture");

		void* bloomSettingsData = m_bloomPassSettingsBuffer->Map(CPUAccess::Write);
		memcpy_s(bloomSettingsData, m_bloomPassSettingsBuffer->GetBufferDesc().size, &bloomPassSettings, sizeof(bloomPassSettings));
		m_bloomPassSettingsBuffer->Unmap();

		renderContext->SetPSO(ComputePipelineStateDesc{
			.cs = {
				.filepath = L"shaders/Bloom.hlsl",
				.entryName = L"Prefilter",
				.shaderType = ShaderType::CS
			}
		});

		renderContext->Dispatch({glm::ceil((glm::float2)m_bloomDownsampleA->GetTextureDesc().GetSize() / 4.f), 1});

		renderContext->EndEvent();
	}

	// Downsample
	{
		renderContext->BeginEvent(L"Downsample", {});

		auto dispatchDownsample =
			[&bloomPassSettings, &renderContext, this](Texture* uavTexture, int32_t uavMipIndex, TextureView* srv, int32_t srvMipReadIndex)
		{
			TextureViewDesc uavDesc = {
				.type = TextureViewType::UAV, .subresourceRange = {.firstMipLevel = uavMipIndex, .numMipLevels = 1}
			};
			renderContext->SetTexture(uavTexture->CreateView(uavDesc), L"g_outputTexture");
			renderContext->SetTexture(srv, L"g_inputTexture");

			bloomPassSettings.lod = srvMipReadIndex;
			void* bloomSettingsData = m_bloomPassSettingsBuffer->Map(CPUAccess::Write);
			memcpy_s(bloomSettingsData, m_bloomPassSettingsBuffer->GetBufferDesc().size, &bloomPassSettings, sizeof(bloomPassSettings));
			m_bloomPassSettingsBuffer->Unmap();

			renderContext->SetPSO(ComputePipelineStateDesc{
				.cs = {
					.filepath = L"shaders/Bloom.hlsl",
					.entryName = L"Downsample",
					.shaderType = ShaderType::CS
				}
			});


			renderContext->Dispatch({glm::ceil((glm::float2)m_bloomDownsampleA->GetTextureDesc().GetSize() / std::pow(2.f, (float)uavMipIndex) / 4.f), 1});
		};

		dispatchDownsample(m_bloomDownsampleB, 0, m_bloomDownsampleA->CreateDefaultSRV(), 0);
		for (int32_t i = 1; i < m_bloomDownsampleA->GetTextureDesc().GetMipLevels(); ++i)
		{
			dispatchDownsample(m_bloomDownsampleA, i, m_bloomDownsampleB->CreateDefaultSRV(), i - 1);
			dispatchDownsample(m_bloomDownsampleB, i, m_bloomDownsampleA->CreateDefaultSRV(), i);
		}

		renderContext->EndEvent();
	}

	// Upsample
	{
		renderContext->BeginEvent(L"Upsample", {});

		renderContext->Barrier(TextureBarrier{
			.texture = m_bloomDownsampleA,
			.syncBefore = BarrierSync::ComputeShading,
			.syncAfter = BarrierSync::ComputeShading,
			.accessBefore = BarrierAccess::UnorderedAccess,
			.accessAfter = BarrierAccess::ShaderResource,
			.layoutBefore = BarrierLayout::UnorderedAccess,
			.layoutAfter = BarrierLayout::ShaderResource,
		});

		for (int32_t i = m_bloomDownsampleA->GetTextureDesc().GetMipLevels() - 2; i >= 0; --i)
		{
			TextureViewDesc uavDesc = {
				.type = TextureViewType::UAV, .subresourceRange = {.firstMipLevel = i, .numMipLevels = 1}
			};
			renderContext->SetTexture(m_bloomUpsampleTexture->CreateView(uavDesc), L"g_outputTexture");
			renderContext->SetTexture(m_bloomDownsampleB->CreateDefaultSRV(), L"g_inputTexture");
			renderContext->SetTexture(i == 5 ? m_bloomDownsampleB->CreateDefaultSRV() : m_bloomUpsampleTexture->CreateDefaultSRV(), L"g_accumulationTexture");

			bloomPassSettings.lod = i;
			void* bloomSettingsData = m_bloomPassSettingsBuffer->Map(CPUAccess::Write);
			memcpy_s(bloomSettingsData, m_bloomPassSettingsBuffer->GetBufferDesc().size, &bloomPassSettings, sizeof(bloomPassSettings));
			m_bloomPassSettingsBuffer->Unmap();

			renderContext->SetPSO(ComputePipelineStateDesc{
				.cs = {
					.filepath = L"shaders/Bloom.hlsl",
					.entryName = L"Upsample",
					.shaderType = ShaderType::CS
				}
			});

			renderContext->Dispatch({glm::ceil((glm::float2)m_bloomDownsampleA->GetTextureDesc().GetSize() / std::pow(2.f, (float)i) / 4.f), 1});
		}

		renderContext->Barrier(TextureBarrier{
			.texture = m_bloomDownsampleA,
			.syncBefore = BarrierSync::ComputeShading,
			.syncAfter = BarrierSync::ComputeShading,
			.accessBefore = BarrierAccess::ShaderResource,
			.accessAfter = BarrierAccess::UnorderedAccess,
			.layoutBefore = BarrierLayout::ShaderResource,
			.layoutAfter = BarrierLayout::UnorderedAccess,
		});

		renderContext->EndEvent();
	}

	renderContext->EndEvent();
}

void PBRSceneRenderPipeline::RenderSelectionOutlinePass(RenderContext* renderContext, const RenderSceneInfo& renderInfo)
{
	if (renderInfo.selectedProxy)
	{
		renderContext->BeginEvent(L"SelectionOutline");

		renderContext->ClearRenderTargetView(m_outlineMask->CreateDefaultRTV());

		auto renderTargetDesc = renderInfo.renderTargetView->GetTexture()->GetTextureDesc();
		renderContext->SetViewport({{0.f, 0.f}, {renderTargetDesc.GetWidth(), renderTargetDesc.GetHeight()}, {0.f, 1.f}});
		renderContext->SetScissor({{0.f, 0.f}, {renderTargetDesc.GetWidth(), renderTargetDesc.GetHeight()}});

		renderContext->SetRenderTarget(m_outlineMask->CreateDefaultRTV(), nullptr);

		renderContext->SetPSO(GraphicsPipelineStateDesc{
			.vs = {
				.filepath = L"shaders/JumpFloodOutline.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			},
			.ps = {
				.filepath = L"shaders/JumpFloodOutline.hlsl",
				.entryName = L"PsMask",
				.shaderType = ShaderType::PS
			},
			.rasterizerState = {
				.cullMode = CullMode::None
			},
			.depthStencilState = {
				.depthEnable = false,
				.depthWriteEnable = false
			},
		});

		auto [vb, ib] = RenderDevice::Get().GetVertexBufferCache().GetOrCreateMeshBuffers(m_defaultVertexAttributeList, renderInfo.selectedProxy->GetMeshAsset());
		renderContext->SetVertexBuffer(vb, GetVertexSize(m_defaultVertexAttributeList));
		renderContext->SetIndexBuffer(ib, IndexBufferFormat::Uint32);

		SceneInfo sceneInfo = {
			.camera = renderInfo.cameraInfo,
		};
		void* sceneData = m_sceneSelectionOutlinePassBuffer->Map(CPUAccess::Write);
		memcpy_s(sceneData, m_sceneSelectionOutlinePassBuffer->GetBufferDesc().size, &sceneInfo, sizeof(sceneInfo));
		m_sceneSelectionOutlinePassBuffer->Unmap();
		renderContext->SetBuffer(m_sceneSelectionOutlinePassBuffer->CreateDefaultUniformBufferView(), L"g_sceneBuffer");

		PrimitiveSelectionOutlinePassInfo primitiveInfo = {
			.world = renderInfo.selectedProxy->GetWorldTransform()
		};
		void* primitiveData = m_primitiveSelectionOutlinePassBuffer->Map(CPUAccess::Write);
		memcpy_s(primitiveData, m_primitiveSelectionOutlinePassBuffer->GetBufferDesc().size, &primitiveInfo, sizeof(primitiveInfo));
		m_primitiveSelectionOutlinePassBuffer->Unmap();
		renderContext->SetBuffer(m_primitiveSelectionOutlinePassBuffer->CreateDefaultUniformBufferView(), L"g_primitiveBuffer");

		auto nodeInfo = RenderDevice::Get().GetVertexBufferCache().GetNodeIndexInfo(renderInfo.selectedProxy->GetMeshAsset(), renderInfo.selectedProxy->GetMeshNode());
		renderContext->DrawIndexed({
			.numIndices = renderInfo.selectedProxy->GetMeshAsset()->GetIndexCount(renderInfo.selectedProxy->GetMeshNode()),
			.numInstances = 1,
			.startIndexLocation = nodeInfo.startIndex,
			.baseVertexLocation = nodeInfo.baseVertex
		});

		renderContext->SetPSO(ComputePipelineStateDesc{
			.cs = {
				.filepath = L"shaders/JumpFloodOutline.hlsl",
				.entryName = L"CsInitMask",
				.shaderType = ShaderType::CS
			}
		});

		renderContext->SetTexture(m_outlineMask->CreateDefaultSRV(), L"g_inputMask");
		renderContext->SetTexture(m_jumpFloodTextureA->CreateDefaultUAV(), L"g_outputMask");

		renderContext->Dispatch({glm::ceil((glm::float2)m_jumpFloodTextureA->GetTextureDesc().GetSize() / 4.f), 1});


		renderContext->SetBuffer(m_jumpFloodSettingsBuffer->CreateDefaultUniformBufferView(), L"g_jumpFloodSettings");

		int32_t jfaIter = (int32_t)std::log2(m_outlineWidth + 1);

		Texture* inputTex = m_jumpFloodTextureA;
		Texture* outputTex = m_jumpFloodTextureB;
		for (int32_t i = jfaIter; i >= 0; --i)
		{
			renderContext->SetPSO(ComputePipelineStateDesc{
				.cs = {
					.filepath = L"shaders/JumpFloodOutline.hlsl",
					.entryName = L"CsJumpFlood",
					.shaderType = ShaderType::CS
				}
			});

			void* data = m_jumpFloodSettingsBuffer->Map(CPUAccess::Write);
			JumpFloodSettings jumpFloodSettings = {
				.stepWidth = (int32_t)(std::pow(2, i) + 0.5)
			};
			memcpy(data, &jumpFloodSettings, sizeof(jumpFloodSettings));
			m_jumpFloodSettingsBuffer->Unmap();

			renderContext->SetTexture(inputTex->CreateDefaultSRV(), L"g_inputMask");
			renderContext->SetTexture(outputTex->CreateDefaultUAV(), L"g_outputMask");

			renderContext->Dispatch({glm::ceil((glm::float2)m_jumpFloodTextureB->GetTextureDesc().GetSize() / 4.f), 1});

			m_outlineOutput = outputTex;

			std::swap(inputTex, outputTex);
		}

		renderContext->EndEvent();
	}
}

void PBRSceneRenderPipeline::RenderFinalCompositionPass(RenderContext* renderContext, const RenderSceneInfo& renderInfo)
{
	renderContext->BeginEvent(L"FinalComposition");

	renderContext->ClearRenderTargetView(renderInfo.renderTargetView);
	renderContext->SetRenderTarget(renderInfo.renderTargetView, nullptr);

	renderContext->SetTexture(m_sceneColor->CreateDefaultSRV(), L"g_sceneColorTexture");
	renderContext->SetTexture(m_bloomUpsampleTexture->CreateDefaultSRV(), L"g_bloomTexture");
	if (m_outlineOutput && renderInfo.selectedProxy)
	{
		renderContext->SetTexture(m_outlineOutput->CreateDefaultSRV(), L"g_outlineTexture");

		renderContext->SetBuffer(m_outlineSettingsBuffer->CreateDefaultUniformBufferView(), L"g_outlineSettings");

		void* data = m_outlineSettingsBuffer->Map(CPUAccess::Write);
		OutlineSettings outlineSettings = {
			.outlineWidth = (float)m_outlineWidth
		};
		memcpy(data, &outlineSettings, sizeof(outlineSettings));
		m_outlineSettingsBuffer->Unmap();
	}

	glm::int2 screenSize = renderInfo.renderTargetView->GetTexture()->GetTextureDesc().GetSize();
	ShaderDesc ps = {
		.filepath = L"shaders/FinalComposition.hlsl",
		.entryName = L"psmain",
		.shaderType = ShaderType::PS
	};
	DrawFullscreenPixelShader(renderContext, screenSize, ps);

	renderContext->EndEvent();
}

template<typename Res, typename Desc>
	requires
	(std::is_same_v<Res, Texture> && std::is_same_v<Desc, TextureDesc>) ||
	(std::is_same_v<Res, Buffer> && std::is_same_v<Desc, BufferDesc>)
void PBRSceneRenderPipeline::ResizeResourceArrayIfNeeded(std::vector<Ref<Res>>& resArray, int32_t sizeToFit, Desc resDesc, BarrierLayout initLayout)
{
	if (sizeToFit > resArray.size())
	{
		resArray.reserve(sizeToFit);

		for (int32_t i = resArray.size(); i < sizeToFit; ++i)
		{
			if constexpr (std::is_same_v<Res, Texture>)
				resArray.push_back(Texture::Create(resDesc, initLayout));
			else
				resArray.push_back(Buffer::Create(resDesc));
		}
	}
}
}
