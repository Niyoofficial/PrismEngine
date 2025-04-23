#pragma once

#include <vector>

#include "Prism/Base/AppEvents.h"
#include "Prism/Base/Window.h"
#include "Prism/Utilities/Duration.h"
#include "Prism/Utilities/StaticSingleton.h"

namespace Prism::Render
{
struct RenderDeviceParams;
class RenderDevice;
class Layer;
}

namespace Prism::Core
{
class Window;

class Application : public StaticPointerSingleton<Application>
{
public:
	Application(int32_t argc, char** argv);
	virtual ~Application();

	template<typename T>
	static T& Get()
	{
		static_assert(std::is_base_of_v<Application, T>);

		return static_cast<T&>(Get());
	}
	static Application& Get();

	void Run();

	void PushLayer(Render::Layer* layer);
	void PopLayer(Render::Layer* layer);

	void RegisterWindow(Window* window);
	void UnregisterWindow(Window* window);

	void CloseApplication();

	Duration GetApplicationTime();
	int64_t GetCurrentFrame() const { return m_frameCounter; }

	const std::vector<WeakRef<Window>>& GetWindows() const { return m_windows; }

protected:
	virtual void BeginFrame();
	virtual void EndFrame();

	virtual void ImGuiNewFrame();

	void InitPlatform();
	void ShutdownPlatform();

	void InitRenderer(const Render::RenderDeviceParams& params);
	void ShutdownRenderer();

	void InitImGui(Window* window, Render::TextureFormat depthFormat);
	void ShutdownImGui();

	void OnQuitEvent(AppEvent event);

	void TransitionBackBuffersToRenderTarget();
	void TransitionBackBuffersToPresent();

protected:
	bool m_running = false;
	int64_t m_frameCounter = 0;

	Duration m_previousFrameTime;

	std::vector<WeakRef<Render::Layer>> m_layerStack;
	Ref<Render::Layer> m_imguiLayer;

	std::vector<WeakRef<Window>> m_windows;

	bool m_imguiInitialized = false;

	bool m_bypassCmdRecording = false;
};
}
