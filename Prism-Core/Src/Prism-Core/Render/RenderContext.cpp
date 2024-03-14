#include "pcpch.h"
#include "RenderContext.h"

#include "Prism-Core/Render/RenderResourceCreation.h"

namespace Prism::Render
{
RenderContext* RenderContext::Create()
{
	return Private::CreateRenderContext();
}

void RenderContext::SetRenderTarget(TextureView* rtv, TextureView* dsv)
{
	PE_ASSERT(rtv != nullptr);

	SetRenderTargets({rtv}, dsv);
}

void RenderContext::SetViewport(Viewport viewport)
{
	SetViewports({viewport});
}

void RenderContext::SetScissor(Scissor scissor)
{
	SetScissors({scissor});
}
}
