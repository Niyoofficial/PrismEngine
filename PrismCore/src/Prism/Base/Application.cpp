#include "pcpch.h"
#include "Application.h"

#include "Base.h"
#include "Prism/Base/Platform.h"
#include "Prism/Render/Layer.h"
#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderConstants.h"
#include "Prism/Render/RenderDevice.h"
#include "Prism/UI/imgui_threaded_rendering.h"

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
		else if (strcmp(argv[i], "-enableRenderDebugLayer") == 0)
			m_enableRenderDebugLayer = true;
	}
}

Application::~Application()
{
	if (m_imguiInitialized)
		ShutdownImGui();
	m_builtinResources.~BuiltinResources();
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

			{
				SCOPED_INSTRUMENTATION("Frame");

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
	}

	// Flush GPU work before exiting
	Render::RenderDevice::Get().GetRenderCommandQueue()->Flush(Render::CommandQueueFlushType::WaitForCompletion);
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

void Application::RegisterWindow(Ref<Window> window)
{
	m_windows.emplace_back(window);
}

void Application::UnregisterWindow(Ref<Window> window)
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

void Application::InitRenderer(Render::RenderDeviceParams params)
{
	using namespace Render;

	params.enableDebugLayer |= m_enableRenderDebugLayer;
	RenderDevice::Create(params);
	RenderDevice::Get().SetBypassCommandRecording(m_bypassCmdRecording);

	TextureDesc texDesc = {
		.width = 2,
		.height = 2,
		.format = TextureFormat::RGBA16_UNorm,
		.bindFlags = BindFlags::ShaderResource,
	};

	std::vector<uint8_t> data;
	data.resize(RenderDevice::Get().GetTotalSizeInBytes(texDesc), 0);

	texDesc.textureName = L"BuiltinBlackTexture";
	m_builtinResources.blackTexture = Texture::Create(texDesc, BarrierLayout::Common, {.data = data.data(), .sizeInBytes = (int64_t)data.size()});

	memset(data.data(), 255, data.size());

	texDesc.textureName = L"BuiltinWhiteTexture";
	m_builtinResources.whiteTexture = Texture::Create(texDesc, BarrierLayout::Common, {.data = data.data(), .sizeInBytes = (int64_t)data.size()});

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
			ImGui::Render();

			auto& snapshot = m_snapshots[Application::Get().GetCurrentFrame() % Render::Constants::MAX_FRAMES_IN_FLIGHT];
			snapshot.SnapUsingSwap(ImGui::GetDrawData(), ImGui::GetTime());

			auto context = Render::RenderDevice::Get().AllocateContext(L"ImGuiUpdate");
			context->RenderImGui(&snapshot.DrawData);
			Render::RenderDevice::Get().SubmitContext(context);
		}

		ImDrawDataSnapshot m_snapshots[Render::Constants::MAX_FRAMES_IN_FLIGHT];
	};

	m_imguiLayer = Ref<ImGuiLayer>::Create();

	m_imguiInitialized = true;
}

void Application::ShutdownImGui()
{
	Render::RenderDevice::Get().ShutdownImGui();
	Platform::Get().ShutdownImGuiPlatform();
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

	for (auto window : m_windows)
	{
		PE_ASSERT(window.IsValid());

		Render::RenderDevice::Get().GetRenderCommandQueue()->EnqueuePresent(window->GetSwapchain());
	}
}
}
