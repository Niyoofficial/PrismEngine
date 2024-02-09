#include "pcpch.h"
#include "Application.h"

#include "Prism-Core/Base/Platform.h"
#include "Prism-Core/Render/Renderer.h"

namespace Prism::Core
{
void Application::Run()
{
	m_running = true;

	Init();

	while (m_running)
	{
		// Wait for idle render thread

		Platform::Get().PumpEvents();


	}

	Shutdown();
}

void Application::PushLayer(Render::Layer* layer)
{
	m_layerStack.push_back(layer);
}

void Application::PopLayer(Render::Layer* layer)
{
	auto it = std::ranges::find(m_layerStack, layer);
	if (it != m_layerStack.end())
		m_layerStack.erase(it);
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

	int64_t test = 2;
	Render::Renderer::Get().Submit(
		[test]()
		{
			PE_CORE_LOG(Warn, "Test Yeah! {}", test);
		});
}

void Application::ShutdownRenderer()
{
	Render::Renderer::TryDestroy();
}
}
