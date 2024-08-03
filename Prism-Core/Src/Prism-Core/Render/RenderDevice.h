#pragma once
#include "Prism-Core/Render/RenderThreadCommands.h"
#include "Prism-Core/Render/PipelineStateCache.h"
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/ShaderCache.h"
#include "Prism-Core/Utilities/StaticSingleton.h"


DECLARE_LOG_CATEGORY(PERender, "Prism-Render");
#define PE_RENDER_LOG(verbosity, ...) PE_LOG(PERender, verbosity, __VA_ARGS__)

namespace Prism::Render
{
struct RenderDeviceParams
{
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
	virtual uint64_t SubmitContext(RenderContext* context) = 0;

	virtual void WaitForCmdListToComplete(uint64_t fenceValue) = 0;
	virtual void FlushCommandQueue() = 0;

	virtual void ReleaseStaleResources() = 0;

	virtual uint64_t GetSubmittedCmdListFenceValue() const = 0;
	virtual uint64_t GetCompletedCmdListFenceValue() const = 0;

	ShaderCache& GetShaderCache() { return m_shaderCache; }
	const ShaderCache& GetShaderCache() const { return m_shaderCache; }

	PipelineStateCache& GetPipelineStateCache() { return m_pipelineStateCache; }
	const PipelineStateCache& GetPipelineStateCache() const { return m_pipelineStateCache; }

private:
	// Holds last frame's cmd list fence values for frames
	// prepared by CPU but not yet completed by the GPU
	std::queue<uint64_t> m_cpuPreparedFrames;

	ShaderCache m_shaderCache;
	PipelineStateCache m_pipelineStateCache;

	RenderThreadCommandQueue m_rtCommandQueue;
};
}
