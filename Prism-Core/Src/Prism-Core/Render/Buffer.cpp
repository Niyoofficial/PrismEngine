#include "pcpch.h"
#include "Buffer.h"
#include "RenderResourceCreation.h"

namespace Prism::Render
{
Ref<Buffer> Buffer::Create(const BufferDesc& desc, RawData initData)
{
	return Private::CreateBuffer(desc, initData);
}

Ref<BufferView> Buffer::CreateView(const BufferViewDesc& desc)
{
	return BufferView::Create(desc, this);
}

Ref<BufferView> Buffer::CreateDefaultCBVView()
{
	auto desc = GetBufferDesc();
	return CreateView({
		.type = BufferViewType::CBV,
		.offset = 0,
		.size = desc.size
	});
}

Ref<BufferView> Buffer::CreateDefaultSRVView(int64_t elementSize)
{
	auto desc = GetBufferDesc();
	return CreateView({
		.type = BufferViewType::SRV,
		.offset = 0,
		.size = desc.size,
		.elementSize = elementSize
	});
}

Ref<BufferView> Buffer::CreateDefaultUAVView(int64_t elementSize, bool bNeedsCounter)
{
	auto desc = GetBufferDesc();
	return CreateView({
		.type = BufferViewType::UAV,
		.offset = 0,
		.size = desc.size,
		.elementSize = elementSize,
		.flags = bNeedsCounter ? BufferViewFlags::NeedsCounter : BufferViewFlags::None
	});
}
}
