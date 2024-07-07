#include "pcpch.h"
#include "RenderDevice.h"

#include "Prism-Core/Render/RenderResourceCreation.h"
#include "Prism-Core/Utilities/ShapeUtils.h"


namespace Prism::Render
{
void RenderDevice::Create(RenderDeviceParams params)
{
	Private::CreateRenderDevice(params);
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

RenderDevice::RenderDevice(RenderDeviceParams params)
{
	ShapeUtils::InitShapeLoading();
}

Ref<RenderContext> RenderDevice::AllocateContext()
{
	return Private::CreateRenderContext();
}
}
