#include "Sandbox.h"

#include "imgui.h"
#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Platform.h"
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/RenderCommandQueue.h"
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/RenderDevice.h"
#include "Prism-Core/Render/RenderUtils.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/TextureView.h"

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

	m_rustedIronAlbedo = Texture::Create(L"Textures/rustediron2_basecolor.png");
	m_rustedIronAlbedoView = m_rustedIronAlbedo->CreateView();
	m_rustedIronMetallic = Texture::Create(L"Textures/rustediron2_metallic.png");
	m_rustedIronMetallicView = m_rustedIronMetallic->CreateView();
	m_rustedIronRoughness = Texture::Create(L"Textures/rustediron2_roughness.png");
	m_rustedIronRoughnessView = m_rustedIronRoughness->CreateView();
	m_rustedIronNormal = Texture::Create(L"Textures/rustediron2_normal.png");
	m_rustedIronNormalView = m_rustedIronNormal->CreateView();

	m_environmentTexture = Texture::Create(L"Textures/tief_etz_4k.hdr");
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

	// Load monkey
	ShapeUtils::ShapeData monkey = ShapeUtils::LoadShapeFromFile(L"Meshes/Monkey.fbx");
	std::vector<Vertex> monkeyVertices = SandboxApplication::GetVerticesFromShapeData(monkey);
	std::vector<uint32_t> monkeyIndices = SandboxApplication::GetIndicesFromShapeData(monkey);

	m_monkey = new Primitive(L"Monkey", sizeof(Vertex), IndexBufferFormat::Uint32,
							 monkeyVertices.data(), (int64_t)monkeyVertices.size(),
							 monkeyIndices.data(), (int64_t)monkeyIndices.size(),
							 L"ModelBuffer", sizeof(CBufferModel));

	// Load floor
	ShapeUtils::ShapeData floor = ShapeUtils::LoadShapeFromFile(L"Meshes/Floor.fbx");
	std::vector<Vertex> floorVertices = SandboxApplication::GetVerticesFromShapeData(floor);
	std::vector<uint32_t> floorIndices = SandboxApplication::GetIndicesFromShapeData(floor);

	m_floor = new Primitive(L"Floor", sizeof(Vertex), IndexBufferFormat::Uint32,
							floorVertices.data(), (int64_t)floorVertices.size(),
							floorIndices.data(), (int64_t)floorIndices.size(),
							L"ModelBuffer", sizeof(CBufferModel));

	// Load cube
	ShapeUtils::ShapeData cube = ShapeUtils::LoadShapeFromFile(L"Meshes/Cube.fbx");
	std::vector<Vertex> cubeVertices = SandboxApplication::GetVerticesFromShapeData(cube);
	std::vector<uint32_t> cubeIndices = SandboxApplication::GetIndicesFromShapeData(cube);

	m_cube = new Primitive(L"Cube", sizeof(Vertex), IndexBufferFormat::Uint32,
						   cubeVertices.data(), (int64_t)cubeVertices.size(),
						   cubeIndices.data(), (int64_t)cubeIndices.size(),
						   L"ModelBuffer", sizeof(CBufferModel));

	{
		auto renderContext = RenderDevice::Get().AllocateContext();

		// HDRI to cubemap skybox
		{
			auto* pso = ComputePipelineState::Create({
				.cs = Shader::Create({
					.filepath = L"Shaders/EquirectToCubemap.hlsl",
					.entryName = L"main",
					.shaderType = ShaderType::CS
				}),
			});
			renderContext->SetPSO(pso);

			renderContext->SetTexture(m_environmentTextureView, L"g_environment");
			renderContext->SetTexture(m_skyboxUAVView, L"g_skybox");

			renderContext->Dispatch(32, 32, 6);
		}

		// Generate SH coefficients
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
					.filepath = L"Shaders/GenSHCoefficients.hlsl",
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

		RenderDevice::Get().SubmitContext(renderContext);
		RenderDevice::Get().GetRenderQueue()->Flush();
	}
}

