#pragma once

#include <vector>

#include "Prism-Core/Base/AppEvents.h"
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Utilities/Duration.h"
#include "Prism-Core/Utilities/StaticSingleton.h"

namespace Prism::Render
{
class RenderDevice;
class Layer;
}

namespace Prism::Core
{
class Window;

class Application : public StaticPointerSingleton<Application>
{
public:
	Application() = default;
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

protected:
	void InitPlatform();
	void ShutdownPlatform();

	void InitRenderer();
	void ShutdownRenderer();

	void OnQuit(AppEvent event);

protected:
	bool m_running = false;
	int64_t m_frameCounter = 0;

	Duration m_previousFrameTime;

	std::vector<WeakRef<Render::Layer>> m_layerStack;

	std::vector<WeakRef<Window>> m_windows;
};
}
