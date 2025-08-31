#include "Sandbox.h"

#include "Prism/Base/Base.h"
#include "Prism/Base/Platform.h"
#include "Prism/Base/Window.h"
#include "Prism/Render/Material.h"
#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderContext.h"
#include "Prism/Render/RenderDevice.h"
#include "Prism/Render/RenderUtils.h"
#include "Prism/Render/Texture.h"
#include "Prism/Render/TextureView.h"

IMPLEMENT_APPLICATION(SandboxApplication);

void GBuffer::CreateResources(glm::int2 windowSize)
{
	using namespace Prism::Render;

	auto depthStencil = Texture::Create({
		.textureName = L"GBuffer_Depth",
		.width = windowSize.x,
		.height = windowSize.y,
		.dimension = ResourceDimension::Tex2D,
		.format = TextureFormat::R32_Typeless,
		.bindFlags = Flags(BindFlags::DepthStencil) | Flags(BindFlags::ShaderResource),
		.optimizedClearValue = DepthStencilClearValue{
			.format = TextureFormat::D32_Float
		}
	},
	BarrierLayout::DepthStencilWrite);
	auto depthStencilDSV = depthStencil->CreateView({
		.type = TextureViewType::DSV, .format = TextureFormat::D32_Float
	});
	auto depthStencilSRV = depthStencil->CreateView({
		.type = TextureViewType::SRV, .format = TextureFormat::R32_Float
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
			{ TextureViewType::DSV, depthStencilDSV },
			{ TextureViewType::SRV, depthStencilSRV }
		}
	};
	entries[(size_t)Type::Color] = {
		.texture = color,
		.views = {
			{ TextureViewType::RTV, colorRTV },
			{ TextureViewType::SRV, colorSRV }
		}
	};
	entries[(size_t)Type::Normal] = {
		.texture = normals,
		.views = {
			{ TextureViewType::RTV, normalsRTV },
			{ TextureViewType::SRV, normalsSRV }
		}
	};
	entries[(size_t)Type::Roughness_Metal_AO] = {
		.texture = roughnessMetalAO,
		.views = {
			{ TextureViewType::RTV, roughnessMetalAORTV },
			{ TextureViewType::SRV, roughnessMetalAOSRV }
		}
	};
}

