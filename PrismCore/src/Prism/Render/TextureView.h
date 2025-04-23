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
	ResourceDimension dimension = ResourceDimension::Tex2D;

	// TODO: Replace with subresource range?
	int32_t firstMipLevel = 0;
	// Set to -1 to use all mip levels
	int32_t numMipLevels = -1;

	int32_t firstArrayOrDepthSlice = -1;
	int32_t arrayOrDepthSlicesCount = -1;
};

class TextureView : public RenderResourceView
{
public:
	static TextureView* Create(const TextureViewDesc& desc, Texture* texture);

	constexpr virtual ResourceType GetResourceType() const override { return ResourceType::Texture; }

	Texture* GetTexture() const;

protected:
	// View keeps a strong ref to its resource to keep if alive as long as there are views to it existing
	Ref<Texture> m_owningTexture;
};
}
