#pragma once

#include "Prism/Base/Application.h"
#include "Prism/Render/Buffer.h"
#include "Prism/Render/Camera.h"
#include "Prism/Render/Layer.h"
#include "Prism/Render/Material.h"
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
	glm::float4x4 invView;
	glm::float4x4 invProj;
	glm::float4x4 invViewProj;

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

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) SceneUniformBuffer
{
	alignas(16)
	CBufferCamera camera;
};

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) LightsUniformBuffer
{
	alignas(16)
	DirectionalLight directionalLights[16];
	alignas(16)
	PointLight pointLights[16];
};

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) PerLightUniformBuffer
{
	alignas(16)
	int32_t lightIndex = -1;
	alignas(16)
	glm::float4x4 shadowViewProj;
};

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) ShadowUniformBuffer
{
	alignas(16)
	glm::float4x4 lightViewProj;
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
	float mipLevel = 0.f;

	alignas(16)
	Material material;
};

struct alignas(Render::Constants::CBUFFER_ALIGNMENT) CBufferModelShadow
{
	glm::float4x4 world;
};

struct GBuffer
{
	enum class Type
	{
		Depth,
		Color,
		Normal,
		Roughness_Metal_AO,
		Count
	};

	void CreateResources(glm::int2 windowSize);

	Render::Texture* GetTexture(Type type)
	{
		return entries[(size_t)type].texture.Raw();
	}
	Render::TextureView* GetView(Type type, Render::TextureViewType view)
	{
		return entries[(size_t)type].views.at(view).Raw();
	}
	
	struct Entry
	{
		Ref<Render::Texture> texture;
		std::unordered_map<Render::TextureViewType, Ref<Render::TextureView>> views;
	};

	std::array<Entry, (size_t)Type::Count> entries;
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

	float m_environmentLightScale = 1.f;
	glm::float3 m_sunRotation = {0.f, glm::radians(-7.f), glm::radians(-101.f)};

	Ref<Render::Camera> m_camera;

	GBuffer m_gbuffer;

	Ref<Render::Texture> m_skybox;
	Ref<Render::TextureView> m_skyboxCubeSRVView;
	Ref<Render::TextureView> m_skyboxArraySRVView;
	Ref<Render::TextureView> m_skyboxUAVView;

	Ref<Render::Texture> m_prefilteredSkybox;
	Ref<Render::TextureView> m_prefilteredEnvMapCubeSRVView;

	Ref<Render::Texture> m_BRDFLUT;

	Ref<Render::Texture> m_environmentTexture;
	Ref<Render::TextureView> m_environmentTextureSRV;

	Ref<Render::Texture> m_sunShadowMap;
	Ref<Render::TextureView> m_sunShadowMapDSV;
	Ref<Render::TextureView> m_sunShadowMapSRV;

	Ref<Render::Buffer> m_sceneUniBuffer;
	Ref<Render::BufferView> m_sceneUniBufferView;

	Ref<Render::Buffer> m_lightsUniBuffer;
	Ref<Render::BufferView> m_lightsUniBufferView;

	Ref<Render::Buffer> m_perLightUniBuffer;
	Ref<Render::BufferView> m_perLightUniBufferView;

	Ref<Render::Buffer> m_sceneShadowUniformBuffer;
	Ref<Render::BufferView> m_sceneShadowView;

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

	static Ref<Render::PrimitiveBatch> LoadMeshFromFile(const std::wstring& primitiveBatchName, const std::wstring& filepath,
														std::function<Render::Material(const MeshUtils::PrimitiveData&)> createMaterialFunc,
														std::wstring primitiveBufferParamName, int64_t primitiveBufferSize, MeshUtils::MeshData* outMeshData = nullptr);
	static Ref<Render::PrimitiveBatch> LoadMeshFromFilePBR(const std::wstring& primitiveBatchName, const std::wstring& filepath);

private:
	Ref<Core::Window> m_window;
	Ref<SandboxLayer> m_sandboxLayer;
};