SandboxLayer::SandboxLayer(Core::Window* owningWindow)
	: m_owningWindow(owningWindow)
{
	using namespace Prism::Render;
	Layer::Attach();

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::WindowResized>(
		[this](Core::AppEvent event)
		{
			m_gbuffer.CreateResources({m_owningWindow->GetSize().x, m_owningWindow->GetSize().y});

			m_camera->SetPerspective(45.f,
									 (float)m_owningWindow->GetSize().x / (float)m_owningWindow->GetSize().y,
									 0.1f, 10000.f);
		});

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::KeyDown>(
		[](Core::AppEvent event)
		{
			if (std::get<Core::AppEvents::KeyDown>(event).keyCode == KeyCode::Escape)
				SandboxApplication::Get().CloseApplication();
		});

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::MouseMotion>(
		[this](Core::AppEvent event)
		{
			if (Core::Platform::Get().IsKeyPressed(KeyCode::RightMouseButton))
			{
				glm::float2 delta = std::get<Core::AppEvents::MouseMotion>(event).relPosition;
				delta *= m_mouseSpeed;
				m_camera->AddRotation({-delta.y, -delta.x, 0.f});
			}
		});

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::MouseWheel>(
		[this](Core::AppEvent event)
		{
			float delta = std::get<Core::AppEvents::MouseWheel>(event).y;
			constexpr float moveSpeedChangeMult = 0.01f;
			constexpr float minMoveSpeed = 0.01f;
			m_cameraSpeed += delta * moveSpeedChangeMult;
			//m_cameraSpeed = std::max(m_cameraSpeed, minMoveSpeed);
		});

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::MouseButtonDown>(
		[this](Core::AppEvent event)
		{
			if (m_owningWindow.IsValid() && std::get<Core::AppEvents::MouseButtonDown>(event).keyCode == KeyCode::RightMouseButton)
				Core::Platform::Get().SetMouseRelativeMode(m_owningWindow.Raw(), true);
		});
	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::MouseButtonUp>(
		[this](Core::AppEvent event)
		{
			if (m_owningWindow.IsValid() && std::get<Core::AppEvents::MouseButtonUp>(event).keyCode == KeyCode::RightMouseButton)
				Core::Platform::Get().SetMouseRelativeMode(m_owningWindow.Raw(), false);
		});

	glm::int2 windowSize = SandboxApplication::Get().GetWindow()->GetSize();

	m_camera = new Camera(45.f, (float)windowSize.x / (float)windowSize.y, 0.1f, 10000.f);
	m_camera->SetPosition({0.f, 0.f, 0.f});

	m_gbuffer.CreateResources(windowSize);

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
	m_skyboxCubeSRVView = m_skybox->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::TexCube,
		.subresourceRange = {
			.firstArraySlice = 0,
			.numArraySlices = 6
		}
	});
	m_skyboxArraySRVView = m_skybox->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::Tex2D,
		.subresourceRange = {
			.firstMipLevel = 0,
			.numMipLevels = 1,
			.firstArraySlice = 0,
			.numArraySlices = 6
		}
	});
	m_skyboxUAVView = m_skybox->CreateView({
		.type = TextureViewType::UAV,
		.dimension = ResourceDimension::Tex2D,
		.subresourceRange = {
			.firstMipLevel = 0,
			.numMipLevels = 1,
			.firstArraySlice = 0,
			.numArraySlices = 6
		}
	});

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
	m_prefilteredEnvMapCubeSRVView = m_prefilteredSkybox->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::TexCube,
		.subresourceRange = {
			.firstMipLevel = 0,
			.numMipLevels = 6,
			.firstArraySlice = 0,
			.numArraySlices = 6
		}
	});

	m_BRDFLUT = Texture::Create({
		.textureName = L"BRDFLUT",
		.width = 1024,
		.height = 1024,
		.dimension = ResourceDimension::Tex2D,
		.format = TextureFormat::RG32_Float,
		.bindFlags = Flags(BindFlags::ShaderResource) | Flags(BindFlags::UnorderedAccess),
	});

	m_sunShadowMap = Texture::Create({
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
	}, BarrierLayout::DepthStencilWrite);
	m_sunShadowMapDSV = m_sunShadowMap->CreateView({.type = TextureViewType::DSV});
	m_sunShadowMapSRV = m_sunShadowMap->CreateView({.type = TextureViewType::SRV, .format = TextureFormat::R32_Float});

	m_environmentTexture = Texture::Create(L"textures/pisa.hdr");
	m_environmentTextureSRV = m_environmentTexture->CreateView();
	
	// Scene cbuffer
	m_sceneUniBuffer = Buffer::Create({
										.bufferName = L"SceneCBuffer",
										.size = sizeof(SceneUniformBuffer),
										.bindFlags = BindFlags::UniformBuffer,
										.usage = ResourceUsage::Dynamic,
										.cpuAccess = CPUAccess::Write
									});
	m_sceneUniBufferView = m_sceneUniBuffer->CreateDefaultCBVView();

	// Lights cbuffer
	m_lightsUniBuffer = Buffer::Create({
										.bufferName = L"LightsCBuffer",
										.size = sizeof(LightsUniformBuffer),
										.bindFlags = BindFlags::UniformBuffer,
										.usage = ResourceUsage::Dynamic,
										.cpuAccess = CPUAccess::Write
		});
	m_lightsUniBufferView = m_lightsUniBuffer->CreateDefaultCBVView();

	// PerLight cbuffer
	m_perLightUniBuffer = Buffer::Create({
										.bufferName = L"PerLightCBuffer",
										.size = sizeof(PerLightUniformBuffer),
										.bindFlags = BindFlags::UniformBuffer,
										.usage = ResourceUsage::Dynamic,
										.cpuAccess = CPUAccess::Write
		});
	m_perLightUniBufferView = m_perLightUniBuffer->CreateDefaultCBVView();

	// Scene shadow cbuffer
	m_sceneShadowUniformBuffer = Buffer::Create({
										.bufferName = L"SceneShadowCBuffer",
										.size = sizeof(SceneUniformBuffer),
										.bindFlags = BindFlags::UniformBuffer,
										.usage = ResourceUsage::Dynamic,
										.cpuAccess = CPUAccess::Write
									});
	m_sceneShadowView = m_sceneShadowUniformBuffer->CreateDefaultCBVView();

	// Load sponza
	m_sponza = SandboxApplication::LoadMeshFromFilePBR(L"Sponza", L"meshes/SponzaCrytek/Sponza.gltf");

	// Load cube
	m_cube = SandboxApplication::LoadMeshFromFile(L"Cube", L"meshes/Cube.fbx",
		[this](auto& primitiveData)
		{
			Render::Material material;
			material.SetVertexShader({
				.filepath = L"shaders/Skybox.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			});
			material.SetPixelShader({
				.filepath = L"shaders/Skybox.hlsl",
				.entryName = L"psmain",
				.shaderType = ShaderType::PS
			});
			material.SetRasterizerState({
				.cullMode = CullMode::Front
			});
			material.SetDepthStencilState({
				.depthEnable = true,
				.depthWriteEnable = true,
				.depthFunc = ComparisionFunction::LessEqual
			});

			material.SetTexture(L"g_skybox", m_prefilteredEnvMapCubeSRVView);
			return material;
		}, L"g_modelBuffer", sizeof(CBufferModel));

	// Load sphere
	m_sphere = SandboxApplication::LoadMeshFromFile(L"Sphere", L"meshes/Sphere.gltf",
		[](auto& primitiveData)
		{
			Render::Material material;
			material.SetVertexShader({
				.filepath = L"shaders/PBR.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			});
			material.SetPixelShader({
				.filepath = L"shaders/PBR.hlsl",
				.entryName = L"spheremain",
				.shaderType = ShaderType::PS
			});
			return material;
		}, L"g_modelBuffer", sizeof(CBufferModel));

	{
		auto renderContext = RenderDevice::Get().AllocateContext(L"Initialization");

		// HDRI to cubemap skybox
		{
			renderContext->SetPSO({
				.cs = {
					.filepath = L"shaders/EquirectToCubemap.hlsl",
					.entryName = L"main",
					.shaderType = ShaderType::CS
				},
			});

			renderContext->SetTexture(m_environmentTextureSRV, L"g_environment");
			renderContext->SetTexture(m_skyboxUAVView, L"g_skybox");

			renderContext->Dispatch(64, 64, 6);

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

		// Generate env diffuse irradiance
		{
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

			renderContext->SetTexture(m_skyboxCubeSRVView, L"g_skybox");
			renderContext->SetBuffer(coeffBufferView, L"g_coefficients");

			renderContext->Dispatch(1, 1, 1);

			m_irradianceSHBuffer = Buffer::Create({
				.bufferName = L"IrradianceSH",
				.size = sizeof(glm::float3) * 9, // RGB channels * numOfCoefficients
				.bindFlags = BindFlags::UniformBuffer,
				.usage = ResourceUsage::Default
			});
			m_irradianceSHBufferView = m_irradianceSHBuffer->CreateDefaultCBVView();

			renderContext->Barrier(BufferBarrier{
				.buffer = coeffGenerationBuffer,
				.syncBefore = BarrierSync::ComputeShading,
				.syncAfter = BarrierSync::Copy,
				.accessBefore = BarrierAccess::UnorderedAccess,
				.accessAfter = BarrierAccess::CopySource
			});

			renderContext->CopyBufferRegion(m_irradianceSHBuffer, 0, coeffGenerationBuffer, 0, coeffGenerationBuffer->GetBufferDesc().size);
		}

		// Generate env specular irradiance (prefiltered skybox)
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

			renderContext->SetTexture(m_skyboxCubeSRVView, L"g_skybox");

			struct alignas(Constants::CBUFFER_ALIGNMENT) PrefilterData
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
															  .bufferName = std::wstring(L"PrefilterDataBuffer_") + std::to_wstring(data.mipResolution),
															  .size = sizeof(PrefilterData),
															  .bindFlags = BindFlags::UniformBuffer,
															  .usage = ResourceUsage::Default,
															  .cpuAccess = CPUAccess::None
														  },
														  {
															  .data = &data,
															  .sizeInBytes = sizeof(data)
														  });

				renderContext->SetBuffer(prefilterDataBuffer->CreateDefaultCBVView(), L"g_prefilterData");

				renderContext->Dispatch(threadGroupSize, threadGroupSize, 6);
			}
		}

		// Generate BRDF integration LUT
		{
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

			renderContext->SetBuffer(integrationDataBuffer->CreateDefaultCBVView(), L"g_integrationData");

			renderContext->SetTexture(m_BRDFLUT->CreateView({.type = TextureViewType::UAV}), L"g_outputTexture");

			renderContext->Dispatch(integrationData.resolution / 8, integrationData.resolution / 8, 1);
		}

		RenderDevice::Get().SubmitContext(renderContext);
		RenderDevice::Get().GetRenderCommandQueue()->Flush();
	}
}

