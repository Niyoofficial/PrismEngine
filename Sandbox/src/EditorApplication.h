#pragma once
#include "Sandbox.h"
#include "Prism/Base/Application.h"

namespace Prism
{
class EditorApplication final : public Core::Application
{
public:
	static EditorApplication& Get();

	EditorApplication(int32_t argc, char** argv);

	virtual Ref<Core::Window> GetMainWindow() const override { return m_window; }

protected:
	void InitImGui(Core::Window* window, Render::TextureFormat depthFormat) override;

private:
	Ref<Core::Window> m_window;
	Ref<EditorLayer> m_sandboxLayer;

	Ref<Scene> m_scene;
};
}
