#include "pcpch.h"
#include "BufferView.h"

#include "Prism-Core/Render/RenderResourceCreation.h"

namespace Prism::Render
{
Ref<BufferView> BufferView::Create(const BufferViewDesc& desc, Buffer* buffer)
{
	return Private::CreateBufferView(desc, buffer);
}
}