void SandboxLayer::UpdateImGui(Duration delta)
{
	Layer::UpdateImGui(delta);

	bool s_showStatWindow = true;
	static bool s_debugMenuOpen = true;

	// Menu bar
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Show"))
		{
			ImGui::MenuItem("Show stat window", nullptr, &s_showStatWindow);
			ImGui::MenuItem("Show debug window", nullptr, &s_debugMenuOpen);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();

		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Recompile Shaders", nullptr))
				Render::RenderDevice::Get().GetShaderCompiler()->RecompileCachedShaders();

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// Stats overlay
	if (s_showStatWindow)
	{
		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove;

		constexpr float padding = 10.0f;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 workPos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		ImVec2 workSize = viewport->WorkSize;
		ImVec2 windowPos;
		windowPos.x = workPos.x + workSize.x - padding;
		windowPos.y = workPos.y + padding;
		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, { 1.f, 0.f });
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::SetNextWindowBgAlpha(0.55f); // Transparent background

		if (ImGui::Begin("Stats overlay", nullptr, window_flags))
		{
			auto& io = ImGui::GetIO();
			ImGui::Text("FPS: %.1f", io.Framerate);
			ImGui::Text("Frame time: %.2f ms", delta.GetMilliseconds());
		}
		ImGui::End();
	}

	if (s_debugMenuOpen)
	{
		ImGui::Begin("Debug", &s_debugMenuOpen, ImGuiWindowFlags_HorizontalScrollbar);

		if (ImGui::CollapsingHeader("GBuffer"))
		{
			float texWidth = (float)m_gbuffer.GetTexture(GBuffer::Type::Color)->GetTextureDesc().GetWidth() / (float)m_gbuffer.GetTexture(GBuffer::Type::Color)->GetTextureDesc().GetHeight() * 256.f;
			ImGui::Text("Color");
			ImGui::Image(m_gbuffer.GetView(GBuffer::Type::Color, Render::TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("Depth");
			ImGui::Image(m_gbuffer.GetView(GBuffer::Type::Depth, Render::TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("Normal");
			ImGui::Image(m_gbuffer.GetView(GBuffer::Type::Normal, Render::TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("R:Roughness G:Metal B:AO");
			static bool s_roughness = true;
			ImGui::Checkbox("Roughness", &s_roughness);
			ImGui::SameLine();
			static bool s_metallic = true;
			ImGui::Checkbox("Metallic", &s_metallic);
			ImGui::SameLine();
			static bool s_ao = true;
			ImGui::Checkbox("AO", &s_ao);
			ImGui::Image(m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, Render::TextureViewType::SRV), {texWidth, 256}, {0, 0}, {1, 1}, {(float)s_roughness, (float)s_metallic, (float)s_ao, 1});
		}

		if (ImGui::CollapsingHeader("Sun"))
		{
			//ImGui::SliderFloat("Environment light scale", &m_environmentLightScale, 0.f, 1.f);
			ImGui::SliderAngle("Sun Rotation Yaw", &m_sunRotation.y, -180.f, 180.f);
			ImGui::SliderAngle("Sun Rotation Pitch", &m_sunRotation.z, -180.f, 180.f);
			ImGui::Image(m_sunShadowMap->CreateView({ .type = Render::TextureViewType::SRV, .format = Render::TextureFormat::R32_Float }), {256, 256});
		}

		if (ImGui::CollapsingHeader("IBL"))
		{
			ImGui::Text("Original image");
			ImGui::Image(m_environmentTextureSRV, {(float)m_environmentTexture->GetTextureDesc().GetWidth() / (float)m_environmentTexture->GetTextureDesc().GetHeight() * 256.f, 256.f});

			ImGui::Text("Prefiltered Env Cube Map");
			static int32_t s_mipIndex = 0;
			ImGui::SliderInt("Mip Index", &s_mipIndex, 0, 5);

			auto viewDesc = Render::TextureViewDesc{
				.type = Render::TextureViewType::SRV,
				.dimension = Render::ResourceDimension::Tex2D,
				.subresourceRange = {.firstMipLevel = s_mipIndex, .numMipLevels = 1, .firstArraySlice = 4, .numArraySlices = 1}
			};
			ImGui::Image(m_prefilteredSkybox->CreateView(viewDesc), {256, 256});
			ImGui::SameLine(0.f, 0.f);
			viewDesc.subresourceRange.firstArraySlice = 0;
			ImGui::Image(m_prefilteredSkybox->CreateView(viewDesc), {256, 256});
			ImGui::SameLine(0.f, 0.f);
			viewDesc.subresourceRange.firstArraySlice = 5;
			ImGui::Image(m_prefilteredSkybox->CreateView(viewDesc), {256, 256});
			ImGui::SameLine(0.f, 0.f);
			viewDesc.subresourceRange.firstArraySlice = 1;
			ImGui::Image(m_prefilteredSkybox->CreateView(viewDesc), {256, 256});
			ImGui::SameLine(0.f, 0.f);
			viewDesc.subresourceRange.firstArraySlice = 2;
			ImGui::Image(m_prefilteredSkybox->CreateView(viewDesc), {256, 256});
			ImGui::SameLine(0.f, 0.f);
			viewDesc.subresourceRange.firstArraySlice = 3;
			ImGui::Image(m_prefilteredSkybox->CreateView(viewDesc), {256, 256});

			ImGui::Text("BRDF LUT");
			ImGui::Image(m_BRDFLUT->CreateView({.type = Render::TextureViewType::SRV}), {256, 256});
		}

		ImGui::End();
	}
}

void SandboxLayer::Update(Duration delta)
{
	using namespace Prism::Render;
	Layer::Update(delta);

	if (Core::Platform::Get().IsKeyPressed(KeyCode::RightMouseButton))
	{
		if (Core::Platform::Get().IsKeyPressed(KeyCode::W))
			m_camera->AddPosition(m_camera->GetForwardVector() * m_cameraSpeed);
		if (Core::Platform::Get().IsKeyPressed(KeyCode::S))
			m_camera->AddPosition(-m_camera->GetForwardVector() * m_cameraSpeed);
		if (Core::Platform::Get().IsKeyPressed(KeyCode::A))
			m_camera->AddPosition(-m_camera->GetRightVector() * m_cameraSpeed);
		if (Core::Platform::Get().IsKeyPressed(KeyCode::D))
			m_camera->AddPosition(m_camera->GetRightVector() * m_cameraSpeed);
		if (Core::Platform::Get().IsKeyPressed(KeyCode::Q))
			m_camera->AddPosition(-m_camera->GetUpVector() * m_cameraSpeed);
		if (Core::Platform::Get().IsKeyPressed(KeyCode::E))
			m_camera->AddPosition(m_camera->GetUpVector() * m_cameraSpeed);
	}

	Ref<RenderContext> renderContext = RenderDevice::Get().AllocateContext(L"SandboxUpdate");

	glm::float3 sunDir = glm::rotate(glm::quat(m_sunRotation), {1.f, 0.f, 0.f});

	glm::float4x4 lightView = glm::lookAt(m_sponza->GetBounds().GetRadius() * -sunDir, sunDir, {0.f, 1.f, 0.f});
	glm::float4x4 lightProj = glm::ortho(-m_sponza->GetBounds().GetRadius(),
										 m_sponza->GetBounds().GetRadius(),
										 -m_sponza->GetBounds().GetRadius(),
										 m_sponza->GetBounds().GetRadius(),
										 0.f, m_sponza->GetBounds().GetRadius() * 2.f);

	// Shadows
	{
		renderContext->BeginEvent({}, L"Shadows");

		glm::float2 shadowMapSize = { (float)m_sunShadowMap->GetTextureDesc().GetWidth(), (float)m_sunShadowMap->GetTextureDesc().GetHeight() };
		renderContext->SetViewport({ {0.f, 0.f}, shadowMapSize, {0.f, 1.f} });
		renderContext->SetScissor({ {0.f, 0.f}, shadowMapSize });
		renderContext->SetRenderTarget(nullptr, m_sunShadowMapDSV);

		renderContext->ClearDepthStencilView(m_sunShadowMapDSV, ClearFlags::ClearDepth);

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

		// Shadow scene
		{
			ShadowUniformBuffer sceneShadow = {
				.lightViewProj = lightProj * lightView,
			};

			void* sceneBufferData = m_sceneShadowUniformBuffer->Map(CPUAccess::Write);
			memcpy_s(sceneBufferData, m_sceneShadowUniformBuffer->GetBufferDesc().size, &sceneShadow, sizeof(sceneShadow));
			m_sceneShadowUniformBuffer->Unmap();

			renderContext->SetBuffer(m_sceneShadowView, L"g_shadowSceneBuffer");
		}

		// Sponza
		{
			CBufferModelShadow modelShadow = {
				.world = glm::float4x4(1.f)
			};
			m_sponza->Draw(renderContext, &modelShadow, sizeof(modelShadow), false);
		}

		renderContext->EndEvent();
	}

	SceneUniformBuffer sceneUniformBuffer = {
				.camera = {
					.view = m_camera->GetViewMatrix(),
					.proj = m_camera->GetProjectionMatrix(),
					.viewProj = m_camera->GetViewProjectionMatrix(),
					.invView = m_camera->GetInvViewMatrix(),
					.invProj = m_camera->GetInvProjectionMatrix(),
					.invViewProj = m_camera->GetInvViewProjectionMatrix(),

					.camPos = m_camera->GetPosition()
				}
	};

	void* sceneData = m_sceneUniBuffer->Map(CPUAccess::Write);
	memcpy_s(sceneData, m_sceneUniBuffer->GetBufferDesc().size, &sceneUniformBuffer, sizeof(sceneUniformBuffer));
	m_sceneUniBuffer->Unmap();

	renderContext->SetBuffer(m_sceneUniBufferView, L"g_sceneBuffer");

	// Base Pass
	{
		renderContext->BeginEvent({}, L"BasePass");

		glm::float2 windowSize = SandboxApplication::Get().GetWindow()->GetSize();

		renderContext->SetViewport({{0.f, 0.f}, windowSize, {0.f, 1.f}});
		renderContext->SetScissor({{0.f, 0.f}, windowSize});

		renderContext->SetRenderTargets({
										   m_gbuffer.GetView(GBuffer::Type::Color, TextureViewType::RTV),
										   m_gbuffer.GetView(GBuffer::Type::Normal, TextureViewType::RTV),
										   m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::RTV)
									   }, m_gbuffer.GetView(GBuffer::Type::Depth, TextureViewType::DSV));

		glm::float4 clearColor = {0.f, 0.f, 0.f, 1.f};
		renderContext->ClearRenderTargetView(m_gbuffer.GetView(GBuffer::Type::Color, TextureViewType::RTV), &clearColor);
		renderContext->ClearRenderTargetView(m_gbuffer.GetView(GBuffer::Type::Normal, TextureViewType::RTV), &clearColor);
		renderContext->ClearRenderTargetView(m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::RTV),
											 &clearColor);
		renderContext->ClearDepthStencilView(m_gbuffer.GetView(GBuffer::Type::Depth, TextureViewType::DSV),
											 Flags(ClearFlags::ClearDepth) | Flags(ClearFlags::ClearStencil));

		renderContext->Barrier(TextureBarrier{
			.texture = m_sunShadowMap,
			.syncBefore = BarrierSync::DepthStencil,
			.syncAfter = BarrierSync::PixelShading,
			.accessBefore = BarrierAccess::DepthStencilWrite,
			.accessAfter = BarrierAccess::ShaderResource,
			.layoutBefore = BarrierLayout::DepthStencilWrite,
			.layoutAfter = BarrierLayout::ShaderResource,
		});

		renderContext->SetTexture(m_sunShadowMapSRV, L"g_shadowMap");

		// Sponza
		{
			CBufferModel modelData = {
				.world = glm::float4x4(1.f),
				.normalMatrix = glm::transpose(glm::inverse(modelData.world)),

				.material = {
					.albedo = glm::float3(1.f, 1.f, 1.f),
					.metallic = 1.f,
					.roughness = 1.f,
					.ao = 1.f
				}
			};
			m_sponza->Draw(renderContext, &modelData, sizeof(CBufferModel));
		}

		// Skybox
		if (0)
		{
			CBufferModel modelData = {
				.world = glm::float4x4(1.f),
				.normalMatrix = glm::transpose(glm::inverse(glm::float3x3(modelData.world))),

				.material = {
					.albedo = glm::float3(0.2f, 0.3f, 0.8f),
					.metallic = 0.2f,
					.roughness = 0.4f,
					.ao = 0.3f
				}
			};
			m_cube->Draw(renderContext, &modelData, sizeof(CBufferModel));
		}

		// Spheres
		if (0)
		{
			CBufferModel modelData = {
				.world = glm::translate(glm::float4x4(1.f), glm::float3(0.f, -5.f, 0.f)),
				.normalMatrix = glm::transpose(glm::inverse(glm::float3x3(modelData.world))),

				.mipLevel = -1.f,

				.material = {
					.albedo = glm::float3(1.f, 1.f, 1.f),
					.metallic = 0.f,
					.roughness = 0.1f,
					.ao = 0.3f
				}
			};

			for (int32_t i = 0; i < 6; ++i)
			{
				modelData.world = glm::translate(modelData.world, glm::float3(2.f, 0.f, 0.f));
				modelData.mipLevel += 1.f;
			
				m_sphere->Draw(renderContext, &modelData, sizeof(CBufferModel));
			}
		}

		renderContext->EndEvent();
	}

	// Lighting Pass
	{
		renderContext->BeginEvent({}, L"LightingPass");

		auto* currentBackBuffer = SandboxApplication::Get().GetWindow()->GetSwapchain()->GetCurrentBackBufferRTV();
		renderContext->SetRenderTarget(currentBackBuffer, nullptr);

		glm::float4 clearColor = {0.8f, 0.2f, 0.3f, 1.f};
		renderContext->ClearRenderTargetView(currentBackBuffer, &clearColor);

		renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Color, TextureViewType::SRV), L"g_colorTexture");
		renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Normal, TextureViewType::SRV), L"g_normalTexture");
		renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::SRV), L"g_roughnessMetalAOTexture");
		renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Depth, TextureViewType::SRV), L"g_depthTexture");

		// Env light
		{
			BlendStateDesc blendState;
			blendState.renderTargetBlendDescs[0] = {.blendEnable = true, .destBlend = BlendFactor::One, .destBlendAlpha = BlendFactor::One};
			renderContext->SetPSO(GraphicsPipelineStateDesc{
			   .vs = {
				   .filepath = L"shaders/DeferredLightingIndirect.hlsl",
				   .entryName = L"vsmain",
				   .shaderType = ShaderType::VS
			   },
			   .ps = {
				   .filepath = L"shaders/DeferredLightingIndirect.hlsl",
				   .entryName = L"psmain",
				   .shaderType = ShaderType::PS
			   },
			   .rasterizerState = {
				   .cullMode = CullMode::Front
			   },
			   .depthStencilState = {
				   .depthEnable = false,
				   .depthWriteEnable = false,
				   .stencilEnable = false
			   },
			});

			renderContext->SetBuffer(m_irradianceSHBufferView, L"g_irradiance");
			renderContext->SetTexture(m_BRDFLUT->CreateView({.type = TextureViewType::SRV}), L"g_brdfLUT");
			renderContext->SetTexture(m_prefilteredEnvMapCubeSRVView, L"g_envMap");

			renderContext->Draw({.numVertices = 3});
		}

		BlendStateDesc blendState;
		blendState.renderTargetBlendDescs[0] = { .blendEnable = true, .destBlend = BlendFactor::One, .destBlendAlpha = BlendFactor::One };
		renderContext->SetPSO(GraphicsPipelineStateDesc{
			.vs = {
				.filepath = L"shaders/DeferredLightingDirect.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			},
			.ps = {
				.filepath = L"shaders/DeferredLightingDirect.hlsl",
				.entryName = L"psmain",
				.shaderType = ShaderType::PS
			},
			.blendState = blendState,
			.rasterizerState = {
				.cullMode = CullMode::Front
			},
			.depthStencilState = {
				.depthEnable = false,
				.depthWriteEnable = false,
				.stencilEnable = false
			},
		});

		LightsUniformBuffer lightsUniformBuffer = {
			.directionalLights = {
				{.direction = sunDir, .lightColor = {1.f, 1.f, 1.f}}
			}
		};

		void* lightsData = m_lightsUniBuffer->Map(CPUAccess::Write);
		memcpy_s(lightsData, m_lightsUniBuffer->GetBufferDesc().size, &lightsUniformBuffer, sizeof(lightsUniformBuffer));
		m_lightsUniBuffer->Unmap();

		renderContext->SetBuffer(m_lightsUniBufferView, L"g_lightsBuffer");

		// Loop over all lights (currently 1)
		{
			PerLightUniformBuffer perLightUniformBuffer = {
				.lightIndex = 0,
				.shadowViewProj = lightProj * lightView
			};

			void* perLightData = m_perLightUniBuffer->Map(CPUAccess::Write);
			memcpy_s(perLightData, m_perLightUniBuffer->GetBufferDesc().size, &perLightUniformBuffer, sizeof(perLightUniformBuffer));
			m_perLightUniBuffer->Unmap();

			renderContext->SetBuffer(m_perLightUniBufferView, L"g_perLightBuffer");

			renderContext->Draw({.numVertices = 3});
		}

		renderContext->Barrier(TextureBarrier{
			.texture = m_sunShadowMap,
			.syncBefore = BarrierSync::PixelShading,
			.syncAfter = BarrierSync::None,
			.accessBefore = BarrierAccess::ShaderResource,
			.accessAfter = BarrierAccess::NoAccess,
			.layoutBefore = BarrierLayout::ShaderResource,
			.layoutAfter = BarrierLayout::DepthStencilWrite,
		});

		renderContext->EndEvent();
	}

	RenderDevice::Get().SubmitContext(renderContext);
}

