#pragma once
#include "Prism/Base/Base.h"
#include "Prism/Render/Swapchain.h"

#include <any>

namespace Prism::Render
{
class Swapchain;
}

namespace Prism::Core
{
struct WindowDesc
{
public:
	std::wstring windowTitle;
	glm::int2 windowSize;
	bool fullscreen = false;
};

class Window : public RefCounted
{
public:
	virtual ~Window() override;

	static Ref<Window> Create(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc);

	// Returns native window for the platform abstraction
	// if we are using any (like SDL) or native platform window if we don't
	virtual std::any GetNativeWindow() const = 0;
	// Returns native window for the current platform (HWND on Windows)
	virtual std::any GetPlatformNativeWindow() const = 0;

	virtual WindowDesc GetWindowParams() const = 0;
	virtual glm::int2 GetSize() const = 0;
	virtual bool GetIsFullscreen() const = 0;
	virtual std::wstring GetTitle() const = 0;

	virtual Render::Swapchain* GetSwapchain() = 0;

protected:
	Window(const WindowDesc& windowParams, const Render::SwapchainDesc& swapchainDesc) {}
};
}
