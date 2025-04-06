#pragma once
#include "Prism/Render/PipelineStateCache.h"
#include "Prism/Render/ReleaseQueue.h"
#include "Prism/Render/RenderContext.h"
#include "Prism/Render/ShaderCache.h"
#include "Prism/Utilities/StaticSingleton.h"


namespace Prism::Core
{
class Application;
class Window;
}

DECLARE_LOG_CATEGORY(PERender, "Prism-Render");
#define PE_RENDER_LOG(verbosity, ...) PE_LOG(PERender, verbosity, __VA_ARGS__)

namespace Prism::Render
{
struct RenderDeviceParams
{
	bool enableDebugLayer = false;
	bool initPixLibrary = false;
};

struct SubresourceFootprint
{
	glm::int3 size;
	int64_t rowPitch = 0;
};

class RenderDevice : public StaticPointerSingleton<RenderDevice>
{
	friend Core::Application;
public:
	static void Create(RenderDeviceParams params = {});
	static void Destroy();
	static void TryDestroy();
	static RenderDevice& Get();
	static RenderDevice* TryGet();


	explicit RenderDevice(RenderDeviceParams params);

	Ref<RenderContext> AllocateContext();
	virtual uint64_t SubmitContext(RenderContext* context);

	virtual void ReleaseStaleResources();

	// Returns the actual resource size that it would occupy in memory
	virtual int64_t GetTotalSizeInBytes(BufferDesc buffDesc) const = 0;
	/**
	 * Returns the actual resource size that it would occupy in memory
	 * @param numSubresources Return the size of the entire resource if this is set to -1
	 */
	virtual int64_t GetTotalSizeInBytes(TextureDesc texDesc, int32_t firstSubresource = 0, int32_t numSubresources = -1) const = 0;
	virtual SubresourceFootprint GetSubresourceFootprint(TextureDesc texDesc, int32_t subresourceIndex = 0) const = 0;
	virtual int64_t GetTexturePitchAlignment() const = 0;
	
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

	virtual void InitializeImGui(Core::Window* window, TextureFormat depthFormat) = 0;
	virtual void ShutdownImGui() = 0;

protected:
	// Called by Application
	virtual void BeginRenderFrame();
	virtual void EndRenderFrame();
	virtual void ImGuiNewFrame() = 0;

	void TransferEndFramePreservedObjectsToReleaseQueue();

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
