#include "pcpch.h"
#include "Texture.h"

#include "Prism-Core/Render/RenderResourceCreation.h"

namespace Prism::Render
{
TextureDesc TextureDesc::CreateTex1D(std::wstring inTextureName,
									 int32_t inWidth,
									 TextureFormat inFormat,
									 Flags<BindFlags> inBindFlags, ResourceUsage inUsage,
									 int32_t inMipLevels, SampleDesc inSampleDesc)
{
	return {
		.textureName = std::move(inTextureName),
		.width = inWidth,
		.height = 1,
		.depthOrArraySize = 1,
		.dimension = ResourceDimension::Tex1D,
		.format = inFormat,
		.mipLevels = inMipLevels,
		.sampleDesc = inSampleDesc,
		.bindFlags = inBindFlags,
		.usage = inUsage
	};
}

TextureDesc TextureDesc::CreateTex2D(std::wstring inTextureName,
									 int32_t inWidth, int32_t inHeight,
									 TextureFormat inFormat,
									 Flags<BindFlags> inBindFlags, ResourceUsage inUsage,
									 int32_t inMipLevels, SampleDesc inSampleDesc)
{
	return {
		.textureName = std::move(inTextureName),
		.width = inWidth,
		.height = inHeight,
		.depthOrArraySize = 1,
		.dimension = ResourceDimension::Tex2D,
		.format = inFormat,
		.mipLevels = inMipLevels,
		.sampleDesc = inSampleDesc,
		.bindFlags = inBindFlags,
		.usage = inUsage
	};
}

TextureDesc TextureDesc::CreateTex3D(std::wstring inTextureName,
									 int32_t inWidth, int32_t inHeight, int32_t inDepth,
									 TextureFormat inFormat,
									 Flags<BindFlags> inBindFlags, ResourceUsage inUsage,
									 int32_t inMipLevels, SampleDesc inSampleDesc)
{
	return {
		.textureName = std::move(inTextureName),
		.width = inWidth,
		.height = inHeight,
		.depthOrArraySize = inDepth,
		.dimension = ResourceDimension::Tex3D,
		.format = inFormat,
		.mipLevels = inMipLevels,
		.sampleDesc = inSampleDesc,
		.bindFlags = inBindFlags,
		.usage = inUsage
	};
}

bool TextureDesc::Is1D() const
{
	return dimension == ResourceDimension::Tex1D;
}

bool TextureDesc::Is2D() const
{
	return dimension == ResourceDimension::Tex2D;
}

bool TextureDesc::Is3D() const
{
	return dimension == ResourceDimension::Tex3D;
}

bool TextureDesc::IsArray() const
{
	return depthOrArraySize > 1;
}

bool TextureDesc::IsCube() const
{
	return
		dimension == ResourceDimension::TexCube &&
		depthOrArraySize >= 6 &&
		depthOrArraySize % 6 == 0;
}

int32_t TextureDesc::GetWidth() const
{
	return width;
}

int32_t TextureDesc::GetHeight() const
{
	return Is1D() ? 1 : height;
}

int32_t TextureDesc::GetArraySize() const
{
	return IsArray() ? depthOrArraySize : 1;
}

int32_t TextureDesc::GetDepth() const
{
	return Is3D() ? depthOrArraySize : 1;
}

int32_t TextureDesc::GetDepthOrArraySize() const
{
	return depthOrArraySize;
}

Texture* Texture::Create(const TextureDesc& desc, const std::vector<TextureInitData>& initData)
{
	return Private::CreateTexture(desc, initData);
}

TextureView* Texture::CreateView(const TextureViewDesc& desc)
{
	return TextureView::Create(desc, this);
}
}
