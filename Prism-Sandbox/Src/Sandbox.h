#pragma once
#include <memory>

#include "Prism-Core/Base/Application.h"
#include "Prism-Core/Render/Buffer.h"
#include "Prism-Core/Render/Layer.h"

struct Vertex
{
	alignas(16)
	glm::float3 position;
	glm::float3 color;
};

class SandboxLayer : public Prism::Render::Layer
{
public:
	SandboxLayer();

	virtual void Update(Prism::Duration delta) override;

private:
	Prism::Render::Buffer* m_vertexBuffer = nullptr;
	Prism::Render::Buffer* m_indexBuffer = nullptr;
};

class SandboxApplication final : public Prism::Core::Application
{
public:
	static SandboxApplication& Get();

	virtual void Init() override;

	Prism::Core::Window* GetWindow() const;

private:
	std::unique_ptr<Prism::Core::Window> m_window;
	std::unique_ptr<SandboxLayer> m_sandboxLayer;
};