SandboxApplication& SandboxApplication::Get()
{
	return Application::Get<SandboxApplication>();
}

SandboxApplication::SandboxApplication(int32_t argc, char** argv)
	: Application(argc, argv)
{
	InitPlatform();
	InitRenderer({.enableDebugLayer = true, .initPixLibrary = false});

	auto displayInfo = Core::Platform::Get().GetDisplayInfo(Core::Platform::Get().GetPrimaryDisplayID());

	Core::WindowDesc windowParams = {
		.windowTitle = L"Test",
		.windowSize = {displayInfo.width / 1.5f, displayInfo.height / 1.5f},
		.fullscreen = false
	};
	Render::SwapchainDesc swapchainDesc = {
		.refreshRate = {
			.numerator = 60,
			.denominator = 1
		},
		.format = Render::TextureFormat::RGBA8_UNorm,
		.sampleDesc = {
			.count = 1,
			.quality = 0
		}
	};
	m_window = Core::Window::Create(windowParams, swapchainDesc);

	InitImGui(m_window, Render::TextureFormat::D24_UNorm_S8_UInt);

	m_sandboxLayer = new SandboxLayer(m_window);
	PushLayer(m_sandboxLayer);
}

Core::Window* SandboxApplication::GetWindow() const
{
	return m_window;
}

