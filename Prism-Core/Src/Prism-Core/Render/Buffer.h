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
};

struct BufferInitData
{
	const void* data = nullptr;
	int64_t sizeInBytes = 0;
	int64_t byteOffset = 0;
};

class Buffer : public RenderResource
{
public:
	static Buffer* Create(const BufferDesc& desc, const BufferInitData& initData = {});
	static Buffer* Create(const BufferDesc& desc, const std::vector<BufferInitData>& initData);

	BufferView* CreateView(const BufferViewDesc& desc);
	BufferView* CreateDefaultView();

	ResourceType GetResourceType() const override { return ResourceType::Buffer; }

	virtual BufferDesc GetBufferDesc() const = 0;
};
}
