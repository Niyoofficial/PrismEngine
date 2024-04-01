#include "Sandbox.h"

#include <array>

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

SandboxLayer::SandboxLayer()
{
	using namespace Prism::Render;
	Layer::Attach();

	std::array vertices = {
		Vertex{
			.position = {-1, -1, -1},
			.color = {0.8f, 0.2f, 0.3f}
		},
		Vertex{
			.position = {-1, +1, -1},
			.color = {0.3f, 0.2f, 0.8f}
		},
		Vertex{
			.position = {+1, +1, -1},
			.color = {0.2f, 0.8f, 0.3f}
		},
		Vertex{
			.position = {+1, -1, -1},
			.color = {0.8f, 0.2f, 0.3f}
		},
		Vertex{
			.position = {-1, -1, +1},
			.color = {0.3f, 0.2f, 0.8f}
		},
		Vertex{
			.position = {-1, +1, +1},
			.color = {0.2f, 0.8f, 0.3f}
		},
		Vertex{
			.position = {+1, +1, +1},
			.color = {0.8f, 0.2f, 0.3f}
		},
		Vertex{
			.position = {+1, -1, +1},
			.color = {0.3f, 0.2f, 0.8f}
		},
	};
	BufferDesc vertexBufferDesc = {
		.bufferName = L"Vertex Buffer",
		.size = vertices.size() * sizeof(Vertex),
		.bindFlags = BindFlags::VertexBuffer,
		.usage = ResourceUsage::Staging
	};
	BufferInitData vertexInitData = {
		.data = vertices.data(),
		.sizeInBytes = vertices.size() * sizeof(Vertex)
	};
	m_vertexBuffer = Buffer::Create(vertexBufferDesc, vertexInitData);

	constexpr std::array<uint16_t, 36> indices = {
		2,0,1, 2,3,0,
		4,6,5, 4,7,6,
		0,7,4, 0,3,7,
		1,0,4, 1,4,5,
		1,5,2, 5,6,2,
		3,6,7, 3,2,6
	};
	BufferDesc indexBufferDesc = {
		.bufferName = L"Index Buffer",
		.size = indices.size() * sizeof(uint16_t),
		.bindFlags = BindFlags::IndexBuffer,
		.usage = ResourceUsage::Staging
	};
	BufferInitData indexInitData = {
		.data = indices.data(),
		.sizeInBytes = indexBufferDesc.size
	};
	m_indexBuffer = Buffer::Create(indexBufferDesc, indexInitData);


	std::unique_ptr<RenderContext> renderContext;
	renderContext.reset(RenderDevice::Get().AllocateContext());

	renderContext->Transition({
		.resource = m_vertexBuffer,
		.oldState = ResourceStateFlags::Common,
		.newState = ResourceStateFlags::VertexBuffer
	});

	renderContext->Transition({
		.resource = m_indexBuffer,
		.oldState = ResourceStateFlags::Common,
		.newState = ResourceStateFlags::IndexBuffer
	});

	RenderDevice::Get().SubmitContext(renderContext.get());
	RenderDevice::Get().FlushCommandQueue();
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
		.depthStencilState = {
			.depthEnable = false,
			.depthWriteEnable = false
		},
		.primitiveTopologyType = TopologyType::TriangleList,
		.numRenderTargets = 1,
		.renderTargetFormats = {TextureFormat::RGBA8_UNorm}
	});

	renderContext->SetPSO(pso);

	renderContext->SetVertexBuffer(m_vertexBuffer, sizeof(Vertex));
	renderContext->SetIndexBuffer(m_indexBuffer, IndexBufferFormat::Uint16);
	 
	auto* currentBackBuffer = SandboxApplication::Get().GetWindow()->GetSwapchain()->GetCurrentBackBufferRTV();
	renderContext->Transition({
		.resource = currentBackBuffer->GetTexture(),
		.oldState = ResourceStateFlags::Present,
		.newState = ResourceStateFlags::RenderTarget
	});

	auto windowSize = SandboxApplication::Get().GetWindow()->GetSize();
	renderContext->SetViewport({{0.f, 0.f}, windowSize, {0.f, 1.f}});
	renderContext->SetScissor({{0.f, 0.f}, windowSize});
	renderContext->SetRenderTarget(currentBackBuffer, nullptr);

	glm::float4 clearColor = {0.8f, 0.2f, 0.3f, 1.f};
	renderContext->ClearRenderTargetView(currentBackBuffer, &clearColor);

	renderContext->DrawIndexed({
		.numIndices = 36,
		.numInstances = 1,
		.startIndexLocation = 0,
		.baseVertexLocation = 0
	});

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
