#include "pcpch.h"
#include "RenderCommandList.h"

#include "RenderResourceCreation.h"

namespace Prism::Render
{
Ref<RenderCommandList> RenderCommandList::Create()
{
	return Private::CreateRenderCommandList();
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
