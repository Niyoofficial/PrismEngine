#include "Sandbox.h"

#include "imgui.h"
#include "Prism/Base/Base.h"
#include "Prism/Base/Platform.h"
#include "Prism/Base/Window.h"
#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderContext.h"
#include "Prism/Render/RenderDevice.h"
#include "Prism/Render/RenderUtils.h"
#include "Prism/Render/Texture.h"
#include "Prism/Render/TextureView.h"

IMPLEMENT_APPLICATION(SandboxApplication);

SandboxLayer::SandboxLayer(Core::Window* owningWindow)
	: m_owningWindow(owningWindow)
{
	using namespace Prism::Render;
	Layer::Attach();

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::WindowResized>(
		[this](Core::AppEvent event)
		{
			m_depthStencil = Texture::Create({
												 .textureName = L"DepthStencil",
												 .width = m_owningWindow->GetSize().x,
												 .height = m_owningWindow->GetSize().y,
												 .dimension = ResourceDimension::Tex2D,
												 .format = TextureFormat::D24_UNorm_S8_UInt,
												 .bindFlags = BindFlags::DepthStencil,
												 .optimizedClearValue = DepthStencilClearValue{
													 .format = TextureFormat::D24_UNorm_S8_UInt
												 }
											 }, BarrierLayout::DepthStencilWrite);
			m_depthStencilView = m_depthStencil->CreateView({
				.type = TextureViewType::DSV,
				.dimension = ResourceDimension::Tex2D,
			});

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
			m_cameraSpeed = std::max(m_cameraSpeed, minMoveSpeed);
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
	m_camera->SetPosition({0.f, 2.f, -5.f});

	m_depthStencil = Texture::Create({
										 .textureName = L"DepthStencil",
										 .width = windowSize.x,
										 .height = windowSize.y,
										 .dimension = ResourceDimension::Tex2D,
										 .format = TextureFormat::D24_UNorm_S8_UInt,
										 .bindFlags = BindFlags::DepthStencil,
										 .optimizedClearValue = DepthStencilClearValue{
											 .format = TextureFormat::D24_UNorm_S8_UInt
										 }
									 }, BarrierLayout::DepthStencilWrite);
	m_depthStencilView = m_depthStencil->CreateView({
		.type = TextureViewType::DSV,
		.dimension = ResourceDimension::Tex2D,
	});

	m_skybox = Texture::Create({
		.textureName = L"Skybox",
		.width = 1024,
		.height = 1024,
		.depthOrArraySize = 6,
		.dimension = ResourceDimension::TexCube,
		.format = TextureFormat::RGBA32_Float,
		.bindFlags = Flags(BindFlags::ShaderResource) | Flags(BindFlags::UnorderedAccess),
		.optimizedClearValue = {}
	});
	m_skyboxCubeSRVView = m_skybox->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::TexCube,
		.firstMipLevel = 0,
		.numMipLevels = 1,
		.firstArrayOrDepthSlice = 0,
		.arrayOrDepthSlicesCount = 6
	});
	m_skyboxArraySRVView = m_skybox->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::Tex2D,
		.firstMipLevel = 0,
		.numMipLevels = 1,
		.firstArrayOrDepthSlice = 0,
		.arrayOrDepthSlicesCount = 6
	});
	m_skyboxUAVView = m_skybox->CreateView({
		.type = TextureViewType::UAV,
		.dimension = ResourceDimension::TexCube,
		.firstMipLevel = 0,
		.numMipLevels = 1,
		.firstArrayOrDepthSlice = 0,
		.arrayOrDepthSlicesCount = 6
	});

	m_prefilteredEnvMap = Texture::Create({
		.textureName = L"PrefilteredEnvMap",
		.width = 4096,
		.height = 4096,
		.depthOrArraySize = 6,
		.mipLevels = 6,
		.dimension = ResourceDimension::TexCube,
		.format = TextureFormat::RGBA16_Float,
		.bindFlags = Flags(BindFlags::ShaderResource) | Flags(BindFlags::UnorderedAccess),
		.optimizedClearValue = {}
	});
	m_prefilteredEnvMapCubeSRVView = m_prefilteredEnvMap->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::TexCube,
		.firstMipLevel = 0,
		.numMipLevels = 6,
		.firstArrayOrDepthSlice = 0,
		.arrayOrDepthSlicesCount = 6
	});
	m_prefilteredEnvMapUAVView = m_prefilteredEnvMap->CreateView({
		.type = TextureViewType::UAV,
		.dimension = ResourceDimension::TexCube,
		.firstMipLevel = 0,
		.numMipLevels = 6,
		.firstArrayOrDepthSlice = 0,
		.arrayOrDepthSlicesCount = 6
	});

	m_irradiance = Texture::Create({
		.textureName = L"Irradiance",
		.width = 1024,
		.height = 1024,
		.depthOrArraySize = 6,
		.dimension = ResourceDimension::TexCube,
		.format = TextureFormat::RGBA16_Float,
		.bindFlags = Flags(BindFlags::ShaderResource) | Flags(BindFlags::UnorderedAccess),
		.optimizedClearValue = {}
	});
	m_irradianceSRVView = m_irradiance->CreateView({
		.type = TextureViewType::SRV,
		.dimension = ResourceDimension::TexCube,
		.firstMipLevel = 0,
		.numMipLevels = 1,
		.firstArrayOrDepthSlice = 0,
		.arrayOrDepthSlicesCount = 6
	});
	m_irradianceUAVView = m_irradiance->CreateView({
		.type = TextureViewType::UAV,
		.dimension = ResourceDimension::TexCube,
		.firstMipLevel = 0,
		.numMipLevels = 1,
		.firstArrayOrDepthSlice = 0,
		.arrayOrDepthSlicesCount = 6
	});

	m_rustedIronAlbedo = Texture::Create(L"textures/rustediron2_basecolor.png");
	m_rustedIronAlbedoView = m_rustedIronAlbedo->CreateView();
	m_rustedIronMetallic = Texture::Create(L"textures/rustediron2_metallic.png");
	m_rustedIronMetallicView = m_rustedIronMetallic->CreateView();
	m_rustedIronRoughness = Texture::Create(L"textures/rustediron2_roughness.png");
	m_rustedIronRoughnessView = m_rustedIronRoughness->CreateView();
	m_rustedIronNormal = Texture::Create(L"textures/rustediron2_normal.png");
	m_rustedIronNormalView = m_rustedIronNormal->CreateView();

	m_environmentTexture = Texture::Create(L"textures/tief_etz_4k.hdr");
	m_environmentTextureView = m_environmentTexture->CreateView();


	// Scene cbuffer
	m_sceneCbuffer = Buffer::Create({
										.bufferName = L"SceneCBuffer",
										.size = sizeof(CBufferScene),
										.bindFlags = BindFlags::ConstantBuffer,
										.usage = ResourceUsage::Dynamic,
										.cpuAccess = CPUAccess::Write
									});
	m_sceneCbufferView = m_sceneCbuffer->CreateDefaultCBVView();

	// Load sponza
	m_sponza = new PrimitiveBatch(L"Sponza", L"g_modelBuffer", sizeof(CBufferModel));

	auto sponzaData = MeshUtils::LoadMeshFromFile(L"meshes/SponzaCrytek/Sponza.gltf");
	auto sponzaMesh = SandboxApplication::ConvertMeshToSandboxFormat(sponzaData);

	for (auto& primitiveData : sponzaMesh.primitives)
	{
		Ref<TextureView> albedo = primitiveData.textures[MeshUtils::TextureType::Albedo]
									  ? primitiveData.textures[MeshUtils::TextureType::Albedo]->CreateView()
									  : nullptr;
		Ref<TextureView> normals = primitiveData.textures[MeshUtils::TextureType::Normals]
									   ? primitiveData.textures[MeshUtils::TextureType::Normals]->CreateView()
									   : nullptr;
		Ref<TextureView> metallic = primitiveData.textures[MeshUtils::TextureType::Metallic]
										? primitiveData.textures[MeshUtils::TextureType::Metallic]->CreateView()
										: nullptr;
		Ref<TextureView> roughness = primitiveData.textures[MeshUtils::TextureType::Roughness]
										 ? primitiveData.textures[MeshUtils::TextureType::Roughness]->CreateView()
										 : nullptr;

		m_sponza->AddPrimitive(sizeof(Vertex), IndexBufferFormat::Uint32,
							   primitiveData.vertices.data(), (int64_t)primitiveData.vertices.size(),
							   primitiveData.indices.data(), (int64_t)primitiveData.indices.size(),
							   {albedo, L"g_albedoTexture"}, {normals, L"g_normalTexture"},
							   {metallic, L"g_metallicTexture"}, {roughness, L"g_roughnessTexture"});
	}

	// Load cube
	m_cube = new PrimitiveBatch(L"Cube", L"g_modelBuffer", sizeof(CBufferModel));

	auto cubeData = MeshUtils::LoadMeshFromFile(L"meshes/Cube.fbx");
	auto cubeMesh = SandboxApplication::ConvertMeshToSandboxFormat(cubeData);

	for (auto& primitiveData : cubeMesh.primitives)
	{
		m_cube->AddPrimitive(sizeof(Vertex), IndexBufferFormat::Uint32,
							 primitiveData.vertices.data(), (int64_t)primitiveData.vertices.size(),
							 primitiveData.indices.data(), (int64_t)primitiveData.indices.size());
	}

	// Load sphere
	m_sphere = new PrimitiveBatch(L"Sphere", L"g_modelBuffer", sizeof(CBufferModel));

	auto sphereData = MeshUtils::LoadMeshFromFile(L"meshes/Sphere.gltf");
	auto sphereMesh = SandboxApplication::ConvertMeshToSandboxFormat(sphereData);

	for (auto& primitiveData : sphereMesh.primitives)
	{
		m_sphere->AddPrimitive(sizeof(Vertex), IndexBufferFormat::Uint32,
			primitiveData.vertices.data(), (int64_t)primitiveData.vertices.size(),
			primitiveData.indices.data(), (int64_t)primitiveData.indices.size());
	}

	{
		auto renderContext = RenderDevice::Get().AllocateContext();

		// HDRI to cubemap skybox
		{
			auto* pso = ComputePipelineState::Create({
				.cs = Shader::Create({
					.filepath = L"shaders/EquirectToCubemap.hlsl",
					.entryName = L"main",
					.shaderType = ShaderType::CS
				}),
			});
			renderContext->SetPSO(pso);

			renderContext->SetTexture(m_environmentTextureView, L"g_environment");
			renderContext->SetTexture(m_skyboxUAVView, L"g_skybox");

			renderContext->Dispatch(32, 32, 6);
		}

		// Generate env diffuse irradiance
		{
			m_coeffBuffer = Buffer::Create({
				.bufferName = L"SHCoefficientsGeneration",
				.size = sizeof(glm::float4) * 9, // RGB channels + one for padding * numOfCoefficients
				.bindFlags = BindFlags::UnorderedAccess,
				.usage = ResourceUsage::Default
			});
			m_coeffBufferView = m_coeffBuffer->CreateDefaultUAVView(sizeof(glm::float4) * 9); // Single element

			renderContext->SetPSO(ComputePipelineState::Create({
				.cs = Shader::Create({
					.filepath = L"shaders/DiffuseIrradianceIntegration.hlsl",
					.entryName = L"main",
					.shaderType = ShaderType::CS
				}),
			}));

			renderContext->SetTexture(m_skyboxArraySRVView, L"g_skybox");
			renderContext->SetBuffer(m_coeffBufferView, L"g_coefficients");

			renderContext->Dispatch(1, 1, 1);

			m_irradianceSHBuffer = Buffer::Create({
				.bufferName = L"IrradianceSH",
				.size = sizeof(glm::float3) * 9, // RGB channels * numOfCoefficients
				.bindFlags = BindFlags::ConstantBuffer,
				.usage = ResourceUsage::Default
			});
			m_irradianceSHBufferView = m_irradianceSHBuffer->CreateDefaultCBVView();

			renderContext->CopyBufferRegion(m_irradianceSHBuffer, 0, m_coeffBuffer, 0, m_coeffBuffer->GetBufferDesc().size);
		}

		// Generate env specular irradiance
		{
			renderContext->SetPSO(ComputePipelineState::Create({
				.cs = Shader::Create({
					.filepath = L"shaders/SpecularIrradianceIntegration.hlsl",
					.entryName = L"main",
					.shaderType = ShaderType::CS
				}),
			}));
		}

		RenderDevice::Get().SubmitContext(renderContext);
		RenderDevice::Get().GetRenderQueue()->Flush();
	}
}

