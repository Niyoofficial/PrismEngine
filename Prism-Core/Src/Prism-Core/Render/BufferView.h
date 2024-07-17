#pragma once
#include "Prism-Core/Render/RenderResourceView.h"

namespace Prism::Render
{
struct BufferViewDesc
{
	int64_t offset = 0;
	int64_t size = 0;
};

class BufferView : public RenderResourceView
{
public:
	static Ref<BufferView> Create(const BufferViewDesc& desc, class Buffer* buffer);

	constexpr virtual ResourceType GetResourceType() const override { return ResourceType::Buffer; }

	Buffer* GetBuffer() const;

protected:
	// View keeps a strong ref to its resource to keep if alive as long as there are views to it existing
	Ref<Buffer> m_owningBuffer;
};
}
