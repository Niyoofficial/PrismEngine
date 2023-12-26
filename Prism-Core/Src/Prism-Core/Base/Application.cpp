#include "pcpch.h"
#include "Application.h"
#include "Platform-Test/Test.h"

namespace Prism::Core
{
void Application::Run()
{
	Init();
}

void Application::Init()
{
	m_window.reset(Window::Create());
	m_window->Init({
	.windowTitle = "Test",
	.windowSize = {1600.f, Platform::Test::TestFunction(2.f)},
	.fullscreen = false});
}
}
