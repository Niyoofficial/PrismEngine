#pragma once
#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/RenderResourceView.h"

namespace Prism::Render
{
enum class TextureViewType
{
	Unknown,
	SRV,
	UAV,
	RTV,
	DSV
};

struct TextureViewDesc
{
public:
	bool Is1D() const;
	bool Is2D() const;
	bool Is3D() const;
	bool IsArray() const;
	bool IsCube() const;

public:
	TextureViewType type;

	TextureFormat format = TextureFormat::Unknown;
	ResourceDimension dimension = ResourceDimension::Undefined;

	SubresourceRange subresourceRange;
};

class TextureView : public RenderResourceView
{
public:
	static TextureView* Create(const TextureViewDesc& desc, Texture* texture);

	constexpr virtual ResourceType GetResourceType() const override { return ResourceType::Texture; }

	virtual TextureViewDesc GetViewDesc() const = 0;

	Texture* GetTexture() const;

protected:
	// View keeps a strong ref to its resource to keep if alive as long as there are views to it existing
	Ref<Texture> m_owningTexture;
};
}

template<>
struct std::hash<Prism::Render::TextureViewDesc>
{
	size_t operator()(const Prism::Render::TextureViewDesc& desc) const noexcept
	{
		using namespace Prism::Render;

#ifdef PE_BUILD_DEBUG
		static_assert(sizeof(desc) == 28,
			"If new field was added, add it to the hash function and update this assert");
#endif

		return
			std::hash<TextureViewType>()(desc.type) ^
			std::hash<TextureFormat>()(desc.format) ^
			std::hash<ResourceDimension>()(desc.dimension) ^
			std::hash<SubresourceRange>()(desc.subresourceRange);
	}
};
