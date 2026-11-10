#pragma once
#include "Prism/Render/RenderResourceView.h"

namespace Prism::Render
{
enum class BufferViewType
{
	Unknown,
	CBV,
	SRV,
	UAV,
};

enum class BufferViewFlags
{
	None = 0,
	// Can only be used on UAV
	NeedsCounter = 1 << 0
};

struct BufferViewDesc
{
	bool operator==(const BufferViewDesc&) const = default;

	BufferViewType type;
	int64_t offset = 0;
	int64_t size = 0;
	// Used only for SRV and UAV, can be left as 0 for CBV
	int64_t elementSize = 0;
	Flags<BufferViewFlags> flags;
};

class BufferView : public RenderResourceView
{
public:
	static Ref<BufferView> Create(const BufferViewDesc& desc, class Buffer* buffer);

	constexpr virtual ResourceType GetResourceType() const override { return ResourceType::Buffer; }

	virtual BufferViewDesc GetViewDesc() const = 0;

	Buffer* GetBuffer() const;

protected:
	WeakRef<Buffer> m_owningBuffer;
};
}

template<>
struct std::hash<Prism::Render::BufferViewDesc>
{
	size_t operator()(const Prism::Render::BufferViewDesc& desc) const noexcept
	{
		using namespace Prism::Render;

		return
			std::hash<BufferViewType>()(desc.type) ^
			std::hash<int64_t>()(desc.offset) ^
			std::hash<int64_t>()(desc.size) ^
			std::hash<int64_t>()(desc.elementSize) ^
			std::hash<Prism::Flags<BufferViewFlags>::MaskType>()(desc.flags.GetUnderlyingType());
	}
};
