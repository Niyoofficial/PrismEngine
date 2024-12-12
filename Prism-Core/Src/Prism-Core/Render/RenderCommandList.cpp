#include "pcpch.h"
#include "RenderCommandList.h"

#include "RenderResourceCreation.h"

namespace Prism::Render
{
RenderCommandList::RenderCommandList(uint64_t fenceValue)
	: m_fenceValue(fenceValue)
{
}

Ref<RenderCommandList> RenderCommandList::Create(uint64_t fenceValue)
{
	return Private::CreateRenderCommandList(fenceValue);
}

void RenderCommandList::Close()
{
	m_closed = true;
}

bool RenderCommandList::IsClosed() const
{
	return m_closed;
}
}
