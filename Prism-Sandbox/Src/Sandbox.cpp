#include "Sandbox.h"

#include <array>

#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Platform.h"
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/RenderDevice.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/TextureView.h"
#include "Prism-Core/Utilities/ShapeUtils.h"


IMPLEMENT_APPLICATION(SandboxApplication);

SandboxLayer::SandboxLayer()
{
	using namespace Prism::Render;
	Layer::Attach();

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::KeyDown>(
		[](Core::AppEvent event)
		{
			if (std::get<Core::AppEvents::KeyDown>(event).keyCode == KeyCode::Escape)
				SandboxApplication::Get().CloseApplication();
		});

	Core::Platform::Get().AddAppEventCallback<Core::AppEvents::MouseMotion>(
		[this](Core::AppEvent event)
		{
			glm::float2 delta = std::get<Core::AppEvents::MouseMotion>(event).relPosition;
			delta *= m_mouseSpeed;
			m_camera->AddRotation({-delta.y, -delta.x, 0.f});
		});

	glm::int2 windowSize = SandboxApplication::Get().GetWindow()->GetSize();

	m_camera = new Camera(45.f, (float)windowSize.x / (float)windowSize.y, 0.1f, 1000.f);
	m_camera->SetPosition({0.f, 0.f, -5.f});

	TextureDesc depthStencilDesc = {
		.textureName = L"DepthStencil",
		.width = windowSize.x,
		.height = windowSize.y,
		.dimension = ResourceDimension::Tex2D,
		.format = TextureFormat::D24_UNorm_S8_UInt,
		.bindFlags = BindFlags::DepthStencil,
		.optimizedClearValue = DepthStencilClearValue{
			.format = TextureFormat::D24_UNorm_S8_UInt
		}
	};
	m_depthStencil = Texture::Create(depthStencilDesc, {}, ResourceStateFlags::DepthWrite);
	TextureViewDesc depthStencilViewDesc = {
		.type = TextureViewType::DSV,
		.dimension = ResourceDimension::Tex2D,
	};
	m_depthStencilView = m_depthStencil->CreateView(depthStencilViewDesc);

	m_texture = Texture::Create(L"Textures/CheckerBoard.jpeg");
	TextureViewDesc textureViewDesc = {
		.type = TextureViewType::SRV,
		.format = m_texture->GetTextureDesc().format,
		.dimension = m_texture->GetTextureDesc().dimension,
		.firstMipLevel = 0,
		.numMipLevels = m_texture->GetTextureDesc().mipLevels
	};
	m_textureView = m_texture->CreateView(textureViewDesc);

	BufferDesc cbufferDesc = {
		.bufferName = L"CBuffer",
		.size = sizeof(CBufferTest),
		.bindFlags = BindFlags::ConstantBuffer,
		.usage = ResourceUsage::Dynamic
	};

	CBufferTest cbufferTest = {
		.view = m_camera->GetViewMatrix(),
		.proj = m_camera->GetProjectionMatrix(),
		.viewProj = m_camera->GetViewProjectionMatrix()
	};
	m_cbuffer = Buffer::Create(cbufferDesc, {.data = &cbufferTest, .sizeInBytes = sizeof(cbufferTest)}, ResourceStateFlags::ConstantBuffer);
	m_cbufferView = m_cbuffer->CreateDefaultView();

	//       5(-1,+1,+1)________________6(+1,+1,+1)
	//                 /|              /|
	//                / |             / |
	//               /  |            /  |
	//              /   |           /   |
	//  4(-1,-1,+1)/____|__________/7(+1,-1,+1)
	//             |    |__________|____|
	//             |   /1(-1,+1,-1)|    /2(+1,+1,-1)
	//             |  /            |   /
	//             | /             |  /
	//             |/              | /
	//             /_______________|/
	//         0(-1,-1,-1)      3(+1,-1,-1)

	/*std::array vertices = {
		Vertex{ // 0
			.position = {-1, -1, -1},
			.color = {0.8f, 0.2f, 0.3f},
			.texCoords = {0.f, 1.f}
		},
		Vertex{ // 1
			.position = {-1, +1, -1},
			.color = {0.3f, 0.2f, 0.8f},
			.texCoords = {0.f, 0.f}
		},
		Vertex{ // 2
			.position = {+1, +1, -1},
			.color = {0.2f, 0.8f, 0.3f},
			.texCoords = {1.f, 0.f}
		},
		Vertex{ // 3
			.position = {+1, -1, -1},
			.color = {0.8f, 0.2f, 0.3f},
			.texCoords = {1.f, 1.f}
		},
		Vertex{ // 4
			.position = {-1, -1, +1},
			.color = {0.3f, 0.2f, 0.8f},
			.texCoords = {0.f, 1.f}
		},
		Vertex{ // 5
			.position = {-1, +1, +1},
			.color = {0.2f, 0.8f, 0.3f},
			.texCoords = {0.f, 0.f}
		},
		Vertex{ // 6
			.position = {+1, +1, +1},
			.color = {0.8f, 0.2f, 0.3f},
			.texCoords = {1.f, 0.f}
		},
		Vertex{ // 7
			.position = {+1, -1, +1},
			.color = {0.3f, 0.2f, 0.8f},
			.texCoords = {1.f, 1.f}
		},
	};*/

	ShapeUtils::ShapeData monkey = ShapeUtils::LoadShapeFromFile(L"Meshes/Monkey.fbx");
	std::vector<Vertex> vertices;
	vertices.reserve(monkey.vertices.size());
	for (ShapeUtils::VertexData& vertex : monkey.vertices)
	{
		vertices.push_back({
			.position = vertex.position,
			.color = vertex.vertexColor,
			.texCoords = vertex.texCoord
		});
	}
	BufferDesc vertexBufferDesc = {
		.bufferName = L"Vertex Buffer",
		.size = (int64_t)(vertices.size() * sizeof(Vertex)),
		.bindFlags = BindFlags::VertexBuffer
	};
	BufferData vertexInitData = {
		.data = vertices.data(),
		.sizeInBytes = (int64_t)(vertices.size() * sizeof(Vertex))
	};
	m_vertexBuffer = Buffer::Create(vertexBufferDesc, {vertexInitData});

	/*constexpr std::array<uint16_t, 36> indices = {
		2,1,0, 2,0,3,
		4,5,6, 4,6,7,
		0,4,7, 0,7,3,
		1,4,0, 1,5,4,
		1,2,5, 5,2,6,
		3,7,6, 3,6,2
	};*/
	std::vector<int16_t> indices;
	indices.reserve(monkey.indices.size());
	for (int32_t index : monkey.indices)
	{
		indices.push_back((int16_t)index);
	}
	BufferDesc indexBufferDesc = {
		.bufferName = L"Index Buffer",
		.size = (int64_t)(indices.size() * sizeof(uint16_t)),
		.bindFlags = BindFlags::IndexBuffer
	};
	BufferData indexInitData = {
		.data = indices.data(),
		.sizeInBytes = indexBufferDesc.size
	};
	m_indexBuffer = Buffer::Create(indexBufferDesc, {indexInitData});

	Ref renderContext = RenderDevice::Get().AllocateContext();

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

	RenderDevice::Get().SubmitContext(renderContext);
	RenderDevice::Get().FlushCommandQueue();
}

