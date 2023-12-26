#include "SDLWindow.h"

#include "SDL3/SDL.h"

namespace Prism::Platform::SDL
{
void SDLWindow::Init(const Core::WindowParameters& params)
{
	Window::Init(params);

	/*SDL_CreateWindow(params.windowTitle.c_str(),
		(int32_t)params.windowSize.x, (int32_t)params.windowSize.y,
		SDL_WINDOW_RESIZABLE);*/
}
}
