#include "pcpch.h"
#include "Application.h"

#include "imgui.h"
#include "Prism-Core/Base/Platform.h"
#include "Prism-Core/Render/Layer.h"
#include "Prism-Core/Render/RenderCommandQueue.h"
#include "Prism-Core/Render/RenderDevice.h"

namespace Prism::Core
{
Application::~Application()
{
	if (initializedImGui)
		ShutdownImGui();
	ShutdownPlatform();
	ShutdownRenderer();
}

Application& Application::Get()
{
	return StaticPointerSingleton::Get();
}

void Application::Run()
{
	m_running = true;

	Platform::Get().AddAppEventCallback<AppEvents::Quit>(
		[this](AppEvent event)
		{
			OnQuitEvent(event);
		});

	while (m_running)
	{
		Platform::Get().PumpEvents();

		if (m_running)
		{
			++m_frameCounter;

			BeginFrame();

			for (auto& layer : m_layerStack)
			{
				auto currTime = GetApplicationTime();
				auto delta = currTime - m_previousFrameTime;

				layer->Update(delta);

				m_previousFrameTime = currTime;
			}

			EndFrame();
		}
	}

	// Flush GPU work before exiting
	Render::RenderDevice::Get().GetRenderQueue()->Flush();
}

void Application::PushLayer(Render::Layer* layer)
{
	m_layerStack.emplace_back(layer);
	layer->Attach();
}

void Application::PopLayer(Render::Layer* layer)
{
	auto it = std::ranges::find(m_layerStack, WeakRef(layer));
	if (it != m_layerStack.end())
	{
		(*it)->Detach();
		m_layerStack.erase(it);
	}
}

void Application::RegisterWindow(Window* window)
{
	m_windows.emplace_back(window);
}

void Application::UnregisterWindow(Window* window)
{
	auto it = std::ranges::find(m_windows, WeakRef(window));
	if (it != m_windows.end())
		m_windows.erase(it);
}

void Application::CloseApplication()
{
	m_running = false;
}

Duration Application::GetApplicationTime()
{
	return Platform::Get().GetApplicationTime();
}

void Application::BeginFrame()
{
	Render::RenderDevice::Get().BeginRenderFrame();
	if (initializedImGui)
		ImGuiNewFrame();
}

void Application::EndFrame()
{
	for (auto window : m_windows)
	{
		PE_ASSERT(window.IsValid());

		window->GetSwapchain()->Present();
	}

	if (initializedImGui)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	Render::RenderDevice::Get().EndRenderFrame();
}

void Application::ImGuiNewFrame()
{
	Render::RenderDevice::Get().ImGuiNewFrame();
	Platform::Get().ImGuiNewFrame();
	ImGui::NewFrame();
}

void Application::InitPlatform()
{
	Platform::Create();
}

void Application::ShutdownPlatform()
{
	Platform::TryDestroy();
}

void Application::InitRenderer(const Render::RenderDeviceParams& params)
{
	Render::RenderDevice::Create(params);
}

void Application::ShutdownRenderer()
{
	Render::RenderDevice::TryDestroy();
}

void Application::InitImGui(Window* window)
{
	// TODO: Implement ImGui as a separate layer maybe?
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
	io.FontGlobalScale = 1.5f;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	Platform::Get().InitializeImGuiPlatform(window);
	Render::RenderDevice::Get().InitializeImGui(window);

	initializedImGui = true;
}

void Application::ShutdownImGui()
{
	Platform::Get().ShutdownImGuiPlatform();
	Render::RenderDevice::Get().ShutdownImGui();
	ImGui::DestroyContext();
}

void Application::OnQuitEvent(AppEvent event)
{
	CloseApplication();
}
}
