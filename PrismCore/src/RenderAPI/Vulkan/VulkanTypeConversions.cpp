#include "VulkanTypeConversions.h"

#include "Prism/Render/VertexBufferCache.h"

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
		PE_ASSERT_NO_ENTRY("Unknown ResourceDimension");
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
		PE_ASSERT_NO_ENTRY("Unknown ResourceDimension");
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
		PE_ASSERT_NO_ENTRY("Unknown TextureFormat");
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
		PE_ASSERT_NO_ENTRY("Unknown TextureFormat");
		return VK_FORMAT_UNDEFINED;
	}
}

VkBufferUsageFlags Prism::Render::Vulkan::GetVkBufferUsageFlags(Flags<BindFlags> flags)
{
	VkBufferUsageFlags bufferUsageFlags{};

	if (flags.HasAnyFlags(BindFlags::VertexBuffer))
	{
		bufferUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	if (flags.HasAnyFlags(BindFlags::IndexBuffer))
	{
		bufferUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}

	if (flags.HasAnyFlags(BindFlags::UniformBuffer))
	{
		bufferUsageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	if (flags.HasAnyFlags(BindFlags::ShaderResource))
	{
		bufferUsageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferUsageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	}

	if (flags.HasAnyFlags(BindFlags::UnorderedAccess))
	{
		bufferUsageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}

	if (flags.HasAnyFlags(BindFlags::IndirectDrawArgs))
	{
		bufferUsageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}

	bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	return bufferUsageFlags;
}

VmaAllocationCreateInfo Prism::Render::Vulkan::GetVmaAllocationCreateInfo(const ResourceUsage usage,
                                                                          const Flags<CPUAccess> cpuAccess)
{
	VmaAllocationCreateInfo allocationCreateInfo{};

	switch (usage)
	{
	case ResourceUsage::Default:
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		break;
	case ResourceUsage::Dynamic:
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		break;
	case ResourceUsage::Staging:
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		if (cpuAccess.HasAnyFlags(CPUAccess::Read))
		{
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}
		else if (cpuAccess.HasAnyFlags(CPUAccess::Write))
		{
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
	}

	return allocationCreateInfo;
}

VkImageLayout Prism::Render::Vulkan::GetVkImageLayout(const BarrierLayout layout)
{
	switch (layout)
	{
	case BarrierLayout::Undefined:
		return VK_IMAGE_LAYOUT_UNDEFINED;
	case BarrierLayout::Common:
		return VK_IMAGE_LAYOUT_GENERAL;
	case BarrierLayout::Present:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	case BarrierLayout::GenericRead:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case BarrierLayout::RenderTarget:
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case BarrierLayout::UnorderedAccess:
		return VK_IMAGE_LAYOUT_GENERAL;
	case BarrierLayout::DepthStencilWrite:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case BarrierLayout::DepthStencilRead:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case BarrierLayout::ShaderResource:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case BarrierLayout::CopySource:
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case BarrierLayout::CopyDest:
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	default:
		PE_ASSERT_NO_ENTRY("Unknown BarrierLayout");
		return VK_IMAGE_LAYOUT_GENERAL;
	}
}

VkShaderStageFlags Prism::Render::Vulkan::GetVkShaderStageFlags(const ShaderType type)
{
	switch (type)
	{
	case ShaderType::VS:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderType::PS:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderType::GS:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderType::HS:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ShaderType::DS:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case ShaderType::CS:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	default:
		PE_ASSERT_NO_ENTRY("Unknown ShaderType");
		return VK_SHADER_STAGE_ALL;
	}
}

VkFormat Prism::Render::Vulkan::GetVkFormat(const VertexAttribute attribute)
{
	switch (attribute)
	{
	case VertexAttribute::Position:
	case VertexAttribute::Normal:
	case VertexAttribute::Tangent:
	case VertexAttribute::Bitangent:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VertexAttribute::TexCoord:
		return VK_FORMAT_R32G32_SFLOAT;
	case VertexAttribute::Color:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		PE_ASSERT_NO_ENTRY("Unknown VertexAttribute");
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
}

VkPrimitiveTopology Prism::Render::Vulkan::GetVkPrimitiveTopology(const TopologyType topologyType)
{
	switch (topologyType)
	{
	case TopologyType::PointList:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case TopologyType::LineList:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case TopologyType::LineStrip:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case TopologyType::TriangleList:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case TopologyType::TriangleStrip:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case TopologyType::LineListAdj:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
	case TopologyType::LineStripAdj:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
	case TopologyType::TriangleListAdj:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
	case TopologyType::TriangleStripAdj:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
	case TopologyType::ControlPointPatchlist1:
	case TopologyType::ControlPointPatchlist2:
	case TopologyType::ControlPointPatchlist3:
	case TopologyType::ControlPointPatchlist4:
	case TopologyType::ControlPointPatchlist5:
	case TopologyType::ControlPointPatchlist6:
	case TopologyType::ControlPointPatchlist7:
	case TopologyType::ControlPointPatchlist8:
	case TopologyType::ControlPointPatchlist9:
	case TopologyType::ControlPointPatchlist10:
	case TopologyType::ControlPointPatchlist11:
	case TopologyType::ControlPointPatchlist12:
	case TopologyType::ControlPointPatchlist13:
	case TopologyType::ControlPointPatchlist14:
	case TopologyType::ControlPointPatchlist15:
	case TopologyType::ControlPointPatchlist16:
	case TopologyType::ControlPointPatchlist17:
	case TopologyType::ControlPointPatchlist18:
	case TopologyType::ControlPointPatchlist19:
	case TopologyType::ControlPointPatchlist20:
	case TopologyType::ControlPointPatchlist21:
	case TopologyType::ControlPointPatchlist22:
	case TopologyType::ControlPointPatchlist23:
	case TopologyType::ControlPointPatchlist24:
	case TopologyType::ControlPointPatchlist25:
	case TopologyType::ControlPointPatchlist26:
	case TopologyType::ControlPointPatchlist27:
	case TopologyType::ControlPointPatchlist28:
	case TopologyType::ControlPointPatchlist29:
	case TopologyType::ControlPointPatchlist30:
	case TopologyType::ControlPointPatchlist31:
	case TopologyType::ControlPointPatchlist32:
		return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	case TopologyType::Undefined:
	default:
		PE_ASSERT_NO_ENTRY("Unknown TopologyType");
		return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}
}

VkPolygonMode Prism::Render::Vulkan::GetVkPolygonMode(const FillMode fillMode)
{
	switch (fillMode)
	{
	case FillMode::Wireframe:
		return VK_POLYGON_MODE_LINE;
	case FillMode::Solid:
		return VK_POLYGON_MODE_FILL;
	default:
		PE_ASSERT_NO_ENTRY("Unknown FillMode");
		return VK_POLYGON_MODE_FILL;
	}
}

VkCullModeFlags Prism::Render::Vulkan::GetVkCullModeFlags(const CullMode cullMode)
{
	switch (cullMode)
	{
	case CullMode::None:
		return VK_CULL_MODE_NONE;
	case CullMode::Front:
		return VK_CULL_MODE_FRONT_BIT;
	case CullMode::Back:
		return VK_CULL_MODE_BACK_BIT;
	default:
		PE_ASSERT_NO_ENTRY("Unknown CullMode");
		return VK_CULL_MODE_BACK_BIT;
	}
}

VkSampleCountFlagBits Prism::Render::Vulkan::GetVkSampleCountFlagBits(const uint32_t sampleCount)
{
	switch (sampleCount)
	{
	case 1:
		return VK_SAMPLE_COUNT_1_BIT;
	case 2:
		return VK_SAMPLE_COUNT_2_BIT;
	case 4:
		return VK_SAMPLE_COUNT_4_BIT;
	case 8:
		return VK_SAMPLE_COUNT_8_BIT;
	case 16:
		return VK_SAMPLE_COUNT_16_BIT;
	case 32:
		return VK_SAMPLE_COUNT_32_BIT;
	case 64:
		return VK_SAMPLE_COUNT_64_BIT;
	default:
		PE_ASSERT_NO_ENTRY("Unknown sampleCount");
		return VK_SAMPLE_COUNT_1_BIT;
	}
}

VkCompareOp Prism::Render::Vulkan::GetVkCompareOp(const ComparisionFunction function)
{
	switch (function)
	{
	case ComparisionFunction::Never:
		return VK_COMPARE_OP_NEVER;
	case ComparisionFunction::Less:
		return VK_COMPARE_OP_LESS;
	case ComparisionFunction::Equal:
		return VK_COMPARE_OP_EQUAL;
	case ComparisionFunction::LessEqual:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case ComparisionFunction::Greater:
		return VK_COMPARE_OP_GREATER;
	case ComparisionFunction::NotEqual:
		return VK_COMPARE_OP_NOT_EQUAL;
	case ComparisionFunction::GreaterEqual:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	default:
		PE_ASSERT_NO_ENTRY("Unknown ComparisionFunction");
		return VK_COMPARE_OP_ALWAYS;
	}
}

VkStencilOp Prism::Render::Vulkan::GetVkStencilOp(const StencilOperation operation)
{
	switch (operation)
	{
	case StencilOperation::Keep:
		return VK_STENCIL_OP_KEEP;
	case StencilOperation::Zero:
		return VK_STENCIL_OP_ZERO;
	case StencilOperation::Replace:
		return VK_STENCIL_OP_REPLACE;
	case StencilOperation::IncrSat:
		return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case StencilOperation::DecrSat:
		return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case StencilOperation::Invert:
		return VK_STENCIL_OP_INVERT;
	case StencilOperation::IncrWrap:
		return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case StencilOperation::DecrWrap:
		return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	case StencilOperation::Undefined:
		return VK_STENCIL_OP_KEEP;
	default:
		PE_ASSERT_NO_ENTRY("Unknown StencilOperation");
		return VK_STENCIL_OP_KEEP;
	}
}

VkStencilOpState Prism::Render::Vulkan::GetVkStencilOpState(const DepthStencilOperationDesc& desc, const uint8_t readMask,
                                                            const uint8_t writeMask, const uint32_t reference)
{
	return VkStencilOpState{
	    .failOp = GetVkStencilOp(desc.stencilFail),
	    .passOp = GetVkStencilOp(desc.stencilPass),
	    .depthFailOp = GetVkStencilOp(desc.stencilDepthFail),
	    .compareOp = GetVkCompareOp(desc.stencilFunction),
	    .compareMask = readMask,
	    .writeMask = writeMask,
	    .reference = reference,
	};
}

VkBlendFactor Prism::Render::Vulkan::GetVkBlendFactor(const BlendFactor factor)
{
	switch (factor)
	{
	case BlendFactor::Zero:
		return VK_BLEND_FACTOR_ZERO;
	case BlendFactor::One:
		return VK_BLEND_FACTOR_ONE;
	case BlendFactor::SrcColor:
		return VK_BLEND_FACTOR_SRC_COLOR;
	case BlendFactor::InvSrcColor:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case BlendFactor::SrcAlpha:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case BlendFactor::InvSrcAlpha:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::DestAlpha:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case BlendFactor::InvDestAlpha:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case BlendFactor::DestColor:
		return VK_BLEND_FACTOR_DST_COLOR;
	case BlendFactor::InvDestColor:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case BlendFactor::SrcAlphaSat:
		return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
	case BlendFactor::ConstantBlendFactor:
		return VK_BLEND_FACTOR_CONSTANT_COLOR;
	case BlendFactor::InvConstantBlendFactor:
		return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	case BlendFactor::Src1Color:
		return VK_BLEND_FACTOR_SRC1_COLOR;
	case BlendFactor::InvSrc1Color:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
	case BlendFactor::Src1Alpha:
		return VK_BLEND_FACTOR_SRC1_ALPHA;
	case BlendFactor::InvSrc1Alpha:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	default:
		PE_ASSERT_NO_ENTRY("Unknown BlendFactor");
		return VK_BLEND_FACTOR_ONE;
	}
}

VkBlendOp Prism::Render::Vulkan::GetVkBlendOp(const BlendOperation operation)
{
	switch (operation)
	{
	case BlendOperation::Add:
		return VK_BLEND_OP_ADD;
	case BlendOperation::Subtract:
		return VK_BLEND_OP_SUBTRACT;
	case BlendOperation::RevSubtract:
		return VK_BLEND_OP_REVERSE_SUBTRACT;
	case BlendOperation::Min:
		return VK_BLEND_OP_MIN;
	case BlendOperation::Max:
		return VK_BLEND_OP_MAX;
	default:
		PE_ASSERT_NO_ENTRY("Unknown BlendOperation");
		return VK_BLEND_OP_ADD;
	}
}

VkColorComponentFlags Prism::Render::Vulkan::GetVkColorComponentFlags(ColorMask mask)
{
	VkColorComponentFlags flags = 0;

	if ((mask & ColorMask::Red) == ColorMask::Red)
	{
		flags |= VK_COLOR_COMPONENT_R_BIT;
	}

	if ((mask & ColorMask::Green) == ColorMask::Green)
	{
		flags |= VK_COLOR_COMPONENT_G_BIT;
	}

	if ((mask & ColorMask::Blue) == ColorMask::Blue)
	{
		flags |= VK_COLOR_COMPONENT_B_BIT;
	}

	if ((mask & ColorMask::Alpha) == ColorMask::Alpha)
	{
		flags |= VK_COLOR_COMPONENT_A_BIT;
	}

	return flags;
}
