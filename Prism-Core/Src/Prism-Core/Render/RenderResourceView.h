#pragma once
#include "Prism-Core/Render/RenderResource.h"

namespace Prism::Render
{
class BufferView;
class TextureView;

class RenderResourceView : public RefCounted
{
public:
	constexpr virtual ResourceType GetResourceType() const = 0;

	template<typename T>
	const T* GetSubType() const
	{
		if constexpr (std::derived_from<T, BufferView>)
			PE_ASSERT(GetResourceType() == ResourceType::Buffer);
		else if constexpr (std::derived_from<T, TextureView>)
			PE_ASSERT(GetResourceType() == ResourceType::Texture);
		else
			static_assert(false);

		return static_cast<const T*>(this);
	}
};
}
