#pragma once

#include "Prism-Core/Base/Application.h"
#include "Prism-Core/Render/Buffer.h"
#include "Prism-Core/Render/Camera.h"
#include "Prism-Core/Render/Layer.h"
#include "Prism-Core/Render/RenderConstants.h"
#include "Prism-Core/Utilities/ShapeUtils.h"


using namespace Prism;

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) CBufferCamera
{
	glm::float4x4 view;
	glm::float4x4 proj;
	glm::float4x4 viewProj;
};

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) CBufferModel
{
	glm::float4x4 world;
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
	Ref<Render::Buffer> m_cameraCbuffer;
	Ref<Render::BufferView> m_cameraCbufferView;
	Ref<Render::Buffer> m_modelCbuffer;
	Ref<Render::BufferView> m_modelCbufferView;
	Ref<Render::Buffer> m_monkeyVertexBuffer;
	Ref<Render::Buffer> m_monkeyIndexBuffer;
	Ref<Render::Buffer> m_floorVertexBuffer;
	Ref<Render::Buffer> m_floorIndexBuffer;

	float m_cameraSpeed = 0.05f;
	float m_mouseSpeed = 0.005f;
};

class SandboxApplication final : public Core::Application
{
public:
	static SandboxApplication& Get();

	SandboxApplication(int32_t argc, char** argv);

	Core::Window* GetWindow() const;

	static std::vector<Vertex> GetVerticesFromShapeData(const ShapeUtils::ShapeData& shapeData);
	static std::vector<uint32_t> GetIndicesFromShapeData(const ShapeUtils::ShapeData& shapeData);

private:
	Ref<Core::Window> m_window;
	Ref<SandboxLayer> m_sandboxLayer;
};
