#include "pcpch.h"
#include "Texture.h"

#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderResourceCreation.h"

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
		.mipLevels = inMipLevels,
		.dimension = ResourceDimension::Tex1D,
		.format = inFormat,
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
		.mipLevels = inMipLevels,
		.dimension = ResourceDimension::Tex2D,
		.format = inFormat,
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
		.mipLevels = inMipLevels,
		.dimension = ResourceDimension::Tex3D,
		.format = inFormat,
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
	return depthOrArraySize > 1 && !Is3D();
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

int32_t TextureDesc::GetMipLevels() const
{
	return mipLevels >= 1 ? mipLevels : 1;
}

int32_t TextureDesc::GetSubresourceCount() const
{
	return (Is3D() ? 1 : GetArraySize()) * GetMipLevels();
}

Ref<Texture> Texture::Create(const TextureDesc& desc, BarrierLayout initLayout, RawData initData)
{
	Ref<Texture> texture = Private::CreateTexture(desc, initLayout);

	if (initData.data && initData.sizeInBytes > 0)
	{
		// TODO: Add copy context
		auto context = RenderDevice::Get().AllocateContext();
		context->UpdateTexture(texture, initData, 0);

		RenderDevice::Get().SubmitContext(context);
		RenderDevice::Get().GetRenderQueue()->Flush();
	}

	return texture;
}

Ref<Texture> Texture::Create(const TextureDesc& desc, Buffer* initDataBuffer, BarrierLayout initLayout)
{
	PE_ASSERT(initDataBuffer);
	PE_ASSERT(initDataBuffer->GetBufferDesc().size >= RenderDevice::Get().GetTotalSizeInBytes(desc));

	Ref<Texture> texture = Private::CreateTexture(desc, initLayout);

	Ref<Buffer> uploadBuffer = initDataBuffer;
	if (initDataBuffer->GetBufferDesc().cpuAccess == CPUAccess::Read)
	{
		auto uploadBufferDesc = initDataBuffer->GetBufferDesc();
		uploadBufferDesc.bufferName = desc.textureName + L"_UploadBuffer";
		uploadBufferDesc.usage = ResourceUsage::Staging;
		uploadBufferDesc.cpuAccess = CPUAccess::Write;
		uploadBufferDesc.bindFlags = BindFlags::None;

		void* data = initDataBuffer->Map(CPUAccess::Read);
		uploadBuffer = Buffer::Create(uploadBufferDesc, {.data = data, .sizeInBytes = initDataBuffer->GetBufferDesc().size});
		initDataBuffer->Unmap();
	}

	// TODO: Add copy context
	auto context = RenderDevice::Get().AllocateContext();
	context->CopyBufferRegion(texture, {0, 0, 0}, 0, uploadBuffer, 0);

	RenderDevice::Get().SubmitContext(context);
	RenderDevice::Get().GetRenderQueue()->Flush();

	return texture;
}

Ref<Texture> Texture::Create(std::wstring filepath, bool loadAsCubemap, bool waitForLoadFinish)
{
	return Private::CreateTexture(filepath, loadAsCubemap, waitForLoadFinish);
}

Ref<TextureView> Texture::CreateView(const TextureViewDesc& desc)
{
	return TextureView::Create(desc, this);
}
}