void SandboxLayer::UpdateImGui(Duration delta)
{
	Layer::UpdateImGui(delta);

	// Menu bar
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Show"))
		{
			ImGui::MenuItem("Show stat window", nullptr, &m_showStatWindow);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// Stats overlay
	if (m_showStatWindow)
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

	{
		ImGui::Begin("Debug");

		ImGui::SliderFloat("Environment diffuse scale", &m_environmentDiffuseScale, 0.f, 1.f);

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

	Ref<RenderContext> renderContext = RenderDevice::Get().AllocateContext();

	glm::float2 windowSize = SandboxApplication::Get().GetWindow()->GetSize();
	auto* currentBackBuffer = SandboxApplication::Get().GetWindow()->GetSwapchain()->GetCurrentBackBufferRTV();

	renderContext->SetViewport({{0.f, 0.f}, windowSize, {0.f, 1.f}});
	renderContext->SetScissor({{0.f, 0.f}, windowSize});
	renderContext->SetRenderTarget(currentBackBuffer, m_depthStencilView);

	glm::float4 clearColor = {0.8f, 0.2f, 0.3f, 1.f};
	renderContext->ClearRenderTargetView(currentBackBuffer, &clearColor);
	renderContext->ClearDepthStencilView(m_depthStencilView, Flags(ClearFlags::ClearDepth) | Flags(ClearFlags::ClearStencil));

	// HDRI to cubemap skybox
	{
		auto* pso = ComputePipelineState::Create({
			.cs = Shader::Create({
				.filepath = L"shaders/EquirectToCubemap.hlsl",
				.entryName = L"main",
				.shaderType = ShaderType::CS
			}),
		});
		renderContext->SetPSO(pso);

		renderContext->SetTexture(m_environmentTextureView, L"g_environment");
		renderContext->SetTexture(m_skyboxUAVView, L"g_skybox");

		renderContext->Dispatch(32, 32, 6);
	}

	// Scene cbuffer
	{
		CBufferScene cbufferScene = {
			.environmentDiffuseScale = m_environmentDiffuseScale,
			.camera = {
				.view = m_camera->GetViewMatrix(),
				.proj = m_camera->GetProjectionMatrix(),
				.viewProj = m_camera->GetViewProjectionMatrix(),

				.camPos = m_camera->GetPosition()
			},
			.directionalLights = {
				{.direction = {0.f, -1.f, -1.f}, .lightColor = {1.f, 1.f, 1.f}}
			}
			//.pointLights = {
			//	{.position = {0.f, 3.f, -2.f}, .lightColor = {1.f, 1.f, 1.f}},
			//	{.position = {2.f, 3.f, -1.f}, .lightColor = {1.f, 0.f, 0.f}}
			//}
		};


		void* sceneCBufferData = m_sceneCbuffer->Map(CPUAccess::Write);
		memcpy_s(sceneCBufferData, m_sceneCbuffer->GetBufferDesc().size, &cbufferScene, sizeof(cbufferScene));
		m_sceneCbuffer->Unmap();

		renderContext->SetBuffer(m_sceneCbufferView, L"g_sceneBuffer");
	}

	renderContext->SetBuffer(m_irradianceSHBufferView, L"g_irradiance");

	// Skybox
	{
		auto* pso = GraphicsPipelineState::Create({
			.vs = Shader::Create({
				.filepath = L"shaders/Skybox.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			}),
			.ps = Shader::Create({
				.filepath = L"shaders/Skybox.hlsl",
				.entryName = L"psmain",
				.shaderType = ShaderType::PS
			}),
			.rasterizerState = {
				.cullMode = CullMode::Front
			},
			.depthStencilState = {
				.depthEnable = true,
				.depthWriteEnable = true,
				.depthFunc = ComparisionFunction::LessEqual
			},
			.primitiveTopologyType = TopologyType::TriangleList,
			.numRenderTargets = 1,
			.renderTargetFormats = {TextureFormat::RGBA8_UNorm},
			.depthStencilFormat = m_depthStencil->GetTextureDesc().format
		});

		renderContext->SetPSO(pso);

		renderContext->SetTexture(m_skyboxCubeSRVView, L"g_skybox");

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

	renderContext->SetTexture(m_rustedIronAlbedoView, L"g_albedoTexture");
	renderContext->SetTexture(m_rustedIronMetallicView, L"g_metallicTexture");
	renderContext->SetTexture(m_rustedIronRoughnessView, L"g_roughnessTexture");
	renderContext->SetTexture(m_rustedIronNormalView, L"g_normalTexture");

	// Sponza
	{
		auto* pso = GraphicsPipelineState::Create({
			.vs = Shader::Create({
				.filepath = L"shaders/PBR.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			}),
			.ps = Shader::Create({
				.filepath = L"shaders/PBR.hlsl",
				.entryName = L"psmain",
				.shaderType = ShaderType::PS
			}),
			.rasterizerState = {
				.cullMode = CullMode::Back
			},
			.depthStencilState = {
				.depthEnable = true,
				.depthWriteEnable = true
			},
			.primitiveTopologyType = TopologyType::TriangleList,
			.numRenderTargets = 1,
			.renderTargetFormats = {TextureFormat::RGBA8_UNorm},
			.depthStencilFormat = m_depthStencil->GetTextureDesc().format
		});
		renderContext->SetPSO(pso);

		CBufferModel modelData = {
			.world = glm::scale(glm::float4x4(1.f), glm::float3(1.f, 1.f, 1.f)),
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


	// Sphere
	{
		auto* pso = GraphicsPipelineState::Create({
			.vs = Shader::Create({
				.filepath = L"shaders/PBR.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			}),
			.ps = Shader::Create({
				.filepath = L"shaders/PBR.hlsl",
				.entryName = L"psmain",
				.shaderType = ShaderType::PS
			}),
			.rasterizerState = {
				.cullMode = CullMode::Back
			},
			.depthStencilState = {
				.depthEnable = true,
				.depthWriteEnable = true
			},
			.primitiveTopologyType = TopologyType::TriangleList,
			.numRenderTargets = 1,
			.renderTargetFormats = {TextureFormat::RGBA8_UNorm},
			.depthStencilFormat = m_depthStencil->GetTextureDesc().format
			});
		renderContext->SetPSO(pso);

		CBufferModel modelData = {
			.world = glm::translate(glm::float4x4(1.f), glm::float3(0.f, -5.f, 0.f)),
			.normalMatrix = glm::transpose(glm::inverse(glm::float3x3(modelData.world))),

			.material = {
				.albedo = glm::float3(1.f, 1.f, 1.f),
				.metallic = 0.f,
				.roughness = 0.1f,
				.ao = 0.3f
			}
		};
		m_sphere->Draw(renderContext, &modelData, sizeof(CBufferModel));
	}


	RenderDevice::Get().SubmitContext(renderContext);
}

SandboxApplication& SandboxApplication::Get()
{
	return Application::Get<SandboxApplication>();
}

SandboxApplication::SandboxApplication(int32_t argc, char** argv)
{
	InitPlatform();
	InitRenderer({.enableDebugLayer = true, .initPixLibrary = false});

	Core::WindowDesc windowParams = {
		.windowTitle = L"Test",
		.windowSize = {2560, 1440},
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

SandboxApplication::MeshData SandboxApplication::ConvertMeshToSandboxFormat(const MeshUtils::MeshData& mesh)
{
	MeshData outMesh;
	for (const auto& [vertices, indices, textures] : mesh.primitives)
	{
		PrimitiveData outPrimitive;
		outPrimitive.vertices.reserve(vertices.size());
		for (const MeshUtils::VertexData& vertex : vertices)
		{
			outPrimitive.vertices.push_back({
				.position = vertex.position,
				.normal = vertex.normal,
				.tangent = vertex.tangent,
				.bitangent = vertex.bitangent,
				.color = vertex.vertexColor,
				.texCoords = vertex.texCoord
			});
		}
		outPrimitive.indices.reserve(indices.size());
		for (uint32_t index : indices)
			outPrimitive.indices.push_back(index);

		outPrimitive.textures = textures;

		outMesh.primitives.push_back(outPrimitive);
	}

	return outMesh;
}
