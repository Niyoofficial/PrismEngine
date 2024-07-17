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

	Ref<RenderContext> AllocateContext();
	virtual void SubmitContext(RenderContext* context) = 0;

	virtual void FlushCommandQueue() = 0;

	virtual uint64_t GetCompletedCmdListFenceValue() const = 0;

	ShaderCache& GetShaderCache() { return m_shaderCache; }
	const ShaderCache& GetShaderCache() const { return m_shaderCache; }

	PipelineStateCache& GetPipelineStateCache() { return m_pipelineStateCache; }
	const PipelineStateCache& GetPipelineStateCache() const { return m_pipelineStateCache; }

private:
	int64_t m_frameCounter = 0;

	ShaderCache m_shaderCache;
	PipelineStateCache m_pipelineStateCache;

	RenderThreadCommandQueue m_rtCommandQueue;
};
}
