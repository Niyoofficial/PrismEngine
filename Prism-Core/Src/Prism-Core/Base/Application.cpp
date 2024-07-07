#include "pcpch.h"
#include "Application.h"

#include "Prism-Core/Base/Platform.h"
#include "Prism-Core/Render/Layer.h"
#include "Prism-Core/Render/RenderDevice.h"

namespace Prism::Core
{
Application::~Application()
{
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
			OnQuit(event);
		});

	while (m_running)
	{
		Platform::Get().PumpEvents();

		if (!m_running)
			continue;

		for (auto& layer : m_layerStack)
		{
			auto currTime = GetApplicationTime();
			auto delta = currTime - m_previousFrameTime;

			layer->Update(delta);

			m_previousFrameTime = currTime;
		}

		++m_frameCounter;
	}
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

void Application::OnQuit(AppEvent event)
{
	CloseApplication();
}
}
