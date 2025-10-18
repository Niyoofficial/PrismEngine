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

IMPLEMENT_APPLICATION(SandboxApplication);

SandboxLayer::SandboxLayer(Core::Window* owningWindow)
	: m_owningWindow(owningWindow)
{
	using namespace Prism::Render;
	Layer::Attach();

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::KeyDown>(
		[this](Core::AppEvent event)
		{
			if (std::get<Core::AppEvents::KeyDown>(event).keyCode == KeyCode::Escape)
				m_scene->SetSelectedEntity(nullptr);
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
			else if (std::get<Core::AppEvents::MouseButtonDown>(event).keyCode == KeyCode::LeftMouseButton)
			{
				m_lastTimeButtonDown = Core::Platform::Get().GetApplicationTime();
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
			else if (std::get<Core::AppEvents::MouseButtonUp>(event).keyCode == KeyCode::LeftMouseButton && m_viewportHovered)
			{
				// Consider this a click
				if ((Core::Platform::Get().GetApplicationTime() - m_lastTimeButtonDown).GetSeconds() < 0.2)
					SelectEntityUnderCursor();
			}
		});

	// TODO: Add create function on Ref class and make RefCounted not allow object creation on stack
	Ref sponza = new MeshLoading::MeshAsset(L"assets/SponzaCrytek/Sponza.gltf");

	m_scene = Scene::Create(L"Test Scene");
	m_renderPipeline = new PBRSceneRenderPipeline;
	m_scene->SetRenderPipeline(m_renderPipeline);
	m_scene->CreateEntityHierarchyForMeshAsset(sponza);
	auto lightEntity = m_scene->AddEntity(L"Light");
	lightEntity->AddComponent<LightRendererComponent>();
	lightEntity->AddComponent<TransformComponent>()->SetRotation(m_sunRotation);

	glm::int2 windowSize = SandboxApplication::Get().GetWindow()->GetSize();

	m_camera = new Camera(45.f, (float)windowSize.x / (float)windowSize.y, 0.1f, 10000.f);
	m_camera->SetPosition({0.f, 0.f, 0.f});

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
	/*m_sphere = SandboxApplication::LoadMeshFromFile(L"Sphere", L"meshes/Sphere.gltf",
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
		}, L"g_modelBuffer", sizeof(ModelUniformBuffer));*/
}

void SandboxLayer::UpdateImGui(Duration delta)
{
	using namespace Prism::Render;
	Layer::UpdateImGui(delta);

	static bool s_showImGuiDemoWindow = false;
	static bool s_showStatWindow = true;
	static bool s_showDebugMenu = true;
	static bool s_showLogMenu = true;
	static bool s_showSceneOutliner = true;
	static bool s_showInspector = true;

	// Menu bar
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Show"))
		{
			ImGui::MenuItem("Show scene outliner", nullptr, &s_showSceneOutliner);
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
		m_viewportPosition = {ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y};

		auto viewportSize = ImGui::GetContentRegionAvail();
		if (CheckForViewportResize({viewportSize.x, viewportSize.y}))
		{
			ImGui::Image(m_editorViewportSRV, viewportSize);

			// Gizmo
			if (Entity* selectedEntity = m_scene->GetSelectedEntity())
			{
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect((float)m_viewportPosition.x, (float)m_viewportPosition.y, (float)m_viewportSize.x, (float)m_viewportSize.y);

				auto viewMatrix = m_camera->GetViewMatrix();
				auto projMatrix = m_camera->GetProjectionMatrix();
				if (auto comp = selectedEntity->GetComponent<TransformComponent>())
				{
					auto transform = comp->GetTransform();
					if (ImGuizmo::Manipulate(glm::value_ptr(viewMatrix),
											 glm::value_ptr(projMatrix),
											 m_gizmoOperation, m_gizmoMode,
											 glm::value_ptr(transform)))
					{
						glm::float3 translation, rotation, scale;
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

						comp->SetTranslation(translation);
						comp->SetRotation(glm::radians(rotation));
						comp->SetScale(scale);
					}
				}
			}

			// Gizmo toolbar
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
					ImGuiChildFlags_AutoResizeX |
					ImGuiChildFlags_AutoResizeY;

				constexpr float padding = 10.0f;
				ImGui::SetCursorPos({ImGui::GetCursorStartPos().x + padding, ImGui::GetCursorStartPos().y + padding});
				ImGui::SetNextWindowBgAlpha(0.55f); // Transparent background

				if (ImGui::BeginChild("##gizmo_toolbar", {0.f, 0.f}, childFlags, windowFlags))
				{
					auto drawGizmoTool =
						[this](ImGuizmo::OPERATION operation, const char* label)
						{
							constexpr glm::float2 buttonSize = {30.f, 30.f};
							glm::float2 startPos = ImGui::GetCursorScreenPos();
							ImGui::PushID(label);
							if (ImGui::Button("", buttonSize))
								m_gizmoOperation = operation;
							ImGui::PopID();

							ImGui::SameLine();
							glm::float2 nextCursorPos = ImGui::GetCursorScreenPos();

							if (m_gizmoOperation == operation)
							{
								constexpr glm::float4 selectColor = {1.f, 0.45f, 0.f, 0.25f};
								ImGui::GetWindowDrawList()->AddRectFilled(startPos, startPos + buttonSize, ImGui::GetColorU32(selectColor), ImGui::GetStyle().FrameRounding);
							}

							ImGui::SetCursorScreenPos({startPos.x + (buttonSize.x / 2.f) - (ImGui::CalcTextSize(label).x / 2.f), startPos.y});
							ImGui::Text("%s", label);

							return nextCursorPos;
						};

					glm::float2 nextCursorPos;
					nextCursorPos = drawGizmoTool(ImGuizmo::UNIVERSAL, "U");
					ImGui::SetCursorScreenPos(nextCursorPos);
					nextCursorPos = drawGizmoTool(ImGuizmo::TRANSLATE, "T");
					ImGui::SetCursorScreenPos(nextCursorPos);
					nextCursorPos = drawGizmoTool(ImGuizmo::ROTATE, "R");
					ImGui::SetCursorScreenPos(nextCursorPos);
					nextCursorPos = drawGizmoTool(ImGuizmo::SCALE, "S");
				}
				ImGui::EndChild();
			}

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
				constexpr float width = 250.0f;

				ImGui::SetCursorPos({ImGui::GetContentRegionAvail().x - width - padding, ImGui::GetCursorStartPos().y + padding});
				ImGui::SetNextWindowBgAlpha(0.55f); // Transparent background

				if (ImGui::BeginChild("##stats_overlay", {width, 0.f}, childFlags, windowFlags))
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

	// TODO: Make the pipeline draw this
	// Debug menu
	/*if (s_showDebugMenu)
	{
		ImGui::Begin("Debug", &s_showDebugMenu, ImGuiWindowFlags_HorizontalScrollbar);

		if (ImGui::CollapsingHeader("GBuffer"))
		{
			auto gbufferSize = m_renderPipeline->GetGBuffer().GetTexture(GBuffer::Type::Color)->GetTextureDesc().GetSize();
			float texWidth = (float)gbufferSize.x / (float)gbufferSize.y * 256.f;
			ImGui::Text("Color");
			ImGui::Image(m_renderPipeline->GetGBuffer().GetView(GBuffer::Type::Color, TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("Depth");
			ImGui::Image(m_renderPipeline->GetGBuffer().GetView(GBuffer::Type::Depth, TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("Normal");
			ImGui::Image(m_renderPipeline->GetGBuffer().GetView(GBuffer::Type::Normal, TextureViewType::SRV), {texWidth, 256});
			ImGui::Text("R:Roughness G:Metal B:AO");
			static bool s_roughness = true;
			ImGui::Checkbox("Roughness", &s_roughness);
			ImGui::SameLine();
			static bool s_metallic = true;
			ImGui::Checkbox("Metallic", &s_metallic);
			ImGui::SameLine();
			static bool s_ao = true;
			ImGui::Checkbox("AO", &s_ao);
			ImGui::Image(m_renderPipeline->GetGBuffer().GetView(GBuffer::Type::Roughness_Metal_AO, TextureViewType::SRV), {texWidth, 256}, {0, 0}, {1, 1}, {(float)s_roughness, (float)s_metallic, (float)s_ao, 1});

		}

		/*if (ImGui::CollapsingHeader("Sun"))
		{
			//ImGui::SliderFloat("Environment light scale", &m_environmentLightScale, 0.f, 1.f);
			ImGui::SliderAngle("Sun Rotation Yaw", &m_sunRotation.y, -180.f, 180.f);
			ImGui::SliderAngle("Sun Rotation Pitch", &m_sunRotation.z, -180.f, 180.f);
			ImGui::Image(m_sunShadowMap->CreateView({ .type = TextureViewType::SRV, .format = TextureFormat::R32_Float }), {256, 256});
		}#1#

		/*if (ImGui::CollapsingHeader("IBL"))
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
		}#1#

		/*if (ImGui::CollapsingHeader("Bloom"))
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
		}#1#

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

	// Scene Outliner
	if (s_showSceneOutliner)
	{
		if (ImGui::Begin("SceneOutliner", &s_showSceneOutliner, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (ImGui::BeginChild("SceneOutliner_Hierarchy", {0, 0}))
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

						Entity* currEntity = m_scene->GetEntityByIndex(i);

						if (currEntity == m_scene->GetSelectedEntity())
							treeFlags |= ImGuiTreeNodeFlags_Selected;

						std::wstring entityName = currEntity->GetName();
						if (ImGui::TreeNodeEx(std::to_string(i).c_str(), treeFlags, "%s", entityName.empty() ? "<unnamed>" : WStringToString(entityName).c_str()))
						{
							if (ImGui::IsItemFocused())
								m_scene->SetSelectedEntity(currEntity);

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

	/*if (s_showInspector && m_scene->GetSelectedEntity())
	{
		if (ImGui::Begin("SceneInspector", &s_showInspector, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			
		}
		ImGui::End();
	}*/
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

	Ref renderContext = RenderDevice::Get().AllocateContext();

	m_scene->RenderScene(renderContext, m_editorViewportRTV, m_camera);

	RenderDevice::Get().SubmitContext(renderContext);
}

bool SandboxLayer::CheckForViewportResize(glm::int2 viewportSize)
{
	using namespace Render;

	if (viewportSize != m_viewportSize)
	{
		m_viewportSize = {viewportSize.x, viewportSize.y};

		PE_RENDER_LOG(Trace, "Viewport resized to x: {} y: {}", viewportSize.x, viewportSize.y);

		if (m_viewportSize.x == 0 || m_viewportSize.y == 0)
			return false;

		m_editorViewport = Texture::Create({
											   .textureName = L"EditorViewport",
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
		m_editorViewportRTV = m_editorViewport->CreateDefaultRTV();
		m_editorViewportSRV = m_editorViewport->CreateDefaultSRV();

		m_hitProxiesTexture = Texture::Create({
												  .textureName = L"HitProxiesTexture",
												  .width = m_viewportSize.x,
												  .height = m_viewportSize.y,
												  .dimension = ResourceDimension::Tex2D,
												  .format = TextureFormat::R32_UInt,
												  .bindFlags = Flags(BindFlags::RenderTarget) | Flags(BindFlags::ShaderResource),
												  .optimizedClearValue = RenderTargetClearValue{
													  .format = TextureFormat::R32_UInt,
													  .color = {0.f, 0.f, 0.f, 0.f}
												  }
											  }, BarrierLayout::RenderTarget);

		m_camera->SetPerspective(45.f,
								 (float)m_viewportSize.x / (float)m_viewportSize.y,
								 0.1f, 10000.f);

		return true;
	}

	return true;
}

void SandboxLayer::SelectEntityUnderCursor()
{
	using namespace Prism::Render;

	auto renderContext = RenderDevice::Get().AllocateContext(L"HitProxies");

	auto entities = m_scene->RenderHitProxies(renderContext, m_hitProxiesTexture->CreateDefaultRTV(), m_camera);
	renderContext->SetPSO(ComputePipelineStateDesc{
		.cs = {
			.filepath = L"shaders/HitProxy.hlsl",
			.entryName = L"CsReadPixel",
			.shaderType = ShaderType::CS,
		}
		});

	struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) HitProxyReadSettings
	{
		glm::uint2 relMousePos;
	};
	Ref<Buffer> hitProxyReadSettings = Buffer::Create({
		.bufferName = L"HitProxyOutputBuffer",
		.size = sizeof(HitProxyReadSettings),
		.bindFlags = BindFlags::UniformBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});
	Ref<Buffer> hitProxyOutput = Buffer::Create({
		.bufferName = L"HitProxyOutputBuffer",
		.size = sizeof(uint32_t),
		.bindFlags = BindFlags::UnorderedAccess,
		.usage = ResourceUsage::Staging,
		.cpuAccess = CPUAccess::Read
	});

	glm::int2 mousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
	auto relMousePos = mousePos - m_viewportPosition;
	PE_ASSERT(relMousePos.x > 0 && relMousePos.y > 0);

	void* data = hitProxyReadSettings->Map(CPUAccess::Write);
	HitProxyReadSettings readSettings = {
		.relMousePos = relMousePos
	};
	memcpy(data, &readSettings, sizeof(readSettings));
	hitProxyReadSettings->Unmap();

	renderContext->SetBuffer(hitProxyReadSettings->CreateDefaultUniformBufferView(), L"g_hitProxyReadSettingsBuffer");
	renderContext->SetTexture(m_hitProxiesTexture->CreateDefaultSRV(), L"g_hitProxiesTexture");
	renderContext->SetBuffer(hitProxyOutput->CreateDefaultUAV(sizeof(uint32_t)), L"g_hitProxyOutputBuffer");
	renderContext->Dispatch({1, 1, 1});
	renderContext->AddGPUCompletionCallback(
		[hitProxyOutput, entities, this]()
		{
			void* data = hitProxyOutput->Map(CPUAccess::Read);

			uint32_t ID = *(uint32_t*)data;
			m_scene->SetSelectedEntity(entities.at(ID));

			hitProxyOutput->Unmap();
		});

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
		.windowTitle = L"Prism Editor",
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

	// TODO: Remove depth format
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

		// Table
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.f, 1.f, 1.f, 0.024f);

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

	// ImGuizmo
	{
		auto& colors = ImGuizmo::GetStyle().Colors;
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
