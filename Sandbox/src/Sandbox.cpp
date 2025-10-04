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
#include "Prism/Render/PBR/PBRSceneRenderPipeline.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/LightRendererComponent.h"
#include "Prism/Scene/MeshRendererComponent.h"

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

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::KeyDown>(
		[](Core::AppEvent event)
		{
			if (std::get<Core::AppEvents::KeyDown>(event).keyCode == KeyCode::Escape)
				SandboxApplication::Get().CloseApplication();
		});

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::MouseMotion>(
		[this](Core::AppEvent event)
		{
			if (m_viewportRelativeMouse && Core::Platform::Get().IsKeyPressed(KeyCode::RightMouseButton))
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
			constexpr float moveSpeedChangeMult = 0.5f;
			constexpr float minMoveSpeed = 0.5f;
			m_cameraSpeed += delta * moveSpeedChangeMult;
			m_cameraSpeed = std::max(m_cameraSpeed, minMoveSpeed);
		});

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::MouseButtonDown>(
		[this](Core::AppEvent event)
		{
			if (m_owningWindow.IsValid() && m_viewportHovered && std::get<Core::AppEvents::MouseButtonDown>(event).keyCode == KeyCode::RightMouseButton)
			{
				m_viewportRelativeMouse = true;
				ImGui::SetWindowFocus("Viewport");
				Core::Platform::Get().SetMouseRelativeMode(m_owningWindow.Raw(), true);
			}
		});
	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::MouseButtonUp>(
		[this](Core::AppEvent event)
		{
			if (m_owningWindow.IsValid() && m_viewportRelativeMouse && std::get<Core::AppEvents::MouseButtonUp>(event).keyCode == KeyCode::RightMouseButton)
			{
				m_viewportRelativeMouse = false;
				Core::Platform::Get().SetMouseRelativeMode(m_owningWindow.Raw(), false);
			}
		});

	// TODO: Add create function on Ref class and make RefCounted not allow object creation on stack
	Ref sponza = new MeshLoading::MeshAsset(L"meshes/SponzaCrytek/Sponza.gltf");

	m_scene = Scene::Create(L"Test Scene");
	m_scene->SetRenderPipeline(new PBRSceneRenderPipeline);
	m_scene->CreateEntityHierarchyForMeshAsset(sponza);
	m_scene->AddEntity(L"Light")->AddComponent<LightRendererComponent>();

	glm::int2 windowSize = SandboxApplication::Get().GetWindow()->GetSize();

	m_camera = new Camera(45.f, (float)windowSize.x / (float)windowSize.y, 0.1f, 10000.f);
	m_camera->SetPosition({0.f, 0.f, 0.f});

	/*
	// Scene cbuffer
	m_sceneUniBuffer = Buffer::Create({
										.bufferName = L"SceneCBuffer",
										.size = sizeof(SceneUniformBuffer),
										.bindFlags = BindFlags::UniformBuffer,
										.usage = ResourceUsage::Dynamic,
										.cpuAccess = CPUAccess::Write
									});
	m_sceneUniBufferView = m_sceneUniBuffer->CreateDefaultUniformBufferView();
	*/

	/*
	// Lights uniform buffer
	m_lightsUniBuffer = Buffer::Create({
		.bufferName = L"LightsBuffer",
		.size = sizeof(LightsUniformBuffer),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});
	m_lightsUniBufferView = m_lightsUniBuffer->CreateDefaultUniformBufferView();
	*/

	// PerLight uniform buffer
	m_perLightUniBuffer = Buffer::Create({
		.bufferName = L"PerLightBuffer",
		.size = sizeof(PerLightUniformBuffer),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});
	m_perLightUniBufferView = m_perLightUniBuffer->CreateDefaultUniformBufferView();


	m_bloomSettingsBuffer = Buffer::Create({
		.bufferName = L"BloomSettingsBuffer",
		.size = sizeof(BloomSettingsUniformBuffer),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});
	m_bloomSettingsBufferView = m_bloomSettingsBuffer->CreateDefaultUniformBufferView();

	// Load sponza
	m_sponza = SandboxApplication::LoadMeshFromFilePBR(L"Sponza", L"meshes/SponzaCrytek/Sponza.gltf");

	/*
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

			material.SetTexture(L"g_skybox", m_prefilteredEnvMapCubeSRV);
			return material;
		}, L"g_modelBuffer", sizeof(ModelUniformBuffer));
		*/

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
		}, L"g_modelBuffer", sizeof(ModelUniformBuffer));

	
}

