#pragma once
#include "Prism-Core/Render/RenderThreadCommands.h"
#include "Prism-Core/Utilities/StaticSingleton.h"

namespace Prism::Render
{
class Renderer : StaticPointerSingleton<Renderer>
{
public:
	static void Create();
	static void Destroy();
	static void TryDestroy();
	static Renderer& Get();


	Renderer();

	class RenderAPI* GetRenderAPI() const { return m_renderAPI; }

	template<typename Func>
	void Submit(Func&& function)
	{
		m_rtCommandQueue.RecordCommandToCurrentCmdList(std::forward<Func>(function));
	}

private:
	RenderAPI* m_renderAPI = nullptr;

	int64_t m_frameCounter = 0;

	RenderThreadCommandQueue m_rtCommandQueue;
};
}
