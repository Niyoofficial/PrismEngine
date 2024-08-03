#include "Sandbox.h"

#include "Prism-Core/Base/Base.h"
#include "Prism-Core/Base/Platform.h"
#include "Prism-Core/Base/Window.h"
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/RenderDevice.h"
#include "Prism-Core/Render/Texture.h"
#include "Prism-Core/Render/TextureView.h"


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

	m_camera = new Camera(45.f, (float)windowSize.x / (float)windowSize.y, 0.1f, 10000.f);
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


	// Camera cbuffer
	m_cameraCbuffer = Buffer::Create({
										 .bufferName = L"CameraCBuffer",
										 .size = sizeof(CBufferCamera),
										 .bindFlags = BindFlags::ConstantBuffer,
										 .usage = ResourceUsage::Dynamic,
										 .cpuAccess = CPUAccess::Write
									 }, {}, ResourceStateFlags::ConstantBuffer);
	m_cameraCbufferView = m_cameraCbuffer->CreateDefaultView();


	// Load monkey
	ShapeUtils::ShapeData monkey = ShapeUtils::LoadShapeFromFile(L"Meshes/Monkey.fbx");
	std::vector<Vertex> monkeyVertices = SandboxApplication::GetVerticesFromShapeData(monkey);
	std::vector<uint32_t> monkeyIndices = SandboxApplication::GetIndicesFromShapeData(monkey);

	m_monkey = new Primitive(L"Monkey", sizeof(Vertex), IndexBufferFormat::Uint32,
							 monkeyVertices.data(), (int64_t)monkeyVertices.size(),
							 monkeyIndices.data(), (int64_t)monkeyIndices.size(),
							 L"ModelBuffer", sizeof(CBufferModel));

	// Load floor
	ShapeUtils::ShapeData floor = ShapeUtils::LoadShapeFromFile(L"Meshes/Floor.fbx");
	std::vector<Vertex> floorVertices = SandboxApplication::GetVerticesFromShapeData(floor);
	std::vector<uint32_t> floorIndices = SandboxApplication::GetIndicesFromShapeData(floor);

	m_floor = new Primitive(L"Floor", sizeof(Vertex), IndexBufferFormat::Uint32,
							floorVertices.data(), (int64_t)floorVertices.size(),
							floorIndices.data(), (int64_t)floorIndices.size(),
							L"ModelBuffer", sizeof(CBufferModel));
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

	renderContext->SetTexture(m_textureView, L"g_texture");

	glm::float2 windowSize = SandboxApplication::Get().GetWindow()->GetSize();

	// Camera cbuffer
	{
		CBufferCamera cbufferCamera = {
			.view = m_camera->GetViewMatrix(),
			.proj = m_camera->GetProjectionMatrix(),
			.viewProj = m_camera->GetViewProjectionMatrix()
		};
		void* cameraCBufferData = m_cameraCbuffer->Map(CPUAccess::Write);
		memcpy_s(cameraCBufferData, m_cameraCbuffer->GetBufferDesc().size, &cbufferCamera, sizeof(cbufferCamera));
		m_cameraCbuffer->Unmap();

		renderContext->SetCBuffer(m_cameraCbufferView, L"CameraBuffer");
	}

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

	// Monkey
	{
		auto* pso = GraphicsPipelineState::Create({
			.vs = Shader::Create({
				.filepath = L"Shaders/Basic.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			}),
			.ps = Shader::Create({
				.filepath = L"Shaders/Basic.hlsl",
				.entryName = L"monkeypsmain",
				.shaderType = ShaderType::PS
			}),
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

		CBufferModel modelData = {
			.world = glm::translate(glm::float4x4(1.f), glm::float3(0.f, 2.f, 0.f))
		};
		m_monkey->BindPrimitive(renderContext, &modelData, sizeof(CBufferModel));
		m_monkey->DrawPrimitive(renderContext);
	}

	// Floor
	{
		auto* pso = GraphicsPipelineState::Create({
			.vs = Shader::Create({
				.filepath = L"Shaders/Basic.hlsl",
				.entryName = L"vsmain",
				.shaderType = ShaderType::VS
			}),
			.ps = Shader::Create({
				.filepath = L"Shaders/Basic.hlsl",
				.entryName = L"floorpsmain",
				.shaderType = ShaderType::PS
			}),
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

		CBufferModel modelData = {
			.world = glm::float4x4(1.f)
		};
		m_floor->BindPrimitive(renderContext, &modelData, sizeof(CBufferModel));
		m_floor->DrawPrimitive(renderContext);
	}

	renderContext->Transition({
		.resource = currentBackBuffer->GetTexture(),
		.oldState = ResourceStateFlags::RenderTarget,
		.newState = ResourceStateFlags::Present
	});

	RenderDevice::Get().SubmitContext(renderContext);

	SandboxApplication::Get().GetWindow()->GetSwapchain()->Present();

	//RenderDevice::Get().FlushCommandQueue();
}

SandboxApplication& SandboxApplication::Get()
{
	return Application::Get<SandboxApplication>();
}

SandboxApplication::SandboxApplication(int32_t argc, char** argv)
{
	InitPlatform();
	InitRenderer({.initPixLibrary = false});

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
		}
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

std::vector<Vertex> SandboxApplication::GetVerticesFromShapeData(const ShapeUtils::ShapeData& shapeData)
{
	std::vector<Vertex> vertices;
	vertices.reserve(shapeData.vertices.size());
	for (const ShapeUtils::VertexData& vertex : shapeData.vertices)
	{
		vertices.push_back({
			.position = vertex.position,
			.color = vertex.vertexColor,
			.texCoords = vertex.texCoord
		});
	}

	return vertices;
}

std::vector<uint32_t> SandboxApplication::GetIndicesFromShapeData(const ShapeUtils::ShapeData& shapeData)
{
	std::vector<uint32_t> indices;
	indices.reserve(shapeData.indices.size());
	for (uint32_t index : shapeData.indices)
		indices.push_back(index);

	return indices;
}
