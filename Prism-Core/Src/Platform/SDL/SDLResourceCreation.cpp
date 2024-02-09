#include "pcpch.h"

#include "Platform/SDL/SDLPlatform.h"
#include "Platform/SDL/SDLWindow.h"
#include "Prism-Core/Base/Platform.h"
#include "Prism-Core/Base/Window.h"

namespace Prism::Core
{
Window* Window::Create(const WindowParameters& params, const Render::SwapchainDesc& swapchainDesc)
{
	return new SDL::SDLWindow(params, swapchainDesc);
}

void Platform::Create()
{
	StaticPointerSingleton<Platform>::Create(new SDL::SDLPlatform);
}
}
