#include "Sandbox.h"

#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/RenderAPI.h"
#include "Prism-Core/Render/Renderer.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/TextureView.h"

Prism::Core::Application* CreateApplication(int32_t argc, char** argv)
{
	return new SandboxApplication;
}

void SandboxLayer::Update(Prism::Duration delta)
{
	Layer::Update(delta);

	auto* renderAPI = Prism::Render::Renderer::Get().GetRenderAPI();

	renderAPI->Begin();

	auto* currentBackBuffer = SandboxApplication::Get().GetWindow()->GetSwapchain()->GetCurrentBackBufferRTV();
	renderAPI->Transition({
		.resource = currentBackBuffer->GetTexture(),
		.oldState = Prism::Render::ResourceStateFlags::Present,
		.newState = Prism::Render::ResourceStateFlags::RenderTarget
	});

	glm::float4 clearColor = {0.8f, 0.2f, 0.3f, 1.f};
	renderAPI->ClearRenderTargetView(currentBackBuffer, &clearColor);

	auto windowSize = SandboxApplication::Get().GetWindow()->GetSize();
	renderAPI->SetViewport({{0.f, 0.f}, windowSize, {0.f, 1.f}});
	renderAPI->SetScissor({{0.f, 0.f}, windowSize});
	renderAPI->SetRenderTarget(currentBackBuffer, nullptr);

	renderAPI->Transition({
		.resource = currentBackBuffer->GetTexture(),
		.oldState = Prism::Render::ResourceStateFlags::RenderTarget,
		.newState = Prism::Render::ResourceStateFlags::Present
	});

	renderAPI->End();

	SandboxApplication::Get().GetWindow()->GetSwapchain()->Present();

	renderAPI->FlushCommandQueue();
}

SandboxApplication& SandboxApplication::Get()
{
	return Application::Get<SandboxApplication>();
}

void SandboxApplication::Init()
{
	Application::Init();

	InitPlatform();
	InitRenderer();

	Prism::Core::WindowDesc windowParams = {
		.windowTitle = L"Test",
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

	m_sandboxLayer = std::make_unique<SandboxLayer>();
	PushLayer(m_sandboxLayer.get());
}

Prism::Core::Window* SandboxApplication::GetWindow() const
{
	return m_window.get();
}
