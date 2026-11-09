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
	explicit RenderResource(class RenderDevice* renderDevice) : m_renderDevice(renderDevice) {}

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

	template<typename T>
	T* GetSubType()
	{
		if constexpr (std::derived_from<T, Buffer>)
			PE_ASSERT(GetResourceType() == ResourceType::Buffer);
		else if constexpr (std::derived_from<T, Texture>)
			PE_ASSERT(GetResourceType() == ResourceType::Texture);
		else
			static_assert(true);

		return static_cast<T*>(this);
	}

protected:
	RenderDevice* m_renderDevice = nullptr;
};
}
