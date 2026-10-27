#pragma once
#include "Prism/Render/ReleaseQueue.h"
#include "Prism/Render/RenderContext.h"
#include "Prism/Render/RenderResourceViewCache.h"
#include "Prism/Render/ShaderCompiler.h"
#include "Prism/Render/VertexBufferCache.h"
#include "Prism/Utilities/StaticSingleton.h"


namespace Prism::Core
{
class Application;
class Window;
}

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
	friend Buffer;
	friend Texture;
	friend RenderResourceViewCache;
public:
	static void Create(RenderDeviceParams params = {});
	static void Destroy();
	static void TryDestroy();
	static RenderDevice& Get();
	static RenderDevice* TryGet();


	explicit RenderDevice(RenderDeviceParams params);

	Ref<RenderContext> AllocateContext(std::wstring debugName = L"");
	virtual uint64_t SubmitContext(Ref<RenderContext>& context);

	Ref<Buffer> CreateBuffer(const BufferDesc& desc, RawData initData);
	virtual Ref<Buffer> CreateBuffer(const BufferDesc& desc) = 0;

	Ref<Texture> CreateTexture(const TextureDesc& desc, BarrierLayout initLayout, RawData initData);
	Ref<Texture> CreateTexture(const TextureDesc& desc, const Ref<Buffer>& initDataBuffer, BarrierLayout initLayout);
	virtual Ref<Texture> CreateTexture(const TextureDesc& desc, BarrierLayout initLayout) = 0;
	virtual Ref<Texture> CreateTexture(std::wstring filepath, bool loadAsCubemap, bool waitForLoadFinish) = 0;
	virtual Ref<Texture> CreateTexture(std::wstring name, void* imageData, int64_t dataSize, bool loadAsCubemap, bool waitForLoadFinish) = 0;

	virtual Ref<BufferView> CreateBufferView(const BufferViewDesc& desc, const Ref<Buffer>& buffer);
	virtual Ref<TextureView> CreateTextureView(const TextureViewDesc& desc, const Ref<Texture>& texture);


	void SetBypassCommandRecording(bool bypass);
	bool GetBypassCommandRecording() const;

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
	
	virtual RenderCommandQueue* GetRenderCommandQueue() const = 0;

	ShaderCompiler* GetShaderCompiler() const { return m_shaderCompiler.get(); }

	const VertexBufferCache& GetVertexBufferCache() const { return m_vertexBufferCache; }
	VertexBufferCache& GetVertexBufferCache() { return m_vertexBufferCache; }

	template<typename T>
	void AddResourceToReleaseQueue(const Ref<T>& resource, uint64_t fenceValue)
	{
		m_releaseQueue.AddResource(resource, fenceValue);
	}

	template<typename T>
	void AddResourceToReleaseQueueWhenFrameEnds(const Ref<T>& resource)
	{
		m_endFramePreservedObjects.AddObject(resource);
	}

	virtual void InitializeImGui(Core::Window* window, TextureFormat depthFormat) = 0;
	virtual void ShutdownImGui() = 0;

protected:
	void InitDeviceSubsystems();

	// Called by Application
	virtual void BeginRenderFrame();
	virtual void EndRenderFrame();
	virtual void ImGuiNewFrame() = 0;

	// Called by Buffer
	virtual void NotifyResourceDestruction(Buffer* resource);
	// Called by Texture
	virtual void NotifyResourceDestruction(Texture* resource);

	void TransferEndFramePreservedObjectsToReleaseQueue();

	virtual Ref<BufferView> CreateBufferView_Impl(const BufferViewDesc& desc, Buffer* buffer) = 0;
	virtual Ref<TextureView> CreateTextureView_Impl(const TextureViewDesc& desc, Texture* texture) = 0;

protected:
	// Holds last frame's cmd list fence values for frames
	// prepared by CPU but not yet completed by the GPU
	std::queue<uint64_t> m_cpuPreparedFrames;

	std::unique_ptr<ShaderCompiler> m_shaderCompiler;

	PreservingObjectContainer m_endFramePreservedObjects;
	ReleaseQueueAny m_releaseQueue;

	bool m_bypassCommandRecording = false;

	VertexBufferCache m_vertexBufferCache;
	RenderResourceViewCache m_renderResourceViewCache;
};
}
