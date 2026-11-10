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
	std::string windowTitle;
	glm::int2 windowSize;
	bool fullscreen = false;
};

class Window : public RefCounted
{
public:
	virtual ~Window() override;

	static Ref<Window> Create(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc);

	virtual void DestroyWindow();

	// Returns native window for the platform abstraction
	// if we are using any (like SDL) or native platform window if we don't
	virtual void* GetNativeWindow() const = 0;
	// Returns native window for the current platform (HWND on Windows)
	virtual void* GetPlatformNativeWindow() const = 0;

	virtual WindowDesc GetWindowParams() const = 0;
	virtual glm::int2 GetSize() const = 0;
	virtual bool GetIsFullscreen() const = 0;
	virtual std::string GetTitle() const = 0;

	virtual Render::Swapchain* GetSwapchain() = 0;

protected:
	Window(const WindowDesc& windowParams, const Render::SwapchainDesc& swapchainDesc) {}
};
} // namespace Prism::Core
