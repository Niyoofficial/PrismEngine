#pragma once

#include "Prism/Base/Application.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Render/Camera.h"
#include "Prism/Render/Layer.h"
#include "Prism/Render/Material.h"
#include "Prism/Render/PrimitiveBatch.h"
#include "Prism/Render/Primitive.h"
#include "Prism/Render/RenderConstants.h"
#include "Prism/Utilities/MeshLoading.h"
#include "ImGuizmo.h"

using namespace Prism;

namespace Prism::Render
{
class PBRSceneRenderPipeline;
}

struct alignas(Render::Constants::UNIFORM_BUFFER_ALIGNMENT) PerLightUniformBuffer
{
	alignas(16)
	int32_t lightIndex = -1;
	alignas(16)
	glm::float4x4 shadowViewProj;
};

struct alignas(Render::Constants::UNIFORM_BUFFER_ALIGNMENT) ShadowUniformBuffer
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

struct alignas(Render::Constants::UNIFORM_BUFFER_ALIGNMENT) ModelUniformBuffer
{
	glm::float4x4 world;
	alignas(16)
	glm::float4x4 normalMatrix;

	alignas(16)
	float mipLevel = 0.f;

	alignas(16)
	Material material;
};

struct alignas(Render::Constants::UNIFORM_BUFFER_ALIGNMENT) BloomSettingsUniformBuffer
{
	float threshold = 1.f;
	float knee = 0.1f;
	int32_t lod = 0;
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
	bool CheckForViewportResize(glm::int2 viewportSize);

	void SelectEntityUnderCursor();

private:
	WeakRef<Core::Window> m_owningWindow;

	Ref<Scene> m_scene;
	Render::PBRSceneRenderPipeline* m_renderPipeline;

	glm::float3 m_sunRotation = {0.f, glm::radians(-7.f), glm::radians(-101.f)};

	ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::UNIVERSAL;
	ImGuizmo::MODE m_gizmoMode = ImGuizmo::LOCAL;

	glm::int2 m_viewportSize;
	bool m_viewportHovered = false;
	glm::int2 m_viewportPosition = {};
	bool m_viewportRelativeMouse = false;

	Ref<Render::Camera> m_camera;

	Ref<Render::Texture> m_editorViewport;
	Ref<Render::TextureView> m_editorViewportSRV;
	Ref<Render::TextureView> m_editorViewportRTV;

	Ref<Render::Texture> m_hitProxiesTexture;

	Ref<Render::PrimitiveBatch> m_cube;
	Ref<Render::PrimitiveBatch> m_sphere;

	std::vector<Ref<MeshLoading::MeshAsset>> m_meshes;

	float m_cameraSpeed = 5.f;
	float m_mouseSpeed = 0.003f;

	Duration m_lastTimeButtonDown;
};

class SandboxApplication final : public Core::Application
{
public:
	struct PrimitiveData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::unordered_map<MeshLoading::TextureType, Ref<Render::Texture>> textures;
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
														std::function<Render::Material(const MeshLoading::PrimitiveData&)> createMaterialFunc,
														std::wstring primitiveBufferParamName, int64_t primitiveBufferSize, MeshLoading::MeshData* outMeshData = nullptr);
	static Ref<Render::PrimitiveBatch> LoadMeshFromFilePBR(const std::wstring& primitiveBatchName, const std::wstring& filepath);

private:
	Ref<Core::Window> m_window;
	Ref<SandboxLayer> m_sandboxLayer;
};
