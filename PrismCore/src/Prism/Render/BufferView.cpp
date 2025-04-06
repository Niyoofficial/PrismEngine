#include "pcpch.h"
#include "BufferView.h"

#include "Prism/Render/RenderResourceCreation.h"

namespace Prism::Render
{
Ref<BufferView> BufferView::Create(const BufferViewDesc& desc, Buffer* buffer)
{
	return Private::CreateBufferView(desc, buffer);
}

Buffer* BufferView::GetBuffer() const
{
	PE_ASSERT(m_owningBuffer);
	return m_owningBuffer;
}
}