Ref<Render::PrimitiveBatch> SandboxApplication::LoadMeshFromFile(const std::wstring& primitiveBatchName,
																 const std::wstring& filepath,
																 std::function<Render::Material(const MeshUtils::PrimitiveData&)> createMaterialFunc,
																 std::wstring primitiveBufferParamName,
																 int64_t primitiveBufferSize,
																 MeshUtils::MeshData* outMeshData)
{
	auto fillBatchData = [&primitiveBatchName, &createMaterialFunc, &primitiveBufferParamName, &primitiveBufferSize](MeshUtils::MeshData& meshData)
		{
			Ref batch = new Render::PrimitiveBatch(primitiveBatchName);

			for (auto& primitive : meshData.primitives)
			{
				std::vector<Vertex> vertices;
				for (auto& loadedVertex : primitive.vertices)
				{
					Vertex v;
					v.position = loadedVertex.position;
					v.normal = loadedVertex.normal;
					v.tangent = loadedVertex.tangent;
					v.bitangent = loadedVertex.bitangent;
					v.texCoords = loadedVertex.texCoords;

					vertices.push_back(v);
				}

				batch->AddPrimitive(sizeof(Vertex), Render::IndexBufferFormat::Uint32, vertices.data(), (int64_t)vertices.size(),
									primitive.indices.data(), (int64_t)primitive.indices.size(), primitiveBufferParamName, primitiveBufferSize,
									createMaterialFunc(primitive), primitive.bounds);
			}

			return batch;
		};

	if (outMeshData)
	{
		*outMeshData = MeshUtils::LoadMeshFromFile(filepath);
		return fillBatchData(*outMeshData);
	}
	else
	{
		auto meshData = MeshUtils::LoadMeshFromFile(filepath);
		return fillBatchData(meshData);
	}
}

