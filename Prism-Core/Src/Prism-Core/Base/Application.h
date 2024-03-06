#pragma once

#include <vector>

#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Utilities/Duration.h"
#include "Prism-Core/Utilities/StaticSingleton.h"

namespace Prism::Render
{
class Layer;
}

namespace Prism::Core
{
class Window;

class Application : public StaticPointerSingleton<Application>
{
public:
	Application() = default;
	virtual ~Application() = default;

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

	Window* CreateWindow(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc);

	void UnregisterWindow(Window* window);

protected:
	virtual void Init();
	virtual void Shutdown();

	void InitPlatform();
	void ShutdownPlatform();

	void InitRenderer();
	void ShutdownRenderer();

protected:
	bool m_running = false;
	int64_t m_frameCounter = 0;

	Duration m_previousFrameTime;

	std::vector<Render::Layer*> m_layerStack;

	std::vector<Window*> m_windows;
};
}
