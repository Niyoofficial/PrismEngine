#pragma once

#include <vector>

namespace Prism::Render
{
class Layer;
}

namespace Prism::Core
{
class Application
{
public:
	virtual ~Application() = default;

	void Run();

	void PushLayer(Render::Layer* layer);
	void PopLayer(Render::Layer* layer);

protected:
	virtual void Init();
	virtual void Shutdown();

	void InitPlatform();
	void ShutdownPlatform();

	void InitRenderer();
	void ShutdownRenderer();

protected:
	bool m_running = false;
	uint64_t m_frameCounter = 0;

	std::vector<Render::Layer*> m_layerStack;
};
}
