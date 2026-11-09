#include "pcpch.h"
#include "Buffer.h"

#include "Prism/Render/RenderDevice.h"

namespace Prism::Render
{
Ref<Buffer> Buffer::Create(const BufferDesc& desc, RawData initData)
{
	return RenderDevice::Get().CreateBuffer(desc, initData);
}

Ref<BufferView> Buffer::CreateView(const BufferViewDesc& desc)
{
	return BufferView::Create(desc, this);
}

Ref<BufferView> Buffer::CreateDefaultUniformBufferView()
{
	auto desc = GetBufferDesc();
	return CreateView({
		.type = BufferViewType::CBV,
		.offset = 0,
		.size = desc.size
	});
}

Ref<BufferView> Buffer::CreateDefaultSRV(int64_t elementSize)
{
	auto desc = GetBufferDesc();
	return CreateView({
		.type = BufferViewType::SRV,
		.offset = 0,
		.size = desc.size,
		.elementSize = elementSize
	});
}

Ref<BufferView> Buffer::CreateDefaultUAV(int64_t elementSize, bool bNeedsCounter)
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

static int32_t test = 0;
Buffer::Buffer(RenderDevice* renderDevice)
	: RenderResource(renderDevice)
{
	test++;
}

Buffer::~Buffer()
{
	test--;
	PE_ASSERT(m_renderDevice);

	m_renderDevice->NotifyResourceDestruction(this);
}
}
