#include "pcpch.h"
#include "SDLWindow.h"

#include "Prism-Core/Render/Swapchain.h"
#include "SDL3/SDL.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Prism::SDL
{
SDLWindow::SDLWindow(const Core::WindowParameters& windowParams, const Render::SwapchainDesc& swapchainDesc)
	: Window(windowParams, swapchainDesc), m_windowed(!windowParams.fullscreen)
{
	m_nativeWindow = SDL_CreateWindow(windowParams.windowTitle.c_str(),
									  windowParams.windowSize.x, windowParams.windowSize.y,
									  SDL_WINDOW_RESIZABLE);

	m_swapchain.reset(Render::Swapchain::Create(this, swapchainDesc));
}

std::any SDLWindow::GetNativeWindow() const
{
	return m_nativeWindow;
}

std::any SDLWindow::GetPlatformNativeWindow() const
{
	return (HWND)SDL_GetProperty(SDL_GetWindowProperties(m_nativeWindow), "SDL.window.win32.hwnd", nullptr);
}

Core::WindowParameters SDLWindow::GetWindowParams() const
{
	return {
		.windowTitle = GetTitle(),
		.windowSize = GetSize(),
		.fullscreen = GetIsFullscreen()
	};
}

glm::int2 SDLWindow::GetSize() const
{
	glm::int2 size;
	SDL_GetWindowSizeInPixels(m_nativeWindow, &size.x, &size.y);
	return size;
}

bool SDLWindow::GetIsFullscreen() const
{
	uint32_t flags = SDL_GetWindowFlags(m_nativeWindow);
	return (flags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN;
}

std::string SDLWindow::GetTitle() const
{
	return SDL_GetWindowTitle(m_nativeWindow);
}
}
