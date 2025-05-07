#include "pcpch.h"
#include "Buffer.h"
#include "RenderResourceCreation.h"
#include "Prism/Render/RenderCommandQueue.h"

namespace Prism::Render
{
Ref<Buffer> Buffer::Create(const BufferDesc& desc, RawData initData)
{
	Ref<Buffer> buffer = Private::CreateBuffer(desc);

	if (initData.data && initData.sizeInBytes > 0)
	{
		if (desc.usage == ResourceUsage::Dynamic || desc.usage == ResourceUsage::Staging)
		{
			void* address = buffer->Map(CPUAccess::Write);
			memcpy_s(address, desc.size, initData.data, initData.sizeInBytes);
			buffer->Unmap();
		}
		else if (desc.usage == ResourceUsage::Default)
		{
			// TODO: Add copy context
			Ref context = RenderDevice::Get().AllocateContext(L"UpdateDefaultBuffer");

			context->UpdateBuffer(buffer, initData);

			RenderDevice::Get().SubmitContext(context);
			RenderDevice::Get().GetRenderCommandQueue()->Flush();
		}
		else
		{
			PE_ASSERT_NO_ENTRY();
		}
	}

	return buffer;
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
