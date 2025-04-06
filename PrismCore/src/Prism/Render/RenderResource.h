#pragma once

namespace Prism::Render
{
enum class ResourceType
{
	Buffer,
	Texture
};

class Buffer;
class Texture;

class RenderResource : public RefCounted
{
public:
	virtual ResourceType GetResourceType() const = 0;

	template<typename T>
	T* GetSubType() const
	{
		if constexpr (std::derived_from<T, Buffer>)
			PE_ASSERT(GetResourceType() == ResourceType::Buffer);
		else if constexpr (std::derived_from<T, Texture>)
			PE_ASSERT(GetResourceType() == ResourceType::Texture);
		else
			static_assert(true);

		return static_cast<const T*>(this);
	}
};
}