void SandboxLayer::Update(Duration delta)
{
	using namespace Prism::Render;
	Layer::Update(delta);

	//bool showDemo = true;
	//ImGui::ShowDemoWindow(&showDemo);

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
		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, {1.f, 0.f});
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

	renderContext->Barrier({
		.texture = currentBackBuffer->GetTexture(),
		.syncBefore = BarrierSync::None,
		.syncAfter = BarrierSync::RenderTarget,
		.accessBefore = BarrierAccess::NoAccess,
		.accessAfter = BarrierAccess::RenderTarget,
		.layoutBefore = BarrierLayout::Present,
		.layoutAfter = BarrierLayout::RenderTarget
	});


	renderContext->SetViewport({{0.f, 0.f}, windowSize, {0.f, 1.f}});
	renderContext->SetScissor({{0.f, 0.f}, windowSize});
	renderContext->SetRenderTarget(currentBackBuffer, m_depthStencilView);

	glm::float4 clearColor = {0.8f, 0.2f, 0.3f, 1.f};
	renderContext->ClearRenderTargetView(currentBackBuffer, &clearColor);
	renderContext->ClearDepthStencilView(m_depthStencilView, Flags(ClearFlags::ClearDepth) | Flags(ClearFlags::ClearStencil));

	// Scene cbuffer
	{
		CBufferScene cbufferScene = {
			.camera = {
				.view = m_camera->GetViewMatrix(),
				.proj = m_camera->GetProjectionMatrix(),
				.viewProj = m_camera->GetViewProjectionMatrix(),

				.camPos = m_camera->GetPosition()
			},
			.directionalLights = {
				{.direction = {0.f, -1.f, 1.f}, .lightColor = {0.025f, 0.025f, 0.025f}}
			},
			.pointLights = {
				{.position = {0.f, 2.f, -2.f}, .lightColor = {1.f, 1.f, 1.f}},
				{.position = {2.f, 2.f, -1.f}, .lightColor = {1.f, 0.f, 0.f}}
			}
		};


		void* sceneCBufferData = m_sceneCbuffer->Map(CPUAccess::Write);
		memcpy_s(sceneCBufferData, m_sceneCbuffer->GetBufferDesc().size, &cbufferScene, sizeof(cbufferScene));
		m_sceneCbuffer->Unmap();

		renderContext->SetBuffer(m_sceneCbufferView, L"SceneBuffer");
	}

	renderContext->SetBuffer(m_irradianceSHBufferView, L"SceneIrradiance");

	// Skybox
	{
		auto* pso = GraphicsPipelineState::Create({
			.vs = Shader::Create({
				.filepath = L"Shaders/Skybox.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			}),
			.ps = Shader::Create({
				.filepath = L"Shaders/Skybox.hlsl",
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

			.useAlbedoTexture = false,
			.useMetallicTexture = false,
			.useRoughnessTexture = false,
			.material = {
				.albedo = glm::float3(0.2f, 0.3f, 0.8f),
				.metallic = 0.2f,
				.roughness = 0.4f,
				.ao = 0.3f
			}
		};
		m_cube->BindPrimitive(renderContext, &modelData, sizeof(CBufferModel));
		m_cube->DrawPrimitive(renderContext);
	}

	renderContext->SetTexture(m_rustedIronAlbedoView, L"g_albedoTexture");
	renderContext->SetTexture(m_rustedIronMetallicView, L"g_metallicTexture");
	renderContext->SetTexture(m_rustedIronRoughnessView, L"g_roughnessTexture");
	renderContext->SetTexture(m_rustedIronNormalView, L"g_normalTexture");

	// Monkey
	{
		auto* pso = GraphicsPipelineState::Create({
			.vs = Shader::Create({
				.filepath = L"Shaders/PBR.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			}),
			.ps = Shader::Create({
				.filepath = L"Shaders/PBR.hlsl",
				.entryName = L"psmain",
				.shaderType = ShaderType::PS
			}),
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
			.world = glm::translate(glm::float4x4(1.f), glm::float3(0.f, 2.f, 0.f)),

			.useAlbedoTexture = true,
			.useMetallicTexture = true,
			.useRoughnessTexture = true,
			.useNormalTexture = true,
			.material = {
				.albedo = glm::float3(1.f, 1.f, 1.f),
				.metallic = 1.f,
				.roughness = 1.f,
				.ao = 0.3f
			}
		};
		m_monkey->BindPrimitive(renderContext, &modelData, sizeof(CBufferModel));
		m_monkey->DrawPrimitive(renderContext);
	}

	// Floor
	{
		auto* pso = GraphicsPipelineState::Create({
			.vs = Shader::Create({
				.filepath = L"Shaders/PBR.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			}),
			.ps = Shader::Create({
				.filepath = L"Shaders/PBR.hlsl",
				.entryName = L"psmain",
				.shaderType = ShaderType::PS
			}),
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
			.world = glm::float4x4(1.f),

			.useAlbedoTexture = true,
			.useMetallicTexture = true,
			.useRoughnessTexture = true,
			.useNormalTexture = true,
			.material = {
				.albedo = glm::float3(1.f, 1.f, 1.f),
				.metallic = 1.f,
				.roughness = 1.f,
				.ao = 0.3f
			}
		};
		m_floor->BindPrimitive(renderContext, &modelData, sizeof(CBufferModel));
		m_floor->DrawPrimitive(renderContext);
	}

	renderContext->RenderImGui();

	renderContext->Barrier({
		.texture = currentBackBuffer->GetTexture(),
		.syncBefore = BarrierSync::RenderTarget,
		.syncAfter = BarrierSync::None,
		.accessBefore = BarrierAccess::RenderTarget,
		.accessAfter = BarrierAccess::NoAccess,
		.layoutBefore = BarrierLayout::RenderTarget,
		.layoutAfter = BarrierLayout::Present
	});

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
	//Core::Platform::Get().SetMouseRelativeMode(m_window, true);

	InitImGui(m_window);

	m_sandboxLayer = new SandboxLayer(m_window);
	PushLayer(m_sandboxLayer);
}

Core::Window* SandboxApplication::GetWindow() const
{
	return m_window;
}

std::vector<Vertex> SandboxApplication::GetVerticesFromShapeData(const ShapeUtils::ShapeData& shapeData)
{
	std::vector<Vertex> vertices;
	vertices.reserve(shapeData.vertices.size());
	for (const ShapeUtils::VertexData& vertex : shapeData.vertices)
	{
		vertices.push_back({
			.position = vertex.position,
			.normal = vertex.normal,
			.tangent = vertex.tangent,
			.color = vertex.vertexColor,
			.texCoords = vertex.texCoord
		});
	}

	return vertices;
}

std::vector<uint32_t> SandboxApplication::GetIndicesFromShapeData(const ShapeUtils::ShapeData& shapeData)
{
	std::vector<uint32_t> indices;
	indices.reserve(shapeData.indices.size());
	for (uint32_t index : shapeData.indices)
		indices.push_back(index);

	return indices;
}
