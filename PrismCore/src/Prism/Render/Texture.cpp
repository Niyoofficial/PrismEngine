#include "pcpch.h"
#include "Texture.h"

#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderResourceCreation.h"
#include "Prism/Render/RenderUtils.h"

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

glm::int2 TextureDesc::GetSize() const
{
	return {GetWidth(), GetHeight()};
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
		auto context = RenderDevice::Get().AllocateContext(L"UpdateTextureWithInitData");
		context->UpdateTexture(texture, initData, 0);

		RenderDevice::Get().SubmitContext(context);
		RenderDevice::Get().GetRenderCommandQueue()->Flush();
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
	auto context = RenderDevice::Get().AllocateContext(L"UpdateTextureWithInitDataBuffer");
	context->CopyBufferRegion(texture, {0, 0, 0}, 0, uploadBuffer, 0);

	RenderDevice::Get().SubmitContext(context);
	RenderDevice::Get().GetRenderCommandQueue()->Flush();

	return texture;
}

Ref<Texture> Texture::CreateFromFile(std::wstring filepath, bool loadAsCubemap, bool waitForLoadFinish)
{
	return Private::CreateTexture(filepath, loadAsCubemap, waitForLoadFinish);
}

Ref<Texture> Texture::CreateFromMemory(std::wstring name, void* imageData, int64_t dataSize, bool loadAsCubemap, bool waitForLoadFinish)
{
	return Private::CreateTexture(name, imageData, dataSize, loadAsCubemap, waitForLoadFinish);
}

Ref<TextureView> Texture::CreateView(const TextureViewDesc& desc)
{
	return TextureView::Create(desc, this);
}

Ref<TextureView> Texture::CreateDefaultRTV()
{
	return TextureView::Create({.type = TextureViewType::RTV}, this);
}

Ref<TextureView> Texture::CreateDefaultSRV()
{
	return TextureView::Create({.type = TextureViewType::SRV}, this);
}

Ref<TextureView> Texture::CreateDefaultUAV()
{
	return TextureView::Create({.type = TextureViewType::UAV}, this);
}

