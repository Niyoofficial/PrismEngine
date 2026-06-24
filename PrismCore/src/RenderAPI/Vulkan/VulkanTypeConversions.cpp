#include "VulkanTypeConversions.h"

VkImageType Prism::Render::Vulkan::GetVkImageType(const ResourceDimension dimension)
{
	switch (dimension)
	{
	case ResourceDimension::Tex1D:
		return VK_IMAGE_TYPE_1D;
	case ResourceDimension::Tex2D:
		return VK_IMAGE_TYPE_2D;
	case ResourceDimension::Tex3D:
		return VK_IMAGE_TYPE_3D;
	case ResourceDimension::TexCube:
		return VK_IMAGE_TYPE_2D;
	default:
		return VK_IMAGE_TYPE_2D;
	}
}

VkImageViewType Prism::Render::Vulkan::GetVkImageViewType(const ResourceDimension dimension, const uint32_t arrayLayers)
{
	switch (dimension)
	{
	case ResourceDimension::Tex1D:
		return arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
	case ResourceDimension::Tex2D:
		return arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
	case ResourceDimension::Tex3D:
		return VK_IMAGE_VIEW_TYPE_3D;
	case ResourceDimension::TexCube:
		return arrayLayers > 6 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
	default:
		return VK_IMAGE_VIEW_TYPE_2D;
	}
}

