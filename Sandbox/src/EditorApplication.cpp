#include "EditorApplication.h"

#include "EditorTheme.h"
#include "Prism/Base/Platform.h"
#include "Prism/Render/PBR/PBRSceneRenderPipeline.h"
#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/LightRendererComponent.h"
#include "Prism/Render/RenderDevice.h"

IMPLEMENT_APPLICATION(EditorApplication);

namespace Prism
{
	EditorApplication& EditorApplication::Get()
	{
		return Application::Get<EditorApplication>();
	}

	EditorApplication::EditorApplication(int32_t argc, char** argv)
		: Application(argc, argv)
	{
		InitPlatform();
		InitRenderer({ .enableDebugLayer = false, .initPixLibrary = false });

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

		ImGuizmo::AllowAxisFlip(false);

		m_scene = Ref<Scene>::Create(L"Test Scene");
		m_scene->SetRenderPipeline<Render::PBRSceneRenderPipeline>();
		//Ref sponza = new MeshAsset(L"assets/SponzaCrytek/Sponza.gltf");

		//m_scene->CreateEntityHierarchyForMeshAsset(sponza);
		auto lightEntity = m_scene->AddEntity(L"Light");
		lightEntity->AddComponent<TransformComponent>()->SetRotation({ 0.f, glm::radians(-7.f), glm::radians(-101.f) });
		lightEntity->AddComponent<LightRendererComponent>();

		m_sandboxLayer = Ref<EditorLayer>::Create(m_window, m_scene);
		PushLayer(m_sandboxLayer);
	}

	void EditorApplication::InitImGui(Core::Window* window, Render::TextureFormat depthFormat)
	{
		Application::InitImGui(window, depthFormat);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// TODO: Fix this!
		io.FontGlobalScale = 1.5f;

		EditorTheme::Init();
	}
}
