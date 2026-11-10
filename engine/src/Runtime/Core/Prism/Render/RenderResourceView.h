#pragma once
#include "Prism/Render/RenderResource.h"

namespace Prism::Render
{
class BufferView;
class TextureView;

class RenderResourceView : public RefCounted
{
public:
	constexpr virtual ResourceType GetResourceType() const = 0;

	template<typename T>
	const T* GetSubType() const requires std::derived_from<T, BufferView> || std::derived_from<T, TextureView>
	{
		if constexpr (std::derived_from<T, BufferView>)
			PE_ASSERT(GetResourceType() == ResourceType::Buffer);
		else if constexpr (std::derived_from<T, TextureView>)
			PE_ASSERT(GetResourceType() == ResourceType::Texture);

		return static_cast<const T*>(this);
	}

	template<typename T>
	T* GetSubType() requires std::derived_from<T, BufferView> || std::derived_from<T, TextureView>
	{
		if constexpr (std::derived_from<T, BufferView>)
			PE_ASSERT(GetResourceType() == ResourceType::Buffer);
		else if constexpr (std::derived_from<T, TextureView>)
			PE_ASSERT(GetResourceType() == ResourceType::Texture);

		return static_cast<T*>(this);
	}

	bool IsViewOfDynamicResource() const;
};
}
