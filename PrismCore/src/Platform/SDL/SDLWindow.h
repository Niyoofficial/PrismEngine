#pragma once
#include "Prism/Base/Base.h"
#include "Prism/Base/Window.h"

struct SDL_Window;

namespace Prism::SDL
{
class SDLWindow final : public Core::Window
{
public:
	explicit SDLWindow(const Core::WindowDesc& windowParams, const Render::SwapchainDesc& swapchainDesc);

	virtual std::any GetNativeWindow() const override;
	virtual std::any GetPlatformNativeWindow() const override;

	virtual Core::WindowDesc GetWindowParams() const override;
	virtual glm::int2 GetSize() const override;
	virtual bool GetIsFullscreen() const override;
	virtual std::wstring GetTitle() const override;

	virtual Render::Swapchain* GetSwapchain() override { return m_swapchain; }

private:
	SDL_Window* m_nativeWindow = nullptr;
	bool m_windowed = false;

	Ref<Render::Swapchain> m_swapchain;
};
}
