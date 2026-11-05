#include "pcpch.h"
#include "Application.h"

#include "Base.h"
#include "Prism/Base/Platform.h"
#include "Prism/Render/Layer.h"
#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderConstants.h"
#include "Prism/Render/RenderDevice.h"
#include "Prism/UI/imgui_threaded_rendering.h"
#include "Prism/Base/Paths.h"

#include "nethost.h"
#include "hostfxr.h"
#include "coreclr_delegates.h"

namespace Prism::Core
{
struct ExportsTable
{
	void*(CORECLR_DELEGATE_CALLTYPE* CreateWindow)(const char*, int32_t, int32_t, int32_t, void*);
};

std::vector<Ref<Window>> managedWindows;

static void* NativeCreateWindow(const char* title, int32_t titleLen, int32_t width, int32_t height, void* parent)
{
	WindowDesc windowDesc = {
		.windowTitle = std::string(title, titleLen),
		.windowSize = {width, height},
		.fullscreen = false
	};
	Render::SwapchainDesc swapchainDesc = {
		.format = Render::TextureFormat::RGBA8_UNorm,
	};
	auto window = Window::Create(windowDesc, swapchainDesc);
	managedWindows.push_back(window);

	return window->GetPlatformNativeWindow();
}

Application::Application(int32_t argc, char** argv)
{
	bool enableRenderDebugLayer = false;
	bool initPixLibrary = false;
	for (int32_t i = 0; i < argc; ++i)
	{
		// TODO: Add some persistent storage for cmd line arguments so they can be queried from the entire engine
		// TODO: Case insensitive comparison
		if (strcmp(argv[i], "-bypassCmdRecord") == 0)
			m_bypassCmdRecording = true;
		else if (strcmp(argv[i], "-enableRenderDebugLayer") == 0)
			m_enableRenderDebugLayer = true;
	}

	InitPlatform();
	InitRenderer({.enableDebugLayer = enableRenderDebugLayer, .initPixLibrary = initPixLibrary});
	InitDotNet();
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

void Application::InitDotNet()
{
	std::array<char_t, 260> hostfxrPath;
	size_t pathSize = hostfxrPath.size();
	int32_t rc = get_hostfxr_path(hostfxrPath.data(), &pathSize, nullptr);
	PE_ASSERT(rc == 0);

	void* lib = Platform::Get().LoadLibrary(hostfxrPath.data());
	auto hostfxrInitFunc = (hostfxr_initialize_for_runtime_config_fn)Platform::Get().LoadFunction(lib, "hostfxr_initialize_for_runtime_config");
	auto getDelegateFunc = (hostfxr_get_runtime_delegate_fn)Platform::Get().LoadFunction(lib, "hostfxr_get_runtime_delegate");
	auto hostfxrCloseFunc = (hostfxr_close_fn)Platform::Get().LoadFunction(lib, "hostfxr_close");
	PE_ASSERT(hostfxrInitFunc && getDelegateFunc && hostfxrCloseFunc);

	hostfxr_handle context;
	rc = hostfxrInitFunc((Paths::Get().GetPrismSharpBinaryDir() / "PrismSharp.runtimeconfig.json").wstring().c_str(), nullptr, &context);
	PE_ASSERT(rc == 0);

	load_assembly_and_get_function_pointer_fn loadAssemblyAndGetFunctionPointer = nullptr;
	rc = getDelegateFunc(context, hdt_load_assembly_and_get_function_pointer, (void**)&loadAssemblyAndGetFunctionPointer);
	PE_ASSERT(rc == 0);

	using BootstrapFn = int32_t(CORECLR_DELEGATE_CALLTYPE*)(ExportsTable*);
	BootstrapFn bootstrap = nullptr;
	rc = loadAssemblyAndGetFunctionPointer((Paths::Get().GetPrismSharpBinaryDir() / "PrismSharp.dll").wstring().c_str(), L"PrismSharp.Host, PrismSharp", L"Bootstrap", UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&bootstrap);
	PE_ASSERT(rc == 0);

	ExportsTable bindings = {
		.CreateWindow = &NativeCreateWindow
	};
	bootstrap(&bindings);
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

		window->GetSwapchain()->PreparePresent();
	}

	Render::RenderDevice::Get().SubmitContext(context);

	for (auto window : m_windows)
	{
		PE_ASSERT(window.IsValid());

		Render::RenderDevice::Get().GetRenderCommandQueue()->EnqueuePresent(window->GetSwapchain());
	}
}
}
