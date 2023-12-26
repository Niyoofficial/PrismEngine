#include "pcpch.h"
#include "Prism-Core/Base/Window.h"
#include "Platform-SDL/SDLWindow.h"

namespace Prism::Core
{
Window* Window::Create()
{
	return nullptr; // new Platform::SDL::SDLWindow;
}
}
