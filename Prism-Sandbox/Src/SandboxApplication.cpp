#include "SandboxApplication.h"

#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Window.h"

Prism::Core::Application* CreateApplication(int32_t argc, char** argv)
{
	return new SandboxApplication;
}

void SandboxApplication::Init()
{
	Application::Init();

	InitPlatform();
	InitRenderer();

	Prism::Core::WindowParameters windowParams = {
		.windowTitle = "Test",
		.windowSize = {2560, 1440},
		.fullscreen = false
	};
	Prism::Render::SwapchainDesc swapchainDesc = {
		.refreshRate = {
			.numerator = 60,
			.denominator = 1
		},
		.format = Prism::Render::TextureFormat::RGBA8_UNorm,
		.sampleDesc = {
			.count = 1,
			.quality = 0
		},
		.bufferCount = 3
	};
	m_window.reset(Prism::Core::Window::Create(windowParams, swapchainDesc));
}
