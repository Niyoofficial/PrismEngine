#include "pcpch.h"

#include "Platform/SDL/SDLPlatform.h"
#include "Platform/SDL/SDLWindow.h"
#include "Prism/Base/Application.h"

namespace Prism::Core::Private
{
Ref<Window> CreateWindow(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc)
{
	return Ref<SDL::SDLWindow>::Create(windowDesc, swapchainDesc);
}

Platform* CreatePlatform()
{
	return new SDL::SDLPlatform;
}
}
