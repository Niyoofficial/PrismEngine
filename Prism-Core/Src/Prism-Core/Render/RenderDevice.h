#pragma once
#include "Prism-Core/Render/PipelineStateCache.h"
#include "Prism-Core/Render/ReleaseQueue.h"
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/ShaderCache.h"
#include "Prism-Core/Utilities/StaticSingleton.h"


DECLARE_LOG_CATEGORY(PERender, "Prism-Render");
#define PE_RENDER_LOG(verbosity, ...) PE_LOG(PERender, verbosity, __VA_ARGS__)

namespace Prism::Render
{
struct RenderDeviceParams
{
	bool enableDebugLayer = false;
	bool initPixLibrary = false;
};

class RenderDevice : public StaticPointerSingleton<RenderDevice>
{
public:
	static void Create(RenderDeviceParams params = {});
	static void Destroy();
	static void TryDestroy();
	static RenderDevice& Get();
	static RenderDevice* TryGet();


	explicit RenderDevice(RenderDeviceParams params);

	// Called by Application
	virtual void BeginRenderFrame();
	virtual void EndRenderFrame();


	Ref<RenderContext> AllocateContext();
	virtual uint64_t SubmitContext(RenderContext* context);

	virtual void ReleaseStaleResources();

	virtual RenderCommandQueue* GetRenderQueue() const = 0;

	ShaderCache& GetShaderCache() { return m_shaderCache; }
	const ShaderCache& GetShaderCache() const { return m_shaderCache; }

	PipelineStateCache& GetPipelineStateCache() { return m_pipelineStateCache; }
	const PipelineStateCache& GetPipelineStateCache() const { return m_pipelineStateCache; }

	template<typename T>
	void AddResourceToReleaseQueue(T&& resource, uint64_t fenceValue)
	{
		m_releaseQueue.AddResource(std::move(resource), fenceValue);
	}

	template<typename T>
	void AddResourceToReleaseQueueWhenFrameEnds(T&& resource)
	{
		m_endFramePreservedObjects.AddObject(std::move(resource));
	}

protected:
	// Holds last frame's cmd list fence values for frames
	// prepared by CPU but not yet completed by the GPU
	std::queue<uint64_t> m_cpuPreparedFrames;

	ShaderCache m_shaderCache;
	PipelineStateCache m_pipelineStateCache;

	PreservingObjectContainer m_endFramePreservedObjects;
	ReleaseQueueAny m_releaseQueue;
};
}
