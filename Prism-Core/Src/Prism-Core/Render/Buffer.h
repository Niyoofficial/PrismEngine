#pragma once
#include "Prism-Core/Render/BufferView.h"
#include "Prism-Core/Render/RenderResource.h"
#include "Prism-Core/Render/RenderTypes.h"

namespace Prism::Render
{
struct BufferDesc
{
	std::wstring bufferName;

	int64_t size = 0;

	Flags<BindFlags> bindFlags = BindFlags::None;
	ResourceUsage usage = ResourceUsage::Default;
	CPUAccess cpuAccess = CPUAccess::None;
};

struct BufferData
{
	const void* data = nullptr;
	int64_t sizeInBytes = 0;
};

class Buffer : public RenderResource
{
public:
	static Ref<Buffer> Create(const BufferDesc& desc, BufferData initData,
							  Flags<ResourceStateFlags> initState = ResourceStateFlags::Common);

	Ref<BufferView> CreateView(const BufferViewDesc& desc);
	Ref<BufferView> CreateDefaultView();

	ResourceType GetResourceType() const override { return ResourceType::Buffer; }

	virtual BufferDesc GetBufferDesc() const = 0;

	// Only Dynamic and Staging buffers can be mapped
	virtual void* Map(CPUAccess access) = 0;
	virtual void Unmap() = 0;
};
}
