#pragma once

#include "Prism/Base/Application.h"
#include "Prism/Render/Buffer.h"
#include "Prism/Render/Camera.h"
#include "Prism/Render/Layer.h"
#include "Prism/Render/PrimitiveBatch.h"
#include "Prism/Render/Primitive.h"
#include "Prism/Render/RenderConstants.h"
#include "Prism/Utilities/MeshUtils.h"


using namespace Prism;

struct CBufferCamera
{
	glm::float4x4 view;
	glm::float4x4 proj;
	glm::float4x4 viewProj;

	glm::float3 camPos;
};

struct DirectionalLight
{
	alignas(16)
	glm::float3 direction;
	alignas(16)
	glm::float3 lightColor;
};

struct PointLight
{
	alignas(16)
	glm::float3 position;
	alignas(16)
	glm::float3 lightColor;
};

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) CBufferScene
{
	alignas(16)
	float environmentDiffuseScale = 1.f;
	alignas(16)
	CBufferCamera camera;

	alignas(16)
	DirectionalLight directionalLights[16];
	alignas(16)
	PointLight pointLights[16];
};

struct Material
{
	glm::float3 albedo;
	float metallic = 0.f;
	float roughness = 0.f;
	float ao = 0.f;
};

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) CBufferModel
{
	glm::float4x4 world;
	alignas(16)
	glm::float4x4 normalMatrix;

	alignas(16)
	Material material;
};

struct Vertex
{
	glm::float3 position;
	glm::float3 normal;
	glm::float3 tangent;
	glm::float3 bitangent;
	glm::float3 color;
	glm::float2 texCoords;
};

class SandboxLayer : public Render::Layer
{
public:
	SandboxLayer(Core::Window* owningWindow);

	void UpdateImGui(Duration delta) override;
	virtual void Update(Duration delta) override;

private:
	WeakRef<Core::Window> m_owningWindow;

	bool m_showStatWindow = true;
	float m_environmentDiffuseScale = 1.f;

	Ref<Render::Camera> m_camera;

	Ref<Render::Texture> m_depthStencil;
	Ref<Render::TextureView> m_depthStencilView;

	Ref<Render::Texture> m_skybox;
	Ref<Render::TextureView> m_skyboxCubeSRVView;
	Ref<Render::TextureView> m_skyboxArraySRVView;
	Ref<Render::TextureView> m_skyboxUAVView;

	Ref<Render::Texture> m_prefilteredEnvMap;
	Ref<Render::TextureView> m_prefilteredEnvMapCubeSRVView;
	Ref<Render::TextureView> m_prefilteredEnvMapUAVView;

	Ref<Render::Texture> m_irradiance;
	Ref<Render::TextureView> m_irradianceSRVView;
	Ref<Render::TextureView> m_irradianceUAVView;

	Ref<Render::Texture> m_rustedIronAlbedo;
	Ref<Render::TextureView> m_rustedIronAlbedoView;
	Ref<Render::Texture> m_rustedIronMetallic;
	Ref<Render::TextureView> m_rustedIronMetallicView;
	Ref<Render::Texture> m_rustedIronRoughness;
	Ref<Render::TextureView> m_rustedIronRoughnessView;
	Ref<Render::Texture> m_rustedIronNormal;
	Ref<Render::TextureView> m_rustedIronNormalView;

	Ref<Render::Texture> m_environmentTexture;
	Ref<Render::TextureView> m_environmentTextureView;

	Ref<Render::Buffer> m_sceneCbuffer;
	Ref<Render::BufferView> m_sceneCbufferView;

	Ref<Render::Buffer> m_coeffBuffer;
	Ref<Render::BufferView> m_coeffBufferView;
	Ref<Render::Buffer> m_irradianceSHBuffer;
	Ref<Render::BufferView> m_irradianceSHBufferView;

	Ref<Render::PrimitiveBatch> m_sponza;
	Ref<Render::PrimitiveBatch> m_cube;
	Ref<Render::PrimitiveBatch> m_sphere;

	float m_cameraSpeed = 0.05f;
	float m_mouseSpeed = 0.003f;
};

class SandboxApplication final : public Core::Application
{
public:
	struct PrimitiveData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::unordered_map<MeshUtils::TextureType, Ref<Render::Texture>> textures;
	};

	struct MeshData
	{
		std::vector<PrimitiveData> primitives;
	};

public:
	static SandboxApplication& Get();

	SandboxApplication(int32_t argc, char** argv);

	Core::Window* GetWindow() const;

	static MeshData ConvertMeshToSandboxFormat(const MeshUtils::MeshData& mesh);

private:
	Ref<Core::Window> m_window;
	Ref<SandboxLayer> m_sandboxLayer;
};
