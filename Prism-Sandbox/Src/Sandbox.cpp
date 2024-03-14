#include "Sandbox.h"

#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/RenderDevice.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/TextureView.h"

Prism::Core::Application* CreateApplication(int32_t argc, char** argv)
{
	return new SandboxApplication;
}

void SandboxLayer::Update(Prism::Duration delta)
{
	using namespace Prism::Render;
	Layer::Update(delta);

	std::unique_ptr<RenderContext> renderContext;
	renderContext.reset(RenderDevice::Get().AllocateContext());

	auto* vs = Shader::Create({
		.filepath = L"Shaders/Basic.hlsl",
		.entryName = L"vsmain",
		.shaderType = ShaderType::VS
	});
	auto* ps = Shader::Create({
		.filepath = L"Shaders/Basic.hlsl",
		.entryName = L"psmain",
		.shaderType = ShaderType::PS
	});
	auto pso = GraphicsPipelineState::Create({
		.vs = vs,
		.ps = ps,
		.inputLayout = {},
		.primitiveTopologyType = TopologyType::TriangleList,
		.numRenderTargets = 1,
		.renderTargetFormats = {TextureFormat::RGBA8_UNorm}
	});

	renderContext->SetPSO(pso);

	auto* currentBackBuffer = SandboxApplication::Get().GetWindow()->GetSwapchain()->GetCurrentBackBufferRTV();
	renderContext->Transition({
		.resource = currentBackBuffer->GetTexture(),
		.oldState = ResourceStateFlags::Present,
		.newState = ResourceStateFlags::RenderTarget
	});

	glm::float4 clearColor = {0.8f, 0.2f, 0.3f, 1.f};
	renderContext->ClearRenderTargetView(currentBackBuffer, &clearColor);

	auto windowSize = SandboxApplication::Get().GetWindow()->GetSize();
	renderContext->SetViewport({{0.f, 0.f}, windowSize, {0.f, 1.f}});
	renderContext->SetScissor({{0.f, 0.f}, windowSize});
	renderContext->SetRenderTarget(currentBackBuffer, nullptr);

	renderContext->Transition({
		.resource = currentBackBuffer->GetTexture(),
		.oldState = ResourceStateFlags::RenderTarget,
		.newState = ResourceStateFlags::Present
	});

	RenderDevice::Get().SubmitContext(renderContext.get());

	SandboxApplication::Get().GetWindow()->GetSwapchain()->Present();

	RenderDevice::Get().FlushCommandQueue();
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
