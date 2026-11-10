#include "pcpch.h"
#include "BufferView.h"

#include "Prism/Render/RenderResourceCreation.h"

namespace Prism::Render
{
Ref<BufferView> BufferView::Create(const BufferViewDesc& desc, Buffer* buffer)
{
	return RenderDevice::Get().CreateBufferView(desc, buffer);
}

Buffer* BufferView::GetBuffer() const
{
	return m_owningBuffer.IsValid() ? m_owningBuffer.Raw() : nullptr;
}
}
