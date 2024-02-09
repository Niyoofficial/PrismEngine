#pragma once
#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Render/Swapchain.h"

namespace Prism::Render
{
class Swapchain;
}

namespace Prism::Core
{
struct WindowParameters
{
public:
	std::string windowTitle;
	glm::int2 windowSize;
	bool fullscreen = false;
};

class Window
{
public:
	static Window* Create(const WindowParameters& params, const Render::SwapchainDesc& swapchainDesc);

	explicit Window(const WindowParameters& windowParams, const Render::SwapchainDesc& swapchainDesc) {}
	virtual ~Window() = default;

	// Returns native window for the platform abstraction
	// if we are using any (like SDL) or native platform window if we don't
	virtual std::any GetNativeWindow() const = 0;
	// Returns native window for the current platform (HWND on Windows)
	virtual std::any GetPlatformNativeWindow() const = 0;

	virtual WindowParameters GetWindowParams() const = 0;
	virtual glm::int2 GetSize() const = 0;
	virtual bool GetIsFullscreen() const = 0;
	virtual std::string GetTitle() const = 0;

	virtual Render::Swapchain* GetSwapchain() = 0;
};
}
