#pragma once
#include "Prism/Render/BufferView.h"
#include "Prism/Render/RenderResource.h"
#include "Prism/Render/RenderTypes.h"

namespace Prism::Render
{
struct BufferDesc
{
	std::wstring bufferName;

	int64_t size = 0;

	Flags<BindFlags> bindFlags = BindFlags::None;
	ResourceUsage usage = ResourceUsage::Default;
	Flags<CPUAccess> cpuAccess = CPUAccess::None;
};

class Buffer : public RenderResource
{
public:
	static Ref<Buffer> Create(const BufferDesc& desc, RawData initData = {});

	Ref<BufferView> CreateView(const BufferViewDesc& desc);
	Ref<BufferView> CreateDefaultCBVView();
	Ref<BufferView> CreateDefaultSRVView(int64_t elementSize);
	Ref<BufferView> CreateDefaultUAVView(int64_t elementSize, bool bNeedsCounter = false);

	ResourceType GetResourceType() const override { return ResourceType::Buffer; }

	virtual BufferDesc GetBufferDesc() const = 0;

	// Only Dynamic and Staging buffers can be mapped
	virtual void* Map(Flags<CPUAccess> access) = 0;
	virtual void Unmap() = 0;
};
}
