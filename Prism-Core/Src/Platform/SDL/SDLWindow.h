#pragma once
#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Window.h"

class SDL_Window;

namespace Prism::SDL
{
class SDLWindow final : public Core::Window
{
public:
	explicit SDLWindow(const Core::WindowParameters& windowParams, const Render::SwapchainDesc& swapchainDesc);

	virtual std::any GetNativeWindow() const override;
	virtual std::any GetPlatformNativeWindow() const override;

	virtual Core::WindowParameters GetWindowParams() const override;
	virtual glm::int2 GetSize() const override;
	virtual bool GetIsFullscreen() const override;
	virtual std::string GetTitle() const override;

	virtual Render::Swapchain* GetSwapchain() override { return m_swapchain.get(); }

private:
	SDL_Window* m_nativeWindow = nullptr;
	bool m_windowed = false;

	std::unique_ptr<Render::Swapchain> m_swapchain;
};
}
