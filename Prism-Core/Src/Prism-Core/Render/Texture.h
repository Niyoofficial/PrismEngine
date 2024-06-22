#pragma once
#include <variant>

#include "Prism-Core/Render/RenderResource.h"
#include "Prism-Core/Render/RenderTypes.h"
#include "Prism-Core/Render/TextureView.h"

namespace Prism::Render
{
struct TextureDesc
{
public:
	static TextureDesc CreateTex1D(std::wstring inTextureName,
								   int32_t inWidth,
								   TextureFormat inFormat, Flags<BindFlags> inBindFlags,
								   ResourceUsage inUsage,
								   int32_t inMipLevels = 1, SampleDesc inSampleDesc = {});
	static TextureDesc CreateTex2D(std::wstring inTextureName,
								   int32_t inWidth, int32_t inHeight,
								   TextureFormat inFormat, Flags<BindFlags> inBindFlags,
								   ResourceUsage inUsage,
								   int32_t inMipLevels = 1, SampleDesc inSampleDesc = {});
	static TextureDesc CreateTex3D(std::wstring inTextureName,
								   int32_t inWidth, int32_t inHeight, int32_t inDepth,
								   TextureFormat inFormat, Flags<BindFlags> inBindFlags,
								   ResourceUsage inUsage,
								   int32_t inMipLevels = 1, SampleDesc inSampleDesc = {});

	bool Is1D() const;
	bool Is2D() const;
	bool Is3D() const;
	bool IsArray() const;
	bool IsCube() const;

	int32_t GetWidth() const;
	int32_t GetHeight() const;
	int32_t GetArraySize() const;
	int32_t GetDepth() const;
	int32_t GetDepthOrArraySize() const;

public:
	std::wstring textureName;

	int32_t width = 0;
	int32_t height = 1;
	int32_t depthOrArraySize = 1;
	ResourceDimension dimension = ResourceDimension::Tex2D;

	TextureFormat format = TextureFormat::Unknown;
	int32_t mipLevels = 1;
	SampleDesc sampleDesc;

	Flags<BindFlags> bindFlags = BindFlags::None;
	ResourceUsage usage = ResourceUsage::Default;

	ClearValue optimizedClearValue;
};

struct TextureData
{
	const void* data = nullptr;

	// For 2D and 3D textures, row stride in bytes
	uint64_t stride = 0;

	// For 3D textures, depth slice stride in bytes
	uint64_t depthStride = 0;
};

class Texture : public RenderResource
{
public:
	static Ref<Texture> Create(const TextureDesc& desc, const std::vector<TextureData>& initData = {},
							   Flags<ResourceStateFlags> initState = ResourceStateFlags::Common);
	static Ref<Texture> Create(std::wstring filepath, bool loadAsCubemap = false);

	Ref<TextureView> CreateView(const TextureViewDesc& desc);

	virtual ResourceType GetResourceType() const override { return ResourceType::Texture; }
	virtual TextureDesc GetTextureDesc() const = 0;
};
}
