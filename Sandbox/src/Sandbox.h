#pragma once

#include "Prism/Base/Application.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Render/Camera.h"
#include "Prism/Render/Layer.h"
#include "Prism/Render/Material.h"
#include "Prism/Render/PrimitiveBatch.h"
#include "Prism/Render/Primitive.h"
#include "Prism/Render/RenderConstants.h"
#include "ImGuizmo.h"
#include "AssetBrowserPanel.h"

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

class EditorLayer : public Render::Layer
{
public:
	EditorLayer(Core::Window* owningWindow, const Ref<Scene>& scene);

	void UpdateImGui(Duration delta) override;
	virtual void Update(Duration delta) override;

private:
	bool CheckForViewportResize(glm::int2 viewportSize);

	void SelectEntityUnderCursor();

private:
	WeakRef<Core::Window> m_owningWindow;

	Ref<Scene> m_scene;
	
	ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::UNIVERSAL;
	ImGuizmo::MODE m_gizmoMode = ImGuizmo::LOCAL;

	AssetBrowserPanel m_assetBrowser;

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

	std::vector<Ref<MeshAsset>> m_meshes;

	float m_cameraSpeed = 5.f;
	float m_mouseSpeed = 0.003f;

	Duration m_lastTimeButtonDown;

	std::mutex m_fileLoadMutex;
	std::vector<std::fs::path> m_filesToLoad;
};
