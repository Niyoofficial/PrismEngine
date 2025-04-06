#include "pcpch.h"
#include "RenderDevice.h"

#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderResourceCreation.h"
#include "Prism/Utilities/MeshUtils.h"


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
	MeshUtils::InitMeshLoading();
}

void RenderDevice::BeginRenderFrame()
{
}

void RenderDevice::EndRenderFrame()
{
	// Add a new CPU prepared frame
	m_cpuPreparedFrames.emplace(GetRenderQueue()->GetLastSubmittedCmdListFenceValue());

	GetRenderQueue()->ExecuteGPUCompletionEvents();

	TransferEndFramePreservedObjectsToReleaseQueue();
	ReleaseStaleResources();

	// Pop any completed CPU frames
	while (!m_cpuPreparedFrames.empty() && GetRenderQueue()->GetCompletedFenceValue() >= m_cpuPreparedFrames.front())
		m_cpuPreparedFrames.pop();

	// If there is too many CPU prepared frames, wait for them to be completed by the GPU and pop them
	while (m_cpuPreparedFrames.size() >= Constants::MAX_FRAMES_IN_FLIGHT)
	{
		GetRenderQueue()->WaitForFenceToComplete(m_cpuPreparedFrames.front());
		m_cpuPreparedFrames.pop();
	}
}

Ref<RenderContext> RenderDevice::AllocateContext()
{
	return new RenderContext;
}

uint64_t RenderDevice::SubmitContext(RenderContext* context)
{
	return GetRenderQueue()->Submit(context);
}

void RenderDevice::ReleaseStaleResources()
{
	m_releaseQueue.PurgeReleaseQueue(GetRenderQueue()->GetCompletedFenceValue());
	GetRenderQueue()->ReleaseStaleResources();
}

void RenderDevice::TransferEndFramePreservedObjectsToReleaseQueue()
{
	for (auto& res : m_endFramePreservedObjects.GetPreservedObjects())
		m_releaseQueue.AddResource(std::move(res), GetRenderQueue()->GetLastSubmittedCmdListFenceValue());
	m_endFramePreservedObjects.GetPreservedObjects().clear();
}
}