void SandboxLayer::UpdateImGui(Duration delta)
{
	using namespace Prism::Render;
	Layer::UpdateImGui(delta);

	static bool s_showImGuiDemoWindow = false;
	static bool s_showStatWindow = true;
	static bool s_showDebugMenu = true;
	static bool s_showLogMenu = true;
	static bool s_showSceneOutline = true;

	// Menu bar
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Show"))
		{
			ImGui::MenuItem("Show scene hierarchy", nullptr, &s_showSceneOutline);
			ImGui::MenuItem("Show stat window", nullptr, &s_showStatWindow);
			ImGui::MenuItem("Show debug window", nullptr, &s_showDebugMenu);
			ImGui::MenuItem("Show log window", nullptr, &s_showLogMenu);
			ImGui::MenuItem("Show ImGui demo window", nullptr, &s_showImGuiDemoWindow);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();

		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Recompile Shaders"))
				RenderDevice::Get().GetShaderCompiler()->RecompileCachedShaders();

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// ImGui demo
	if (s_showImGuiDemoWindow)
	{
		ImGui::ShowDemoWindow(&s_showImGuiDemoWindow);
	}

	// Viewport
	{
		ImGui::DockSpaceOverViewport();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		ImGui::Begin("Viewport");

		m_viewportHovered = ImGui::IsWindowHovered();

		auto viewportSize = ImGui::GetContentRegionAvail();
		if (CheckForViewportResize({viewportSize.x, viewportSize.y}))
		{
			float viewportStartHeight = ImGui::GetCursorPos().y;

			ImGui::Image(m_finalCompositionSRV, viewportSize);

			// Stats overlay
			if (s_showStatWindow)
			{
				ImGuiWindowFlags windowFlags =
					ImGuiWindowFlags_NoDecoration |
					ImGuiWindowFlags_NoDocking |
					ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoFocusOnAppearing |
					ImGuiWindowFlags_NoNav |
					ImGuiWindowFlags_NoMove;

				ImGuiChildFlags childFlags =
					ImGuiChildFlags_FrameStyle |
					ImGuiChildFlags_AutoResizeY;

				constexpr float padding = 10.0f;
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImVec2 workPos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
				ImVec2 workSize = viewport->WorkSize;
				ImVec2 windowPos;
				windowPos.x = workPos.x + workSize.x - padding;
				windowPos.y = workPos.y + padding + viewportStartHeight;
				ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, { 1.f, 0.f });
				ImGui::SetNextWindowBgAlpha(0.55f); // Transparent background

				if (ImGui::BeginChild("Stats overlay", { 250, 0 }, childFlags, windowFlags))
				{
					auto& io = ImGui::GetIO();
					ImGui::Text("FPS: %.1f", io.Framerate);
					ImGui::Text("Frame time: %.2f ms", delta.GetMilliseconds());
				}
				ImGui::EndChild();
			}
		}

		ImGui::End();

		ImGui::PopStyleVar();
	}

	// Debug menu
	/*if (s_showDebugMenu)
	{
		ImGui::Begin("Debug", &s_showDebugMenu, ImGuiWindowFlags_HorizontalScrollbar);

		if (ImGui::CollapsingHeader("GBuffer"))
		{
			float texWidth = (float)m_gbuffer.GetTexture(GBuffer::Type::Color)->GetTextureDesc().GetWidth() / (float)m_gbuffer.GetTexture(GBuffer::Type::Color)->GetTextureDesc().GetHeight() * 256.f;
			ImGui::Text("Color");
			ImGui::Image(m_gbuffer.GetView(GBuffer::Type::Color, TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("Depth");
			ImGui::Image(m_gbuffer.GetView(GBuffer::Type::Depth, TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("Normal");
			ImGui::Image(m_gbuffer.GetView(GBuffer::Type::Normal, TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("R:Roughness G:Metal B:AO");
			static bool s_roughness = true;
			ImGui::Checkbox("Roughness", &s_roughness);
			ImGui::SameLine();
			static bool s_metallic = true;
			ImGui::Checkbox("Metallic", &s_metallic);
			ImGui::SameLine();
			static bool s_ao = true;
			ImGui::Checkbox("AO", &s_ao);
			ImGui::Image(m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::SRV), {texWidth, 256}, {0, 0}, {1, 1}, {(float)s_roughness, (float)s_metallic, (float)s_ao, 1});
		}

		if (ImGui::CollapsingHeader("Sun"))
		{
			//ImGui::SliderFloat("Environment light scale", &m_environmentLightScale, 0.f, 1.f);
			ImGui::SliderAngle("Sun Rotation Yaw", &m_sunRotation.y, -180.f, 180.f);
			ImGui::SliderAngle("Sun Rotation Pitch", &m_sunRotation.z, -180.f, 180.f);
			ImGui::Image(m_sunShadowMap->CreateView({ .type = TextureViewType::SRV, .format = TextureFormat::R32_Float }), {256, 256});
		}

		if (ImGui::CollapsingHeader("IBL"))
		{
			ImGui::Text("Original image");
			ImGui::Image(m_environmentTextureSRV, {(float)m_environmentTexture->GetTextureDesc().GetWidth() / (float)m_environmentTexture->GetTextureDesc().GetHeight() * 256.f, 256.f});

			ImGui::Text("Prefiltered Env Cube Map");
			static int32_t s_mipIndex = 0;
			ImGui::SliderInt("Mip Index", &s_mipIndex, 0, 5);

			auto viewDesc = TextureViewDesc{
				.type = TextureViewType::SRV,
				.dimension = ResourceDimension::Tex2D,
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
			ImGui::Image(m_BRDFLUT->CreateView({.type = TextureViewType::SRV}), {256, 256});
		}

		if (ImGui::CollapsingHeader("Bloom"))
		{

			ImGui::DragFloat("Threshold", &m_bloomThreshold, 0.005f);
			ImGui::DragFloat("Knee", &m_bloomKnee, 0.005f);

			ImGui::Text("Bloom downsample A");
			static int32_t s_downsampleAMipIndex = 0;
			ImGui::SliderInt("Downsample A Mip Index", &s_downsampleAMipIndex, 0, 6);

			float texWidth = (float)m_bloomDownsampleA->GetTextureDesc().GetWidth() / (float)m_bloomDownsampleA->GetTextureDesc().GetHeight() * 256.f;
			auto viewDesc = TextureViewDesc{
				.type = TextureViewType::SRV,
				.dimension = ResourceDimension::Tex2D,
				.subresourceRange = {.firstMipLevel = s_downsampleAMipIndex, .numMipLevels = 1}
			};
			ImGui::Image(m_bloomDownsampleA->CreateView(viewDesc), {texWidth, 256});

			ImGui::Text("Bloom downsample B");
			static int32_t s_downsampleBMipIndex = 0;
			ImGui::SliderInt("Downsample B Mip Index", &s_downsampleBMipIndex, 0, 6);

			texWidth = (float)m_bloomDownsampleB->GetTextureDesc().GetWidth() / (float)m_bloomDownsampleB->GetTextureDesc().GetHeight() * 256.f;
			viewDesc = TextureViewDesc{
				.type = TextureViewType::SRV,
				.dimension = ResourceDimension::Tex2D,
				.subresourceRange = {.firstMipLevel = s_downsampleBMipIndex, .numMipLevels = 1}
			};
			ImGui::Image(m_bloomDownsampleB->CreateView(viewDesc), {texWidth, 256});

			ImGui::Text("Bloom upsample");
			static int32_t s_upsampleMipIndex = 0;
			ImGui::SliderInt("Upsample Mip Index", &s_upsampleMipIndex, 0, 5);

			viewDesc = TextureViewDesc{
				.type = TextureViewType::SRV,
				.dimension = ResourceDimension::Tex2D,
				.subresourceRange = {.firstMipLevel = s_upsampleMipIndex, .numMipLevels = 1}
			};
			ImGui::Image(m_bloomUpsampleTexture->CreateView(viewDesc), {texWidth, 256});
		}

		ImGui::End();
	}*/

	// Log
	if (s_showLogMenu)
	{
		ImGui::Begin("Log", &s_showLogMenu);

		auto imguiSink = Log::LogsRegistry::Get().GetImGuiSink();

		if (ImGui::Button("Clear"))
			imguiSink->Clear();

		ImGui::Separator();

		if (ImGui::BeginChild("Child_Log", ImVec2(0, 0), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar) &&
			imguiSink->GetLogBuffer().size())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

			ImGuiListClipper clipper;
			clipper.Begin((int32_t)imguiSink->GetLineOffsets().size());
			while (clipper.Step())
			{
				glm::float3 lastLogColor = {1.f, 1.f, 1.f};
				for (int line = clipper.DisplayStart; line < clipper.DisplayEnd; line++)
				{
					const char* lineStart = imguiSink->GetLogBuffer().data() + imguiSink->GetLineOffsets()[line];
					const char* lineEnd = (line + 1 < imguiSink->GetLineOffsets().size()) ? (imguiSink->GetLogBuffer().data() + imguiSink->GetLineOffsets()[line + 1] - 1) : imguiSink->GetLogBuffer().end();

					std::string_view view(lineStart, lineEnd);

					auto lastDateBracket = view.find_first_of(']');
					auto firstLabelBracket = view.find_first_of('[', lastDateBracket);
					auto secLabelBracket = view.find_first_of(']', firstLabelBracket);
					if (firstLabelBracket != std::string_view::npos && secLabelBracket != std::string_view::npos)
					{
						std::string_view label(lineStart + firstLabelBracket + 1, lineStart + secLabelBracket);
						static std::unordered_map<std::string, glm::float3> colorMap = {
							{"error", {1.f, 0.f, 0.f}},
							{"warning", {1.f, 1.f, 0.f}},
							{"info", {0.f, 1.f, 0.f}},
							{"trace", {1.f, 1.f, 1.f}},
						};

						auto it = std::ranges::find_if(colorMap, 
							[&label](auto val)
							{
								return val.first == label;
							});
						glm::float3 logColor = it != colorMap.end() ? it->second : glm::float3{1.f, 1.f, 1.f};
						int64_t lineLength = imguiSink->GetLineOffsets()[line + 1] - imguiSink->GetLineOffsets()[line] - 1;
						ImGui::TextColored({logColor.r, logColor.g, logColor.b, 1.f}, "%.*s", lineLength, lineStart);
						lastLogColor = logColor;
					}
					else
					{
						if (imguiSink->GetLineOffsets().size() > line + 1)
						{
							int64_t lineLength = imguiSink->GetLineOffsets()[line + 1] - imguiSink->GetLineOffsets()[line] - 1;
							ImGui::TextColored({ lastLogColor.r, lastLogColor.g, lastLogColor.b, 1.f }, "%.*s", lineLength, lineStart);
						}
					}
				}
			}
			clipper.End();

			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 0.5f)
				ImGui::SetScrollHereY(1.0f);

			ImGui::PopStyleVar();
		}
		ImGui::EndChild();

		ImGui::End();
	}

	// Scene Hierarchy
	if (s_showSceneOutline)
	{
		if (ImGui::Begin("SceneOutline", &s_showSceneOutline, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (ImGui::BeginChild("SceneOutline_Hierarchy", {0, 0}))
			{
				if (ImGui::BeginTable("SceneHierarchy_Table", 1, ImGuiTableFlags_RowBg))
				{
					for (int32_t i = 0; i < m_scene->GetEntityCount(); ++i)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGuiTreeNodeFlags treeFlags =
							ImGuiTreeNodeFlags_OpenOnArrow |
							ImGuiTreeNodeFlags_OpenOnDoubleClick |
							ImGuiTreeNodeFlags_NavLeftJumpsBackHere |
							ImGuiTreeNodeFlags_SpanFullWidth |
							ImGuiTreeNodeFlags_Leaf;

						std::wstring entityName = m_scene->GetEntityByIndex(i)->GetName();
						if (ImGui::TreeNodeEx(std::to_string(i).c_str(), treeFlags, "%s", entityName.empty() ? "<unnamed>" : WStringToString(entityName).c_str()))
						{
							ImGui::TreePop();
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
}

void SandboxLayer::Update(Duration delta)
{
	using namespace Prism::Render;
	Layer::Update(delta);

	if (m_viewportSize.x == 0 || m_viewportSize.y == 0)
		return;

	if (m_viewportRelativeMouse && Core::Platform::Get().IsKeyPressed(KeyCode::RightMouseButton))
	{
		if (Core::Platform::Get().IsKeyPressed(KeyCode::W))
			m_camera->AddPosition(m_camera->GetForwardVector() * m_cameraSpeed * (float)delta.GetSeconds());
		if (Core::Platform::Get().IsKeyPressed(KeyCode::S))
			m_camera->AddPosition(-m_camera->GetForwardVector() * m_cameraSpeed * (float)delta.GetSeconds());
		if (Core::Platform::Get().IsKeyPressed(KeyCode::A))
			m_camera->AddPosition(-m_camera->GetRightVector() * m_cameraSpeed * (float)delta.GetSeconds());
		if (Core::Platform::Get().IsKeyPressed(KeyCode::D))
			m_camera->AddPosition(m_camera->GetRightVector() * m_cameraSpeed * (float)delta.GetSeconds());
		if (Core::Platform::Get().IsKeyPressed(KeyCode::Q))
			m_camera->AddPosition(-m_camera->GetUpVector() * m_cameraSpeed * (float)delta.GetSeconds());
		if (Core::Platform::Get().IsKeyPressed(KeyCode::E))
			m_camera->AddPosition(m_camera->GetUpVector() * m_cameraSpeed * (float)delta.GetSeconds());
	}

	m_scene->Update(delta);
	m_scene->RenderScene(m_finalCompositionRTV, m_camera);

	/*
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

		renderContext->SetViewport({{0.f, 0.f}, m_viewportSize, {0.f, 1.f}});
		renderContext->SetScissor({{0.f, 0.f}, m_viewportSize});

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
			ModelUniformBuffer modelData = {
				.world = glm::float4x4(1.f),
				.normalMatrix = glm::transpose(glm::inverse(modelData.world)),

				.material = {
					.albedo = glm::float3(1.f, 1.f, 1.f),
					.metallic = 1.f,
					.roughness = 1.f,
					.ao = 1.f
				}
			};
			m_sponza->Draw(renderContext, &modelData, sizeof(ModelUniformBuffer));
		}

		// Skybox
		if (0)
		{
			ModelUniformBuffer modelData = {
				.world = glm::float4x4(1.f),
				.normalMatrix = glm::transpose(glm::inverse(glm::float3x3(modelData.world))),

				.material = {
					.albedo = glm::float3(0.2f, 0.3f, 0.8f),
					.metallic = 0.2f,
					.roughness = 0.4f,
					.ao = 0.3f
				}
			};
			m_cube->Draw(renderContext, &modelData, sizeof(ModelUniformBuffer));
		}

		// Spheres
		if (0)
		{
			ModelUniformBuffer modelData = {
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
			
				m_sphere->Draw(renderContext, &modelData, sizeof(ModelUniformBuffer));
			}
		}

		renderContext->EndEvent();
	}

	// Lighting Pass
	{
		renderContext->BeginEvent({}, L"LightingPass");

		//auto* currentBackBuffer = SandboxApplication::Get().GetWindow()->GetSwapchain()->GetCurrentBackBufferRTV();
		renderContext->SetRenderTarget(m_sceneColorRTV, nullptr);

		glm::float4 clearColor = {0.f, 0.f, 0.f, 1.f};
		renderContext->ClearRenderTargetView(m_sceneColorRTV, &clearColor);

		renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Color, TextureViewType::SRV), L"g_colorTexture");
		renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Normal, TextureViewType::SRV), L"g_normalTexture");
		renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::SRV), L"g_roughnessMetalAOTexture");
		renderContext->SetTexture(m_gbuffer.GetView(GBuffer::Type::Depth, TextureViewType::SRV), L"g_depthTexture");

		// Env light
		{
			BlendStateDesc blendState;
			blendState.renderTargetBlendDesc = RenderTargetBlendDesc{.blendEnable = true, .destBlend = BlendFactor::One, .destBlendAlpha = BlendFactor::One};
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
			renderContext->SetTexture(m_prefilteredEnvMapCubeSRV, L"g_envMap");

			renderContext->Draw({.numVertices = 3});
		}

		BlendStateDesc blendState;
		blendState.renderTargetBlendDesc = RenderTargetBlendDesc{.blendEnable = true, .destBlend = BlendFactor::One, .destBlendAlpha = BlendFactor::One};
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


	// Bloom
	{
		renderContext->BeginEvent({}, L"Bloom");

		BloomSettingsUniformBuffer bloomSettingsUniformBuffer = {
			.threshold = m_bloomThreshold,
			.knee = m_bloomKnee,
			.lod = 0,
		};

		renderContext->SetBuffer(m_bloomSettingsBufferView, L"g_bloomSettings");

		// Prefilter
		{
			renderContext->BeginEvent({}, L"Prefilter");

			renderContext->SetTexture(m_sceneColorSRV, L"g_inputTexture");
			renderContext->SetTexture(m_bloomDownsampleA->CreateView({.type = TextureViewType::UAV}), L"g_outputTexture");

			void* bloomSettingsData = m_bloomSettingsBuffer->Map(CPUAccess::Write);
			memcpy_s(bloomSettingsData, m_bloomSettingsBuffer->GetBufferDesc().size, &bloomSettingsUniformBuffer, sizeof(bloomSettingsUniformBuffer));
			m_bloomSettingsBuffer->Unmap();

			renderContext->SetPSO(ComputePipelineStateDesc{
				.cs = {
					.filepath = L"shaders/Bloom.hlsl",
					.entryName = L"Prefilter",
					.shaderType = ShaderType::CS
				}
			});

			renderContext->Dispatch({m_bloomDownsampleA->GetTextureDesc().GetWidth() / 4, m_bloomDownsampleA->GetTextureDesc().GetHeight() / 4, 1});

			renderContext->EndEvent();
		}

		// Downsample
		{
			renderContext->BeginEvent({}, L"Downsample");

			auto dispatchDownsample =
				[&bloomSettingsUniformBuffer, &renderContext, this](Texture* uavTexture, int32_t uavMipIndex, TextureView* srv, int32_t srvMipReadIndex)
				{
					TextureViewDesc uavDesc = {
						.type = TextureViewType::UAV, .subresourceRange = {.firstMipLevel = uavMipIndex, .numMipLevels = 1}
					};
					renderContext->SetTexture(uavTexture->CreateView(uavDesc), L"g_outputTexture");
					renderContext->SetTexture(srv, L"g_inputTexture");

					bloomSettingsUniformBuffer.lod = srvMipReadIndex;
					void* bloomSettingsData = m_bloomSettingsBuffer->Map(CPUAccess::Write);
					memcpy_s(bloomSettingsData, m_bloomSettingsBuffer->GetBufferDesc().size, &bloomSettingsUniformBuffer,
							 sizeof(bloomSettingsUniformBuffer));
					m_bloomSettingsBuffer->Unmap();

					renderContext->SetPSO(ComputePipelineStateDesc{
						.cs = {
							.filepath = L"shaders/Bloom.hlsl",
							.entryName = L"Downsample",
							.shaderType = ShaderType::CS
						}
					});

					renderContext->Dispatch({
						std::ceil((float)m_bloomDownsampleA->GetTextureDesc().GetWidth() / std::pow(2.f, (float)uavMipIndex) / 4.f),
						std::ceil((float)m_bloomDownsampleA->GetTextureDesc().GetHeight() / std::pow(2.f, (float)uavMipIndex) / 4.f),
						1
					});
				};

			dispatchDownsample(m_bloomDownsampleB, 0, m_bloomDownsampleAsrv, 0);
			for (int32_t i = 1; i < m_bloomDownsampleA->GetTextureDesc().GetMipLevels(); ++i)
			{
				dispatchDownsample(m_bloomDownsampleA, i, m_bloomDownsampleBsrv, i - 1);
				dispatchDownsample(m_bloomDownsampleB, i, m_bloomDownsampleAsrv, i);
			}

			renderContext->EndEvent();
		}

		// Upsample
		{
			renderContext->BeginEvent({}, L"Upsample");

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
				renderContext->SetTexture(m_bloomDownsampleBsrv, L"g_inputTexture");
				renderContext->SetTexture(i == 5 ? m_bloomDownsampleBsrv : m_bloomUpsampleTextureSRV, L"g_accumulationTexture");

				bloomSettingsUniformBuffer.lod = i;
				void* bloomSettingsData = m_bloomSettingsBuffer->Map(CPUAccess::Write);
				memcpy_s(bloomSettingsData, m_bloomSettingsBuffer->GetBufferDesc().size, &bloomSettingsUniformBuffer, sizeof(bloomSettingsUniformBuffer));
				m_bloomSettingsBuffer->Unmap();

				renderContext->SetPSO(ComputePipelineStateDesc{
					.cs = {
						.filepath = L"shaders/Bloom.hlsl",
						.entryName = L"Upsample",
						.shaderType = ShaderType::CS
					}
				});

				renderContext->Dispatch({
					std::ceil((float)m_bloomUpsampleTexture->GetTextureDesc().GetWidth() / std::pow(2.f, (float)i) / 4.f),
					std::ceil((float)m_bloomUpsampleTexture->GetTextureDesc().GetHeight() / std::pow(2.f, (float)i) / 4.f),
					1
				});
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

	// Final composition
	{
		renderContext->ClearRenderTargetView(m_finalCompositionRTV);
		renderContext->SetRenderTarget(m_finalCompositionRTV, nullptr);

		renderContext->SetPSO(GraphicsPipelineStateDesc{
			.vs = {
				.filepath = L"shaders/FinalComposition.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			},
			.ps = {
				.filepath = L"shaders/FinalComposition.hlsl",
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

		renderContext->SetTexture(m_sceneColorSRV, L"g_sceneColorTexture");
		renderContext->SetTexture(m_bloomUpsampleTextureSRV, L"g_bloomTexture");

		renderContext->Draw({.numVertices = 3 });
	}
	*/
}

bool SandboxLayer::CheckForViewportResize(glm::int2 viewportSize)
{
	using namespace Render;

	if (viewportSize.x != m_viewportSize.x || viewportSize.y != m_viewportSize.y)
	{
		m_viewportSize = {viewportSize.x, viewportSize.y};

		PE_RENDER_LOG(Trace, "Viewport resized to x: {} y: {}", viewportSize.x, viewportSize.y);

		if (m_viewportSize.x == 0 || m_viewportSize.y == 0)
			return false;

		m_gbuffer.CreateResources(m_viewportSize);

		m_sceneColor = Texture::Create({
										   .textureName = L"SceneColor",
										   .width = m_viewportSize.x,
										   .height = m_viewportSize.y,
										   .dimension = ResourceDimension::Tex2D,
										   .format = TextureFormat::RGBA16_UNorm,
										   .bindFlags = Flags(BindFlags::RenderTarget) | Flags(BindFlags::ShaderResource),
										   .optimizedClearValue = RenderTargetClearValue{
											   .format = TextureFormat::RGBA16_UNorm,
											   .color = {0.f, 0.f, 0.f, 1.f}
										   }
									   }, BarrierLayout::RenderTarget);
		m_sceneColorRTV = m_sceneColor->CreateView({.type = TextureViewType::RTV});
		m_sceneColorSRV = m_sceneColor->CreateView({.type = TextureViewType::SRV});

		m_bloomDownsampleA = Texture::Create({
			.textureName = L"DownsampleBloomTextureA",
			.width = m_viewportSize.x,
			.height = m_viewportSize.y,
			.mipLevels = 7,
			.dimension = ResourceDimension::Tex2D,
			.format = TextureFormat::R11G11B10_Float,
			.bindFlags = Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::ShaderResource),
		}, BarrierLayout::UnorderedAccess);
		m_bloomDownsampleAsrv = m_bloomDownsampleA->CreateView({.type = TextureViewType::SRV});
		m_bloomDownsampleB = Texture::Create({
			.textureName = L"DownsampleBloomTextureB",
			.width = m_viewportSize.x,
			.height = m_viewportSize.y,
			.mipLevels = 7,
			.dimension = ResourceDimension::Tex2D,
			.format = TextureFormat::R11G11B10_Float,
			.bindFlags = Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::ShaderResource),
			}, BarrierLayout::UnorderedAccess);
		m_bloomDownsampleBsrv = m_bloomDownsampleB->CreateView({ .type = TextureViewType::SRV });
		m_bloomUpsampleTexture = Texture::Create({
			.textureName = L"UpsampleBloomTexture",
			.width = m_viewportSize.x,
			.height = m_viewportSize.y,
			.mipLevels = 6,
			.dimension = ResourceDimension::Tex2D,
			.format = TextureFormat::R11G11B10_Float,
			.bindFlags = Flags(BindFlags::UnorderedAccess) | Flags(BindFlags::ShaderResource),
		}, BarrierLayout::UnorderedAccess);
		m_bloomUpsampleTextureSRV = m_bloomUpsampleTexture->CreateView({.type = TextureViewType::SRV});

		m_finalComposition = Texture::Create({
										   .textureName = L"FinalComposition",
										   .width = m_viewportSize.x,
										   .height = m_viewportSize.y,
										   .dimension = ResourceDimension::Tex2D,
										   .format = TextureFormat::R11G11B10_Float,
										   .bindFlags = Flags(BindFlags::RenderTarget) | Flags(BindFlags::ShaderResource),
										   .optimizedClearValue = RenderTargetClearValue{
											   .format = TextureFormat::R11G11B10_Float,
											   .color = {0.f, 0.f, 0.f, 1.f}
										   }
			}, BarrierLayout::RenderTarget);
		m_finalCompositionRTV = m_finalComposition->CreateView({.type = TextureViewType::RTV});
		m_finalCompositionSRV = m_finalComposition->CreateView({.type = TextureViewType::SRV});

		m_camera->SetPerspective(45.f,
			(float)m_viewportSize.x / (float)m_viewportSize.y,
			0.1f, 10000.f);

		return true;
	}

	return true;
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
		.windowSize = {displayInfo.width / 1.25f, displayInfo.height / 1.25f},
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

	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Primary background
		colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);  // #131318
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f); // #131318

		colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);

		// Headers
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);

		// Buttons
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.32f, 0.40f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.38f, 0.50f, 1.00f);

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

		// Borders
		colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Text
		colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

		// Highlights
		colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.80f, 1.00f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 1.00f, 0.50f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.75f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.90f, 1.00f, 1.00f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.00f);

		// Style tweaks
		style.WindowRounding = 5.0f;
		style.FrameRounding = 5.0f;
		style.GrabRounding = 5.0f;
		style.TabRounding = 5.0f;
		style.PopupRounding = 5.0f;
		style.ScrollbarRounding = 5.0f;
		style.WindowPadding = ImVec2(10, 10);
		style.FramePadding = ImVec2(6, 4);
		style.ItemSpacing = ImVec2(8, 6);
		style.PopupBorderSize = 0.f;
	}

	m_sandboxLayer = new SandboxLayer(m_window);
	PushLayer(m_sandboxLayer);
}

