#pragma once

#include "Prism-Core/Base/Application.h"
#include "Prism-Core/Render/Buffer.h"
#include "Prism-Core/Render/Camera.h"
#include "Prism-Core/Render/Layer.h"
#include "Prism-Core/Render/RenderConstants.h"


using namespace Prism;

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) CBufferTest
{
	glm::float4x4 view;
	glm::float4x4 proj;
	glm::float4x4 viewProj;
};

struct Vertex
{
	alignas(16)
	glm::float3 position;
	glm::float3 color;
	glm::float2 texCoords;
};

class SandboxLayer : public Render::Layer
{
public:
	SandboxLayer();

	virtual void Update(Duration delta) override;

private:
	Ref<Render::Camera> m_camera;

	Ref<Render::Texture> m_depthStencil;
	Ref<Render::TextureView> m_depthStencilView;
	Ref<Render::Texture> m_texture;
	Ref<Render::TextureView> m_textureView;
	Ref<Render::Buffer> m_cbuffer;
	Ref<Render::BufferView> m_cbufferView;
	Ref<Render::Buffer> m_vertexBuffer;
	Ref<Render::Buffer> m_indexBuffer;

	float m_cameraSpeed = 0.5f;
	float m_mouseSpeed = 0.005f;
};

class SandboxApplication final : public Core::Application
{
public:
	static SandboxApplication& Get();

	SandboxApplication(int32_t argc, char** argv);

	Core::Window* GetWindow() const;

private:
	Ref<Core::Window> m_window;
	Ref<SandboxLayer> m_sandboxLayer;
};
