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

	m_shaderCompiler.reset(Private::CreateShaderCompiler());
}

void RenderDevice::BeginRenderFrame()
{
}

void RenderDevice::EndRenderFrame()
{
	// Add a new CPU prepared frame
	m_cpuPreparedFrames.emplace(GetRenderCommandQueue()->GetLastSubmittedCmdListFenceValue());

	GetRenderCommandQueue()->ExecuteGPUCompletionEvents();

	TransferEndFramePreservedObjectsToReleaseQueue();
	ReleaseStaleResources();

	// Pop any completed CPU frames
	while (!m_cpuPreparedFrames.empty() && GetRenderCommandQueue()->GetCompletedFenceValue() >= m_cpuPreparedFrames.front())
		m_cpuPreparedFrames.pop();

	// If there is too many CPU prepared frames, wait for them to be completed by the GPU and pop them
	while (m_cpuPreparedFrames.size() >= Constants::MAX_FRAMES_IN_FLIGHT)
	{
		GetRenderCommandQueue()->WaitForFenceToComplete(m_cpuPreparedFrames.front());
		m_cpuPreparedFrames.pop();
	}
}

Ref<RenderContext> RenderDevice::AllocateContext()
{
	return new RenderContext;
}

uint64_t RenderDevice::SubmitContext(RenderContext* context)
{
	return GetRenderCommandQueue()->Submit(context);
}

void RenderDevice::SetBypassCommandRecording(bool bypass)
{
	m_bypassCommandRecording = bypass;
}

bool RenderDevice::GetBypassCommandRecording() const
{
	return m_bypassCommandRecording;
}

void RenderDevice::ReleaseStaleResources()
{
	m_releaseQueue.PurgeReleaseQueue(GetRenderCommandQueue()->GetCompletedFenceValue());
	GetRenderCommandQueue()->ReleaseStaleResources();
}

void RenderDevice::TransferEndFramePreservedObjectsToReleaseQueue()
{
	for (auto& res : m_endFramePreservedObjects.GetPreservedObjects())
		m_releaseQueue.AddResource(std::move(res), GetRenderCommandQueue()->GetLastSubmittedCmdListFenceValue());
	m_endFramePreservedObjects.GetPreservedObjects().clear();
}
}