void SandboxLayer::Update(Duration delta)
{
	using namespace Prism::Render;
	Layer::Update(delta);

	if (Core::Platform::Get().IsKeyPressed(KeyCode::W))
		m_camera->AddPosition(m_camera->GetForwardVector() * m_cameraSpeed);
	if (Core::Platform::Get().IsKeyPressed(KeyCode::S))
		m_camera->AddPosition(-m_camera->GetForwardVector() * m_cameraSpeed);
	if (Core::Platform::Get().IsKeyPressed(KeyCode::A))
		m_camera->AddPosition(-m_camera->GetRightVector() * m_cameraSpeed);
	if (Core::Platform::Get().IsKeyPressed(KeyCode::D))
		m_camera->AddPosition(m_camera->GetRightVector() * m_cameraSpeed);
	if (Core::Platform::Get().IsKeyPressed(KeyCode::Q))
		m_camera->AddPosition(-m_camera->GetUpVector() * m_cameraSpeed);
	if (Core::Platform::Get().IsKeyPressed(KeyCode::E))
		m_camera->AddPosition(m_camera->GetUpVector() * m_cameraSpeed);

	Ref<RenderContext> renderContext = RenderDevice::Get().AllocateContext();

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
			.depthEnable = true,
			.depthWriteEnable = true
		},
		.primitiveTopologyType = TopologyType::TriangleList,
		.numRenderTargets = 1,
		.renderTargetFormats = {TextureFormat::RGBA8_UNorm},
		.depthStencilFormat = m_depthStencil->GetTextureDesc().format
	});

	renderContext->SetPSO(pso);

	renderContext->SetVertexBuffer(m_vertexBuffer, sizeof(Vertex));
	renderContext->SetIndexBuffer(m_indexBuffer, IndexBufferFormat::Uint16);

	renderContext->SetTexture(m_textureView, L"g_texture");

	glm::float2 windowSize = SandboxApplication::Get().GetWindow()->GetSize();

	CBufferTest cbufferTest = {
		.view = m_camera->GetViewMatrix(),
		.proj = m_camera->GetProjectionMatrix(),
		.viewProj = m_camera->GetViewProjectionMatrix()
	};
	void* cbufferData = m_cbuffer->Map(CPUAccess::Write);
	memcpy_s(cbufferData, m_cbuffer->GetBufferDesc().size, &cbufferTest, sizeof(cbufferTest));
	m_cbuffer->Unmap();

	renderContext->SetCBuffer(m_cbufferView, L"TestBuffer");

	auto* currentBackBuffer = SandboxApplication::Get().GetWindow()->GetSwapchain()->GetCurrentBackBufferRTV();
	renderContext->Transition({
		.resource = currentBackBuffer->GetTexture(),
		.oldState = ResourceStateFlags::Present,
		.newState = ResourceStateFlags::RenderTarget
	});

	renderContext->SetViewport({{0.f, 0.f}, windowSize, {0.f, 1.f}});
	renderContext->SetScissor({{0.f, 0.f}, windowSize});
	renderContext->SetRenderTarget(currentBackBuffer, m_depthStencilView);

	glm::float4 clearColor = {0.8f, 0.2f, 0.3f, 1.f};
	renderContext->ClearRenderTargetView(currentBackBuffer, &clearColor);
	renderContext->ClearDepthStencilView(m_depthStencilView, Flags(ClearFlags::ClearDepth) | Flags(ClearFlags::ClearStencil));

	renderContext->DrawIndexed({
		.numIndices = (int32_t)(m_indexBuffer->GetBufferDesc().size / (int64_t)sizeof(int16_t)),
		.numInstances = 1,
		.startIndexLocation = 0,
		.baseVertexLocation = 0
	});

	renderContext->Transition({
		.resource = currentBackBuffer->GetTexture(),
		.oldState = ResourceStateFlags::RenderTarget,
		.newState = ResourceStateFlags::Present
	});

	RenderDevice::Get().SubmitContext(renderContext);

	SandboxApplication::Get().GetWindow()->GetSwapchain()->Present();

	RenderDevice::Get().FlushCommandQueue();
}

SandboxApplication& SandboxApplication::Get()
{
	return Application::Get<SandboxApplication>();
}

SandboxApplication::SandboxApplication(int32_t argc, char** argv)
{
	InitPlatform();
	InitRenderer();

	Core::WindowDesc windowParams = {
		.windowTitle = L"Test",
		.windowSize = {2560, 1440},
		.fullscreen = false
	};
	Render::SwapchainDesc swapchainDesc = {
		.refreshRate = {
			.numerator = 60,
			.denominator = 1
		},
		.format = Render::TextureFormat::RGBA8_UNorm,
		.sampleDesc = {
			.count = 1,
			.quality = 0
		},
		.bufferCount = 3
	};
	m_window = Core::Window::Create(windowParams, swapchainDesc);

	Core::Platform::Get().SetMouseRelativeMode(true);

	m_sandboxLayer = new SandboxLayer();
	PushLayer(m_sandboxLayer);
}

Core::Window* SandboxApplication::GetWindow() const
{
	return m_window;
}
