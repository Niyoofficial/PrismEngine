#include "pcpch.h"
#include "RenderAPI.h"

#include "Prism-Core/Render/RenderResourceCreation.h"

namespace Prism::Render
{
RenderAPI* RenderAPI::Create()
{
	return Private::CreateRenderAPI();
}

void RenderAPI::SetRenderTarget(TextureView* rtv, TextureView* dsv)
{
	PE_ASSERT(rtv != nullptr);

	SetRenderTargets({rtv}, dsv);
}

void RenderAPI::SetViewport(Viewport viewport)
{
	SetViewports({viewport});
}

void RenderAPI::SetScissor(Scissor scissor)
{
	SetScissors({scissor});
}
}
