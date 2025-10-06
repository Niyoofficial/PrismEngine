#pragma once
#include <variant>

#include "Prism/Render/RenderResource.h"
#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/TextureView.h"

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

	glm::int2 GetSize() const;
	int32_t GetWidth() const;
	int32_t GetHeight() const;
	int32_t GetArraySize() const;
	int32_t GetDepth() const;
	int32_t GetDepthOrArraySize() const;
	int32_t GetMipLevels() const;
	int32_t GetSubresourceCount() const;

public:
	std::wstring textureName;

	int32_t width = 0;
	int32_t height = 1;
	int32_t depthOrArraySize = 1;
	int32_t mipLevels = 1;
	ResourceDimension dimension = ResourceDimension::Tex2D;

	TextureFormat format = TextureFormat::Unknown;
	SampleDesc sampleDesc;

	Flags<BindFlags> bindFlags = BindFlags::None;
	ResourceUsage usage = ResourceUsage::Default;
	Flags<CPUAccess> cpuAccess = CPUAccess::None;

	std::optional<ClearValue> optimizedClearValue;
};

class Texture : public RenderResource
{
public:
	static Ref<Texture> Create(const TextureDesc& desc,
							   BarrierLayout initLayout = BarrierLayout::Common,
							   RawData initData = {});
	static Ref<Texture> Create(const TextureDesc& desc,
							   Buffer* initDataBuffer,
							   BarrierLayout initLayout = BarrierLayout::Common);
	static Ref<Texture> CreateFromFile(std::wstring filepath, bool loadAsCubemap = false, bool waitForLoadFinish = true);
	static Ref<Texture> CreateFromMemory(std::wstring name, void* imageData, int64_t dataSize, bool loadAsCubemap = false, bool waitForLoadFinish = true);

	Ref<TextureView> CreateView(const TextureViewDesc& desc);

	// Usage: pass the desired mip levels to texture desc during creation,
	// fill out the mip 0 of the texture, and then call this function to generate the rest of the mip maps
	// If you leave the context nullptr, the function will create a new context and wait for it to finish
	void GenerateMipMaps(class RenderContext* context = nullptr);

	virtual void WaitForLoadFinish() = 0;

	virtual ResourceType GetResourceType() const override { return ResourceType::Texture; }
	virtual TextureDesc GetTextureDesc() const = 0;
};
}
