#pragma once
#include "Prism-Core/Render/RenderTypes.h"
#include "Prism-Core/Render/RenderResourceView.h"

namespace Prism::Render
{
enum class TextureViewType
{
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

	TextureFormat format;
	ResourceDimension dimension;

	int32_t firstMipLevel = 0;
	int32_t numMipLevels = 1;

	int32_t firstArrayOrDepthSlice = -1;
	int32_t arrayOrDepthSlicesCount = -1;
};

class TextureView : public RenderResourceView
{
public:
	static TextureView* Create(const TextureViewDesc& desc, class Texture* texture);

	ResourceType GetResourceType() const override { return ResourceType::Texture; }

	virtual Texture* GetTexture() const = 0;
};
}
