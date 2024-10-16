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

void RenderDevice::BeginRenderFrame()
{
}

void RenderDevice::EndRenderFrame()
{
	m_cpuPreparedFrames.emplace(GetLastSubmittedCmdListFenceValue());

	for (auto& res : m_endFramePreservedObjects.GetPreservedObjects())
		m_releaseQueue.AddResource(std::move(res), GetLastSubmittedCmdListFenceValue());
	m_endFramePreservedObjects.GetPreservedObjects().clear();

	ReleaseStaleResources();

	while (!m_cpuPreparedFrames.empty() && GetLastCompletedCmdListFenceValue() >= m_cpuPreparedFrames.front())
		m_cpuPreparedFrames.pop();

	while (m_cpuPreparedFrames.size() >= Constants::MAX_FRAMES_IN_FLIGHT)
	{
		WaitForCmdListToComplete(m_cpuPreparedFrames.front());
		m_cpuPreparedFrames.pop();
	}
}

Ref<RenderContext> RenderDevice::AllocateContext()
{
	return Private::CreateRenderContext();
}
}
