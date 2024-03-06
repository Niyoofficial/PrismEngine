#pragma once
#include <memory>

#include "Prism-Core/Base/Application.h"
#include "Prism-Core/Render/Layer.h"

class SandboxLayer : public Prism::Render::Layer
{
public:
	virtual void Update(Prism::Duration delta) override;
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
