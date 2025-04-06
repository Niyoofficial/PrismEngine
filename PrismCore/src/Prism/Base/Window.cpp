#include "pcpch.h"
#include "Window.h"

#include "Prism/Base/Application.h"
#include "Prism/Base/PlatformResourceCreation.h"

namespace Prism::Core
{
Ref<Window> Window::Create(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc)
{
	Ref<Window> window = Private::CreateWindow(windowDesc, swapchainDesc);
	Application::Get().RegisterWindow(window);
	return window;
}

Window::~Window()
{
	Application::Get().UnregisterWindow(this);
}
}
