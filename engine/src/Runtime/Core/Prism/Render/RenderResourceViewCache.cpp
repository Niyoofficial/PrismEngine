#include "pcpch.h"
#include "RenderResourceViewCache.h"

#include "Prism/Render/RenderDevice.h"

namespace Prism::Render
{
Ref<BufferView> RenderResourceViewCache::GetOrCreateBufferView(const BufferViewDesc& desc, Buffer* buffer)
{
	PE_ASSERT(buffer);

	if (m_bufferViewCache.contains(buffer))
	{
		if (m_bufferViewCache.at(buffer).contains(desc))
		{
			PE_ASSERT(m_bufferViewCache.at(buffer).at(desc));
			return m_bufferViewCache.at(buffer).at(desc);
		}
	}

	auto bufferView = m_renderDevice->CreateBufferView_Impl(desc, buffer);
	m_bufferViewCache[buffer][desc] = bufferView;
	return bufferView;
}

Ref<TextureView> RenderResourceViewCache::GetOrCreateTextureView(const TextureViewDesc& desc, Texture* texture)
{
	PE_ASSERT(texture);

	if (m_textureViewCache.contains(texture))
	{
		if (m_textureViewCache.at(texture).contains(desc))
		{
			PE_ASSERT(m_textureViewCache.at(texture).at(desc));
			return m_textureViewCache.at(texture).at(desc);
		}
	}

	auto textureView = m_renderDevice->CreateTextureView_Impl(desc, texture);
	m_textureViewCache[texture][desc] = textureView;
	return textureView;
}

void RenderResourceViewCache::NotifyResourceDestruction(Buffer* resource)
{
	if (m_bufferViewCache.contains(resource))
		m_bufferViewCache.erase(resource);
}

void RenderResourceViewCache::NotifyResourceDestruction(Texture* resource)
{
	if (m_textureViewCache.contains(resource))
		m_textureViewCache.erase(resource);
}

void RenderResourceViewCache::ClearCache()
{
	m_bufferViewCache.clear();
	m_textureViewCache.clear();
}
}
