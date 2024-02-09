#include "pcpch.h"
#include "Renderer.h"

#include "Prism-Core/Render/RenderAPI.h"


namespace Prism::Render
{
void Renderer::Create()
{
	StaticPointerSingleton<Renderer>::Create();
}

void Renderer::Destroy()
{
	StaticPointerSingleton<Renderer>::Destroy();
}

void Renderer::TryDestroy()
{
	StaticPointerSingleton<Renderer>::TryDestroy();
}

Renderer& Renderer::Get()
{
	return StaticPointerSingleton<Renderer>::Get();
}

Renderer::Renderer()
	: m_renderAPI(RenderAPI::Create())
{
}
}