void Texture::GenerateMipMaps(RenderContext* context)
{
	// From: https://github.com/microsoft/DirectX-Graphics-Samples/blob/909adc1d8142f403e2c1435aeae3f6e2ad4d020b/MiniEngine/Core/ColorBuffer.cpp#L163

	int32_t numMipMaps = GetTextureDesc().GetMipLevels() - 1;
	if (numMipMaps == 0)
		return;

	Ref<RenderContext> renderContext;
	if (context)
	{
		renderContext = context;
		renderContext->BeginEvent({1.f, 0.f, 1.f}, std::format(L"GenerateMipMaps_{}", GetTextureDesc().textureName));
	}
	else
	{
		renderContext = RenderDevice::Get().AllocateContext(std::format(L"GenerateMipMaps_{}", GetTextureDesc().textureName));
	}


	ShaderDesc mipMapShadersLinear[4];
	mipMapShadersLinear[0] = {
		.filepath = L"shaders/GenerateMipsLinear.hlsl",
		.entryName = L"main",
		.shaderType = ShaderType::CS
	};
	mipMapShadersLinear[1] = {
		.filepath = L"shaders/GenerateMipsLinearOddX.hlsl",
		.entryName = L"main",
		.shaderType = ShaderType::CS
	};
	mipMapShadersLinear[2] = {
		.filepath = L"shaders/GenerateMipsLinearOddY.hlsl",
		.entryName = L"main",
		.shaderType = ShaderType::CS
	};
	mipMapShadersLinear[3] = {
		.filepath = L"shaders/GenerateMipsLinearOddXY.hlsl",
		.entryName = L"main",
		.shaderType = ShaderType::CS
	};

	ShaderDesc mipMapShadersGamma[4];
	mipMapShadersGamma[0] = {
		.filepath = L"shaders/GenerateMipsGamma.hlsl",
		.entryName = L"main",
		.shaderType = ShaderType::CS
	};
	mipMapShadersGamma[1] = {
		.filepath = L"shaders/GenerateMipsGammaOddX.hlsl",
		.entryName = L"main",
		.shaderType = ShaderType::CS
	};
	mipMapShadersGamma[2] = {
		.filepath = L"shaders/GenerateMipsGammaOddY.hlsl",
		.entryName = L"main",
		.shaderType = ShaderType::CS
	};
	mipMapShadersGamma[3] = {
		.filepath = L"shaders/GenerateMipsGammaOddXY.hlsl",
		.entryName = L"main",
		.shaderType = ShaderType::CS
	};

	TextureDesc tempTexDesc = GetTextureDesc();
	tempTexDesc.textureName = GetTextureDesc().textureName + L"_TempTexMipMapGen";
	tempTexDesc.width = GetTextureDesc().GetWidth() >> 1;
	tempTexDesc.height = GetTextureDesc().GetHeight() >> 1;
	tempTexDesc.bindFlags |= BindFlags::UnorderedAccess;
	tempTexDesc.mipLevels = numMipMaps;

	auto tempTexture = Texture::Create(tempTexDesc);

	for (int32_t i = 0; i < GetTextureDesc().GetDepthOrArraySize(); ++i)
	{
		renderContext->BeginEvent({}, std::format(L"ArrayIndex_{}", i));

		for (int32_t topMip = 0; topMip < numMipMaps;)
		{
			int32_t srcWidth = GetTextureDesc().GetWidth() >> topMip;
			int32_t srcHeight = GetTextureDesc().GetHeight() >> topMip;
			int32_t dstWidth = srcWidth >> 1;
			int32_t dstHeight = srcHeight >> 1;

			// We can downsample up to four times, but if the ratio between levels is not
			// exactly 2:1, we have to shift our blend weights, which gets complicated or
			// expensive. Maybe we can update the code later to compute sample weights for
			// each successive downsample. We use std::countr_zero to count number of zeros
			// in the low bits. Zeros indicate we can divide by two without truncating.
			int32_t additionalMips = std::countr_zero((uint32_t)((dstWidth == 1 ? dstHeight : dstWidth) | (dstHeight == 1 ? dstWidth : dstHeight)));
			int32_t mipsToGenerate = 1 + (additionalMips > 3 ? 3 : additionalMips);
			if (topMip + mipsToGenerate > numMipMaps)
				mipsToGenerate = numMipMaps - topMip;

			renderContext->BeginEvent({}, std::format(L"GenerateLevels {}..{}", topMip + 1, mipsToGenerate + topMip + 1));

			// Determine if the first downsample is more than 2:1. This happens whenever
			// the source width or height is odd.
			int32_t shaderType = (srcWidth & 1) | (srcHeight & 1) << 1; // 0 - XY even, 1 - X odd, 2 - Y odd, 3 XY odd
			if (GetTextureDesc().format == TextureFormat::RGBA8_UNorm_SRGB)
				renderContext->SetPSO({ .cs = mipMapShadersGamma[shaderType] });
			else
				renderContext->SetPSO({ .cs = mipMapShadersLinear[shaderType] });

			// These are clamped to 1 after computing additional mips because clamped
			// dimensions should not limit us from downsampling multiple times.
			// (E.g. 16x1 -> 8x1 -> 4x1 -> 2x1 -> 1x1.)
			dstWidth = std::max(dstWidth, 1);
			dstHeight = std::max(dstHeight, 1);

			struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) MipMapGenerationInfo
			{
				uint32_t srcMipLevel;
				uint32_t numMipLevels;
				glm::float2 texelSize;
			};

			MipMapGenerationInfo info = {
				.srcMipLevel = (uint32_t)(topMip == 0 ? topMip : topMip - 1),
				.numMipLevels = (uint32_t)mipsToGenerate,
				.texelSize = {1.f / (float)dstWidth, 1.f / (float)dstHeight}
			};

			auto mipMapGenInfoBuffer = Buffer::Create(
				{
					.bufferName = L"MipMapGenerationInfo",
					.size = sizeof(MipMapGenerationInfo),
					.bindFlags = BindFlags::UniformBuffer,
					.usage = ResourceUsage::Default,
					.cpuAccess = CPUAccess::None
				},
				{
					.data = &info,
					.sizeInBytes = sizeof(info)
				});
			renderContext->SetBuffer(mipMapGenInfoBuffer->CreateDefaultUniformBufferView(), L"g_infoBuffer");

			if (topMip == 0)
			{
				auto srcView = TextureView::Create({
													   .type = TextureViewType::SRV,
													   .dimension = ResourceDimension::Tex2D,
													   .subresourceRange = {
														   .firstArraySlice = i,
														   .numArraySlices = 1
													   }
												   }, this);
				renderContext->SetTexture(srcView, L"g_srcMip");
			}
			else
			{
				auto srcView = TextureView::Create({
													   .type = TextureViewType::SRV,
													   .dimension = ResourceDimension::Tex2D,
													   .subresourceRange = {
														   .firstArraySlice = i,
														   .numArraySlices = 1
													   }
												   }, tempTexture);
				renderContext->SetTexture(srcView, L"g_srcMip");
			}

			for (int32_t j = 0; j < mipsToGenerate; ++j)
			{
				auto view = TextureView::Create({
													.type = TextureViewType::UAV,
													.dimension = ResourceDimension::Tex2D,
													.subresourceRange = {
														.firstMipLevel = topMip + j,
														.numMipLevels = 1,
														.firstArraySlice = i,
														.numArraySlices = 1
													}
												}, tempTexture);
				renderContext->SetTexture(view, std::wstring(L"g_outMip") + std::to_wstring(j + 1));
			}

			renderContext->Dispatch({dstWidth, dstHeight, 1});

			if (topMip != numMipMaps - 1)
			{
				renderContext->Barrier(TextureBarrier{
				   .texture = tempTexture,
				   .syncBefore = BarrierSync::ComputeShading,
				   .syncAfter = BarrierSync::ComputeShading,
				   .accessBefore = BarrierAccess::UnorderedAccess,
				   .accessAfter = BarrierAccess::UnorderedAccess,
				   .layoutBefore = BarrierLayout::UnorderedAccess,
				   .layoutAfter = BarrierLayout::UnorderedAccess,
				   .subresourceRange = {
					   .firstMipLevel = topMip,
					   .numMipLevels = numMipMaps - topMip,
					   .firstArraySlice = i,
					   .numArraySlices = 1,
				   }
			   });
			}

			topMip += mipsToGenerate;

			renderContext->EndEvent();
		}

		renderContext->EndEvent();
	}

	renderContext->Barrier(TextureBarrier{
		.texture = this,
		.syncBefore = BarrierSync::ComputeShading,
		.syncAfter = BarrierSync::Copy,
		.accessBefore = BarrierAccess::ShaderResource,
		.accessAfter = BarrierAccess::CopyDest,
		.layoutBefore = BarrierLayout::Common,
		.layoutAfter = BarrierLayout::CopyDest,
		/*.subresourceRange = {
			.firstMipLevel = 1,
			.numMipLevels = numMipMaps,
			.firstArraySlice = i,
			.numArraySlices = 1,
		}*/
	});

	context->BeginEvent({}, L"MipMapCopy");
	for (int32_t i = 0; i < GetTextureDesc().GetDepthOrArraySize(); ++i)
	{
		for (int32_t j = 0; j < numMipMaps; ++j)
		{
			renderContext->CopyTextureRegion(
				this, {}, GetSubresourceIndex(j + 1, GetTextureDesc().GetMipLevels(), i, GetTextureDesc().GetDepthOrArraySize()),
				tempTexture, GetSubresourceIndex(j, tempTexture->GetTextureDesc().GetMipLevels(), i, tempTexture->GetTextureDesc().GetDepthOrArraySize()));
		}
	}
	context->EndEvent();
	
	// If the renderContext was passed from the outside, we don't want to submit and flush it but give the control back to the caller
	if (!context)
	{
		RenderDevice::Get().SubmitContext(renderContext);
		RenderDevice::Get().GetRenderCommandQueue()->Flush();
	}
	else
	{
		renderContext->EndEvent();
	}
}
}
