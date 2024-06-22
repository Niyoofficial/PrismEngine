#include "pcpch.h"
#include "Buffer.h"
#include "RenderResourceCreation.h"

namespace Prism::Render
{
Ref<Buffer> Buffer::Create(const BufferDesc& desc, BufferData initData, Flags<ResourceStateFlags> initState)
{
	return Private::CreateBuffer(desc, initData, initState);
}

Ref<BufferView> Buffer::CreateView(const BufferViewDesc& desc)
{
	return BufferView::Create(desc, this);
}

Ref<BufferView> Buffer::CreateDefaultView()
{
	return CreateView({
		.offset = 0,
		.size = GetBufferDesc().size
	});
}
}