Core::Window* SandboxApplication::GetWindow() const
{
	return m_window;
}

Ref<Render::PrimitiveBatch> SandboxApplication::LoadMeshFromFile(const std::wstring& primitiveBatchName,
																 const std::wstring& filepath,
																 std::function<Render::Material(const MeshLoading::PrimitiveData&)> createMaterialFunc,
																 std::wstring primitiveBufferParamName,
																 int64_t primitiveBufferSize,
																 MeshLoading::MeshData* outMeshData)
{
	auto fillBatchData = [&primitiveBatchName, &createMaterialFunc, &primitiveBufferParamName, &primitiveBufferSize](MeshLoading::MeshData& meshData)
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
		*outMeshData = MeshLoading::LoadMeshFromFile(filepath);
		return fillBatchData(*outMeshData);
	}
	else
	{
		auto meshData = MeshLoading::LoadMeshFromFile(filepath);
		return fillBatchData(meshData);
	}
}

Ref<Render::PrimitiveBatch> SandboxApplication::LoadMeshFromFilePBR(const std::wstring& primitiveBatchName, const std::wstring& filepath)
{
	MeshLoading::MeshData meshData;
	auto batch = LoadMeshFromFile(primitiveBatchName, filepath,
								  [](const MeshLoading::PrimitiveData& primitiveData)
								  {
									  Render::Material material;
									  Render::TextureViewDesc srvDesc = {.type = Render::TextureViewType::SRV};
									  if (primitiveData.textures.contains(MeshLoading::TextureType::Albedo))
										  material.SetTexture(L"g_albedoTexture", primitiveData.textures.at(MeshLoading::TextureType::Albedo)->CreateView(srvDesc));
									  if (primitiveData.textures.contains(MeshLoading::TextureType::Normals))
										  material.SetTexture(L"g_normalTexture", primitiveData.textures.at(MeshLoading::TextureType::Normals)->CreateView(srvDesc));
									  if (primitiveData.textures.contains(MeshLoading::TextureType::Metallic))
										  material.SetTexture(L"g_metallicTexture", primitiveData.textures.at(MeshLoading::TextureType::Metallic)->CreateView(srvDesc));
									  if (primitiveData.textures.contains(MeshLoading::TextureType::Roughness))
										  material.SetTexture(L"g_roughnessTexture", primitiveData.textures.at(MeshLoading::TextureType::Roughness)->CreateView(srvDesc));

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
								  }, L"g_modelBuffer", sizeof(ModelUniformBuffer), &meshData);

	return batch;
}
