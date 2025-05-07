#include "pcpch.h"
#include "Application.h"

#include "imgui.h"
#include "Prism/Base/Platform.h"
#include "Prism/Render/Layer.h"
#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderDevice.h"

namespace Prism::Core
{
Application::Application(int32_t argc, char** argv)
{
	for (int32_t i = 0; i < argc; ++i)
	{
		// TODO: Add some persistent storage for cmd line arguments so they can be queried from the entire engine
		// TODO: Case insensitive comparison
		if (strcmp(argv[i], "-bypassCmdRecord") == 0)
			m_bypassCmdRecording = true;
	}
}

Application::~Application()
{
	if (m_imguiInitialized)
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

			auto currTime = GetApplicationTime();
			auto delta = currTime - m_previousFrameTime;
			m_previousFrameTime = currTime;

			if (m_imguiInitialized)
			{
				ImGuiNewFrame();
				for (auto& layer : m_layerStack)
					layer->UpdateImGui(delta);
				ImGui::EndFrame();
			}

			for (auto& layer : m_layerStack)
				layer->Update(delta);

			if (m_imguiInitialized)
				m_imguiLayer->Update(delta);

			EndFrame();
		}
	}

	// Flush GPU work before exiting
	Render::RenderDevice::Get().GetRenderCommandQueue()->Flush();
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

	TransitionBackBuffersToRenderTarget();
}

void Application::EndFrame()
{
	TransitionBackBuffersToPresent();

	for (auto window : m_windows)
	{
		PE_ASSERT(window.IsValid());

		window->GetSwapchain()->Present();
	}

	if (m_imguiInitialized)
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
	Render::RenderDevice::Get().SetBypassCommandRecording(true);
}

void Application::ShutdownRenderer()
{
	Render::RenderDevice::TryDestroy();
}

void Application::InitImGui(Window* window, Render::TextureFormat depthFormat)
{
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
	Render::RenderDevice::Get().InitializeImGui(window, depthFormat);

	class ImGuiLayer : public Render::Layer
	{
		void Update(Duration delta) override
		{
			auto context = Render::RenderDevice::Get().AllocateContext(L"ImguiUpdate");
			context->RenderImGui();
			Render::RenderDevice::Get().SubmitContext(context);
		}
	};

	m_imguiLayer = new ImGuiLayer;

	m_imguiInitialized = true;
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

void Application::TransitionBackBuffersToRenderTarget()
{
	auto context = Render::RenderDevice::Get().AllocateContext();
	for (auto window : m_windows)
	{
		PE_ASSERT(window.IsValid());
		auto* backBuffer = window->GetSwapchain()->GetCurrentBackBufferRTV();
		context->Barrier({
			.texture = backBuffer->GetTexture(),
			.syncBefore = Render::BarrierSync::None,
			.syncAfter = Render::BarrierSync::None,
			.accessBefore = Render::BarrierAccess::NoAccess,
			.accessAfter = Render::BarrierAccess::NoAccess,
			.layoutBefore = Render::BarrierLayout::Present,
			.layoutAfter = Render::BarrierLayout::RenderTarget
		});
	}
	Render::RenderDevice::Get().SubmitContext(context);
}

void Application::TransitionBackBuffersToPresent()
{
	auto context = Render::RenderDevice::Get().AllocateContext();
	for (auto window : m_windows)
	{
		PE_ASSERT(window.IsValid());
		auto* backBuffer = window->GetSwapchain()->GetCurrentBackBufferRTV();
		context->Barrier({
			.texture = backBuffer->GetTexture(),
			.syncBefore = Render::BarrierSync::None,
			.syncAfter = Render::BarrierSync::None,
			.accessBefore = Render::BarrierAccess::NoAccess,
			.accessAfter = Render::BarrierAccess::NoAccess,
			.layoutBefore = Render::BarrierLayout::RenderTarget,
			.layoutAfter = Render::BarrierLayout::Present
		});
	}
	Render::RenderDevice::Get().SubmitContext(context);
}
}