Ref<Render::PrimitiveBatch> SandboxApplication::LoadMeshFromFilePBR(const std::wstring& primitiveBatchName, const std::wstring& filepath)
{
	MeshUtils::MeshData meshData;
	auto batch = LoadMeshFromFile(primitiveBatchName, filepath,
								  [](const MeshUtils::PrimitiveData& primitiveData)
								  {
									  Render::Material material;
									  if (primitiveData.textures.contains(MeshUtils::TextureType::Albedo))
										  material.SetTexture(L"g_albedoTexture", primitiveData.textures.at(MeshUtils::TextureType::Albedo)->CreateView());
									  if (primitiveData.textures.contains(MeshUtils::TextureType::Normals))
										  material.SetTexture(L"g_normalTexture", primitiveData.textures.at(MeshUtils::TextureType::Normals)->CreateView());
									  if (primitiveData.textures.contains(MeshUtils::TextureType::Metallic))
										  material.SetTexture(L"g_metallicTexture", primitiveData.textures.at(MeshUtils::TextureType::Metallic)->CreateView());
									  if (primitiveData.textures.contains(MeshUtils::TextureType::Roughness))
										  material.SetTexture(L"g_roughnessTexture", primitiveData.textures.at(MeshUtils::TextureType::Roughness)->CreateView());

									  material.SetVertexShader({
										  .filepath = L"shaders/DeferredBasePass.hlsl",
										  .entryName = L"vsmain",
										  .shaderType = Render::ShaderType::VS
									  });
									  material.SetPixelShader({
										  .filepath = L"shaders/DeferredBasePass.hlsl",
										  .entryName = L"psmain",
										  .shaderType = Render::ShaderType::PS
									  });
									  material.SetRasterizerState({
										  .cullMode = Render::CullMode::Back
									  });
									  material.SetDepthStencilState({
										  .depthEnable = true,
										  .depthWriteEnable = true
									  });
									  material.SetTopologyType(Render::TopologyType::TriangleList);

									  return material;
								  }, L"g_modelBuffer", sizeof(CBufferModel), &meshData);

	return batch;
}
