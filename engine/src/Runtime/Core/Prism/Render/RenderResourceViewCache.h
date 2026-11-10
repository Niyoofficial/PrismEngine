#pragma once
#include "Prism/Render/BufferView.h"
#include "Prism/Render/TextureView.h"

namespace Prism::Render
{
class Texture;
class Buffer;

// Cache to avoid recreating resource views
class RenderResourceViewCache
{
public:
	explicit RenderResourceViewCache(RenderDevice* renderDevice) : m_renderDevice(renderDevice) {}

	Ref<BufferView> GetOrCreateBufferView(const BufferViewDesc& desc, Buffer* buffer);
	Ref<TextureView> GetOrCreateTextureView(const TextureViewDesc& desc, Texture* texture);

	void NotifyResourceDestruction(Buffer* resource);
	void NotifyResourceDestruction(Texture* resource);

	void ClearCache();

private:
	RenderDevice* m_renderDevice = nullptr;

	template<typename ResType, typename DescType, typename ViewType>
	using ResourceViewMap = std::unordered_map<ResType*, std::unordered_map<DescType, Ref<ViewType>>>;

	ResourceViewMap<Buffer, BufferViewDesc, BufferView> m_bufferViewCache;
	ResourceViewMap<Texture, TextureViewDesc, TextureView> m_textureViewCache;
};
}
