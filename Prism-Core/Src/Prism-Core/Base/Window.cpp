#include "pcpch.h"
#include "Window.h"

#include "Prism-Core/Base/Application.h"
#include "Prism-Core/Base/PlatformResourceCreation.h"

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
