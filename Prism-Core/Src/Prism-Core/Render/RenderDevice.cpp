#include "pcpch.h"
#include "RenderDevice.h"

#include "Prism-Core/Render/RenderResourceCreation.h"


namespace Prism::Render
{
void RenderDevice::Create()
{
	Private::CreateRenderDevice();
}

void RenderDevice::Destroy()
{
	StaticPointerSingleton<RenderDevice>::Destroy();
}

void RenderDevice::TryDestroy()
{
	StaticPointerSingleton<RenderDevice>::TryDestroy();
}

RenderDevice& RenderDevice::Get()
{
	return StaticPointerSingleton<RenderDevice>::Get();
}

RenderDevice* RenderDevice::TryGet()
{
	return StaticPointerSingleton<RenderDevice>::TryGet();
}

RenderContext* RenderDevice::AllocateContext()
{
	return Private::CreateRenderContext();
}
}
