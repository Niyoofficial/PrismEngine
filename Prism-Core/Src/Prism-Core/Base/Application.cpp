#include "pcpch.h"
#include "Application.h"

#include "Prism-Core/Base/Platform.h"
#include "Prism-Core/Render/Layer.h"
#include "Prism-Core/Render/Renderer.h"
#include "Prism-Core/Base/PlatformResourceCreation.h"

namespace Prism::Core
{
Application& Application::Get()
{
	return StaticPointerSingleton::Get();
}

void Application::Run()
{
	m_running = true;

	Init();

	while (m_running)
	{
		Platform::Get().PumpEvents();

		for (Render::Layer* layer : m_layerStack)
		{
			auto currTime = Platform::Get().GetApplicationTime();
			auto delta = currTime - m_previousFrameTime;

			layer->Update(delta);

			m_previousFrameTime = currTime;
		}

		++m_frameCounter;
	}

	Shutdown();
}

void Application::PushLayer(Render::Layer* layer)
{
	m_layerStack.push_back(layer);
	layer->Attach();
}

void Application::PopLayer(Render::Layer* layer)
{
	auto it = std::ranges::find(m_layerStack, layer);
	if (it != m_layerStack.end())
	{
		(*it)->Detach();
		m_layerStack.erase(it);
	}
}

Window* Application::CreateWindow(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc)
{
	Window* window = Private::CreateWindow(windowDesc, swapchainDesc);
	m_windows.push_back(window);
	return window;
}

void Application::UnregisterWindow(Window* window)
{
	auto it = std::ranges::find(m_windows, window);
	if (it != m_windows.end())
		m_windows.erase(it);
}

void Application::Init()
{
}

void Application::Shutdown()
{
	ShutdownPlatform();
	ShutdownRenderer();
}

void Application::InitPlatform()
{
	Platform::Create();
}

void Application::ShutdownPlatform()
{
	Platform::TryDestroy();
}

void Application::InitRenderer()
{
	Render::Renderer::Create();

	/*int64_t test = 2;
	Render::Renderer::Get().Submit(
		[test]()
		{
			PE_CORE_LOG(Warn, "Test Yeah! {}", test);
		});*/
}

void Application::ShutdownRenderer()
{
	Render::Renderer::TryDestroy();
}
}
