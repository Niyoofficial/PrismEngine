#include "pcpch.h"
#include "Buffer.h"
#include "RenderResourceCreation.h"

namespace Prism::Render
{
Buffer* Buffer::Create(const BufferDesc& desc, const BufferInitData& initData)
{
	return Buffer::Create(desc, std::vector{initData});
}

Buffer* Buffer::Create(const BufferDesc& desc, const std::vector<BufferInitData>& initData)
{
	return Private::CreateBuffer(desc, initData);
}

BufferView* Buffer::CreateView(const BufferViewDesc& desc)
{
	return BufferView::Create(desc, this);
}

BufferView* Buffer::CreateDefaultView()
{
	return CreateView({
		.offset = 0,
		.size = GetBufferDesc().size
	});
}
}
