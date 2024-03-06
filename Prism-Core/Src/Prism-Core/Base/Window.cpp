#include "pcpch.h"
#include "Window.h"

#include "Prism-Core/Base/Application.h"

namespace Prism::Core
{
Window* Window::Create(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc)
{
	return Application::Get().CreateWindow(windowDesc, swapchainDesc);
}

Window::~Window()
{
	Application::Get().UnregisterWindow(this);
}
}
