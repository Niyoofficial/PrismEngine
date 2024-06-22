#include "pcpch.h"

#include "Platform/SDL/SDLPlatform.h"
#include "Platform/SDL/SDLWindow.h"
#include "Prism-Core/Base/Application.h"

namespace Prism::Core::Private
{
Window* CreateWindow(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc)
{
	return new SDL::SDLWindow(windowDesc, swapchainDesc);
}

Platform* CreatePlatform()
{
	return new SDL::SDLPlatform;
}
}