VkImageUsageFlags Prism::Render::Vulkan::GetVkImageUsageFlags(const Flags<BindFlags> flags)
{
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if (flags & BindFlags::ShaderResource)
	{
		usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if (flags & BindFlags::RenderTarget)
	{
		usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (flags & BindFlags::DepthStencil)
	{
		usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (flags & BindFlags::UnorderedAccess)
	{
		usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	return usage;
}

VkImageAspectFlags Prism::Render::Vulkan::GetVkImageAspectFlags(const TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::D16_UNorm:
	case TextureFormat::D32_Float:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case TextureFormat::D24_UNorm_S8_UInt:
	case TextureFormat::D32_Float_S8X24_UInt:
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

VkFormat Prism::Render::Vulkan::GetVkFormat(const TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::RGBA32_Typeless:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case TextureFormat::RGBA32_Float:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case TextureFormat::RGBA32_UInt:
		return VK_FORMAT_R32G32B32A32_UINT;
	case TextureFormat::RGBA32_SInt:
		return VK_FORMAT_R32G32B32A32_SINT;
	case TextureFormat::RGB32_Typeless:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case TextureFormat::RGB32_Float:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case TextureFormat::RGB32_UInt:
		return VK_FORMAT_R32G32B32_UINT;
	case TextureFormat::RGB32_SInt:
		return VK_FORMAT_R32G32B32_SINT;
	case TextureFormat::RGBA16_Typeless:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case TextureFormat::RGBA16_Float:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case TextureFormat::RGBA16_UNorm:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case TextureFormat::RGBA16_UInt:
		return VK_FORMAT_R16G16B16A16_UINT;
	case TextureFormat::RGBA16_SNorm:
		return VK_FORMAT_R16G16B16A16_SNORM;
	case TextureFormat::RGBA16_SInt:
		return VK_FORMAT_R16G16B16A16_SINT;
	case TextureFormat::RG32_Typeless:
		return VK_FORMAT_R32G32_SFLOAT;
	case TextureFormat::RG32_Float:
		return VK_FORMAT_R32G32_SFLOAT;
	case TextureFormat::RG32_UInt:
		return VK_FORMAT_R32G32_UINT;
	case TextureFormat::RG32_SInt:
		return VK_FORMAT_R32G32_SINT;
	case TextureFormat::RGB10A2_Typeless:
		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case TextureFormat::RGB10A2_UNorm:
		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case TextureFormat::RGB10A2_UInt:
		return VK_FORMAT_A2B10G10R10_UINT_PACK32;
	case TextureFormat::R11G11B10_Float:
		return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	case TextureFormat::RGBA8_Typeless:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA8_UNorm:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA8_UNorm_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case TextureFormat::RGBA8_UInt:
		return VK_FORMAT_R8G8B8A8_UINT;
	case TextureFormat::RGBA8_SNorm:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case TextureFormat::RGBA8_SInt:
		return VK_FORMAT_R8G8B8A8_SINT;
	case TextureFormat::RG16_Typeless:
		return VK_FORMAT_R16G16_UNORM;
	case TextureFormat::RG16_Float:
		return VK_FORMAT_R16G16_SFLOAT;
	case TextureFormat::RG16_UNorm:
		return VK_FORMAT_R16G16_UNORM;
	case TextureFormat::RG16_UInt:
		return VK_FORMAT_R16G16_UINT;
	case TextureFormat::RG16_SNorm:
		return VK_FORMAT_R16G16_SNORM;
	case TextureFormat::RG16_SInt:
		return VK_FORMAT_R16G16_SINT;
	case TextureFormat::R32_Typeless:
		return VK_FORMAT_R32_SFLOAT;
	case TextureFormat::R32_Float:
		return VK_FORMAT_R32_SFLOAT;
	case TextureFormat::R32_UInt:
		return VK_FORMAT_R32_UINT;
	case TextureFormat::R32_SInt:
		return VK_FORMAT_R32_SINT;
	case TextureFormat::D32_Float:
		return VK_FORMAT_D32_SFLOAT;
	case TextureFormat::D16_UNorm:
		return VK_FORMAT_D16_UNORM;
	case TextureFormat::D24_UNorm_S8_UInt:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::D32_Float_S8X24_UInt:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	case TextureFormat::RG8_Typeless:
		return VK_FORMAT_R8G8_UNORM;
	case TextureFormat::RG8_UNorm:
		return VK_FORMAT_R8G8_UNORM;
	case TextureFormat::RG8_UInt:
		return VK_FORMAT_R8G8_UINT;
	case TextureFormat::RG8_SNorm:
		return VK_FORMAT_R8G8_SNORM;
	case TextureFormat::RG8_SInt:
		return VK_FORMAT_R8G8_SINT;
	case TextureFormat::R16_Typeless:
		return VK_FORMAT_R16_UNORM;
	case TextureFormat::R16_Float:
		return VK_FORMAT_R16_SFLOAT;
	case TextureFormat::R16_UNorm:
		return VK_FORMAT_R16_UNORM;
	case TextureFormat::R16_UInt:
		return VK_FORMAT_R16_UINT;
	case TextureFormat::R16_SNorm:
		return VK_FORMAT_R16_SNORM;
	case TextureFormat::R16_SInt:
		return VK_FORMAT_R16_SINT;
	case TextureFormat::R8_Typeless:
		return VK_FORMAT_R8_UNORM;
	case TextureFormat::R8_UNorm:
		return VK_FORMAT_R8_UNORM;
	case TextureFormat::R8_UInt:
		return VK_FORMAT_R8_UINT;
	case TextureFormat::R8_SNorm:
		return VK_FORMAT_R8_SNORM;
	case TextureFormat::R8_SInt:
		return VK_FORMAT_R8_SINT;
	case TextureFormat::A8_UNorm:
		return VK_FORMAT_R8_UNORM;
	case TextureFormat::BGRA8_UNorm:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case TextureFormat::BGRA8_UNorm_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case TextureFormat::BGRA8_Typeless:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case TextureFormat::BGRX8_UNorm:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case TextureFormat::BGRX8_UNorm_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case TextureFormat::BGRX8_Typeless:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case TextureFormat::BC1_Typeless:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case TextureFormat::BC1_UNorm:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case TextureFormat::BC1_UNorm_SRGB:
		return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
	case TextureFormat::BC2_Typeless:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case TextureFormat::BC2_UNorm:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case TextureFormat::BC2_UNorm_SRGB:
		return VK_FORMAT_BC2_SRGB_BLOCK;
	case TextureFormat::BC3_Typeless:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case TextureFormat::BC3_UNorm:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case TextureFormat::BC3_UNorm_SRGB:
		return VK_FORMAT_BC3_SRGB_BLOCK;
	case TextureFormat::BC4_Typeless:
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case TextureFormat::BC4_UNorm:
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case TextureFormat::BC4_SNorm:
		return VK_FORMAT_BC4_SNORM_BLOCK;
	case TextureFormat::BC5_Typeless:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case TextureFormat::BC5_UNorm:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case TextureFormat::BC5_SNorm:
		return VK_FORMAT_BC5_SNORM_BLOCK;
	case TextureFormat::BC6H_Typeless:
		return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case TextureFormat::BC6H_UF16:
		return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case TextureFormat::BC6H_SF16:
		return VK_FORMAT_BC6H_SFLOAT_BLOCK;
	case TextureFormat::BC7_Typeless:
		return VK_FORMAT_BC7_UNORM_BLOCK;
	case TextureFormat::BC7_UNorm:
		return VK_FORMAT_BC7_UNORM_BLOCK;
	case TextureFormat::BC7_UNorm_SRGB:
		return VK_FORMAT_BC7_SRGB_BLOCK;
	default:
		return VK_FORMAT_UNDEFINED;
	}
}
