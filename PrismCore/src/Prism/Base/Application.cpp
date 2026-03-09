#include "pcpch.h"
#include "Application.h"

#include "Base.h"
#include "Prism/Base/Paths.h"
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

	m_assetManager.GetHandleFromPath(Core::Paths::Get().GetProjectAssetsDir() / "A/B/C.txt");
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
	ImGuizmo::BeginFrame();
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
	using namespace Render;

	RenderDevice::Create(params);

	uint8_t data[32] = {0};
	m_builtinResources.whiteTexture = Texture::Create({
		.textureName = L"BuiltinWhiteTexture",
		.width = 2,
		.height = 2,
		.format = TextureFormat::RGBA16_UNorm,
		.bindFlags = BindFlags::ShaderResource,
		}, BarrierLayout::Common, {.data = data, .sizeInBytes = sizeof(data)});
	memset(data, 1, sizeof(data));
	m_builtinResources.blackTexture = Texture::Create({
		.textureName = L"BuiltinBlackTexture",
		.width = 2,
		.height = 2,
		.format = TextureFormat::RGBA16_UNorm,
		.bindFlags = BindFlags::ShaderResource,
		}, BarrierLayout::Common, {.data = data, .sizeInBytes = sizeof(data)});
}

void Application::ShutdownRenderer()
{
	Render::RenderDevice::TryDestroy();
}

void Application::InitImGui(Window* window, Render::TextureFormat depthFormat)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	Platform::Get().InitializeImGuiPlatform(window);
	Render::RenderDevice::Get().InitializeImGui(window, depthFormat);

	class ImGuiLayer : public Render::Layer
	{
		void Update(Duration delta) override
		{
			auto context = Render::RenderDevice::Get().AllocateContext(L"ImGuiUpdate");
			context->RenderImGui();
			Render::RenderDevice::Get().SubmitContext(context);
		}
	};

	m_imguiLayer = Ref<ImGuiLayer>::Create();

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
	auto context = Render::RenderDevice::Get().AllocateContext(L"TransitionBackBuffersToRenderTarget");
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
	auto context = Render::RenderDevice::Get().AllocateContext(L"TransitionBackBuffersToPresent");
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
