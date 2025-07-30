#include "pcpch.h"
#include "D3D12TypeConversions.h"

#include "glm/ext/scalar_integer.hpp"
#include "RenderAPI/D3D12/D3D12Buffer.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12RootSignature.h"
#include "RenderAPI/D3D12/D3D12Texture.h"

namespace Prism::Render::D3D12
{
using namespace Render;

DXGI_FORMAT GetDXGIFormat(TextureFormat format)
{
	static DXGI_FORMAT s_dxgiFormatMap[(int)TextureFormat::NumFormats] = { DXGI_FORMAT_UNKNOWN };
	static bool s_formatMapInitialized = false;

	if (!s_formatMapInitialized)
	{
		s_dxgiFormatMap[(int)TextureFormat::Unknown]                   = DXGI_FORMAT_UNKNOWN;

		s_dxgiFormatMap[(int)TextureFormat::RGBA32_Typeless]           = DXGI_FORMAT_R32G32B32A32_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::RGBA32_Float]              = DXGI_FORMAT_R32G32B32A32_FLOAT;
		s_dxgiFormatMap[(int)TextureFormat::RGBA32_UInt]               = DXGI_FORMAT_R32G32B32A32_UINT;
		s_dxgiFormatMap[(int)TextureFormat::RGBA32_SInt]               = DXGI_FORMAT_R32G32B32A32_SINT;

		s_dxgiFormatMap[(int)TextureFormat::RGB32_Typeless]            = DXGI_FORMAT_R32G32B32_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::RGB32_Float]               = DXGI_FORMAT_R32G32B32_FLOAT;
		s_dxgiFormatMap[(int)TextureFormat::RGB32_UInt]                = DXGI_FORMAT_R32G32B32_UINT;
		s_dxgiFormatMap[(int)TextureFormat::RGB32_SInt]                = DXGI_FORMAT_R32G32B32_SINT;

		s_dxgiFormatMap[(int)TextureFormat::RGBA16_Typeless]           = DXGI_FORMAT_R16G16B16A16_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::RGBA16_Float]              = DXGI_FORMAT_R16G16B16A16_FLOAT;
		s_dxgiFormatMap[(int)TextureFormat::RGBA16_UNorm]              = DXGI_FORMAT_R16G16B16A16_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::RGBA16_UInt]               = DXGI_FORMAT_R16G16B16A16_UINT;
		s_dxgiFormatMap[(int)TextureFormat::RGBA16_SNorm]              = DXGI_FORMAT_R16G16B16A16_SNORM;
		s_dxgiFormatMap[(int)TextureFormat::RGBA16_SInt]               = DXGI_FORMAT_R16G16B16A16_SINT;

		s_dxgiFormatMap[(int)TextureFormat::RG32_Typeless]             = DXGI_FORMAT_R32G32_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::RG32_Float]                = DXGI_FORMAT_R32G32_FLOAT;
		s_dxgiFormatMap[(int)TextureFormat::RG32_UInt]                 = DXGI_FORMAT_R32G32_UINT;
		s_dxgiFormatMap[(int)TextureFormat::RG32_SInt]                 = DXGI_FORMAT_R32G32_SINT;

		s_dxgiFormatMap[(int)TextureFormat::R32G8X24_Typeless]         = DXGI_FORMAT_R32G8X24_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::D32_Float_S8X24_UInt]      = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		s_dxgiFormatMap[(int)TextureFormat::R32_Float_X8X24_Typeless]  = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::X32_Typeless_G8X24_UInt]   = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

		s_dxgiFormatMap[(int)TextureFormat::RGB10A2_Typeless]          = DXGI_FORMAT_R10G10B10A2_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::RGB10A2_UNorm]             = DXGI_FORMAT_R10G10B10A2_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::RGB10A2_UInt]              = DXGI_FORMAT_R10G10B10A2_UINT;

		s_dxgiFormatMap[(int)TextureFormat::R11G11B10_Float]           = DXGI_FORMAT_R11G11B10_FLOAT;

		s_dxgiFormatMap[(int)TextureFormat::RGBA8_Typeless]            = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::RGBA8_UNorm]               = DXGI_FORMAT_R8G8B8A8_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::RGBA8_UNorm_SRGB]          = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		s_dxgiFormatMap[(int)TextureFormat::RGBA8_UInt]                = DXGI_FORMAT_R8G8B8A8_UINT;
		s_dxgiFormatMap[(int)TextureFormat::RGBA8_SNorm]               = DXGI_FORMAT_R8G8B8A8_SNORM;
		s_dxgiFormatMap[(int)TextureFormat::RGBA8_SInt]                = DXGI_FORMAT_R8G8B8A8_SINT;

		s_dxgiFormatMap[(int)TextureFormat::RG16_Typeless]             = DXGI_FORMAT_R16G16_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::RG16_Float]                = DXGI_FORMAT_R16G16_FLOAT;
		s_dxgiFormatMap[(int)TextureFormat::RG16_UNorm]                = DXGI_FORMAT_R16G16_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::RG16_UInt]                 = DXGI_FORMAT_R16G16_UINT;
		s_dxgiFormatMap[(int)TextureFormat::RG16_SNorm]                = DXGI_FORMAT_R16G16_SNORM;
		s_dxgiFormatMap[(int)TextureFormat::RG16_SInt]                 = DXGI_FORMAT_R16G16_SINT;

		s_dxgiFormatMap[(int)TextureFormat::R32_Typeless]              = DXGI_FORMAT_R32_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::D32_Float]                 = DXGI_FORMAT_D32_FLOAT;
		s_dxgiFormatMap[(int)TextureFormat::R32_Float]                 = DXGI_FORMAT_R32_FLOAT;
		s_dxgiFormatMap[(int)TextureFormat::R32_UInt]                  = DXGI_FORMAT_R32_UINT;
		s_dxgiFormatMap[(int)TextureFormat::R32_SInt]                  = DXGI_FORMAT_R32_SINT;

		s_dxgiFormatMap[(int)TextureFormat::R24G8_Typeless]            = DXGI_FORMAT_R24G8_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::D24_UNorm_S8_UInt]         = DXGI_FORMAT_D24_UNORM_S8_UINT;
		s_dxgiFormatMap[(int)TextureFormat::R24_UNorm_X8_Typeless]     = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::X24_Typeless_G8_UInt]      = DXGI_FORMAT_X24_TYPELESS_G8_UINT;

		s_dxgiFormatMap[(int)TextureFormat::RG8_Typeless]              = DXGI_FORMAT_R8G8_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::RG8_UNorm]                 = DXGI_FORMAT_R8G8_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::RG8_UInt]                  = DXGI_FORMAT_R8G8_UINT;
		s_dxgiFormatMap[(int)TextureFormat::RG8_SNorm]                 = DXGI_FORMAT_R8G8_SNORM;
		s_dxgiFormatMap[(int)TextureFormat::RG8_SInt]                  = DXGI_FORMAT_R8G8_SINT;

		s_dxgiFormatMap[(int)TextureFormat::R16_Typeless]              = DXGI_FORMAT_R16_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::R16_Float]                 = DXGI_FORMAT_R16_FLOAT;
		s_dxgiFormatMap[(int)TextureFormat::D16_UNorm]                 = DXGI_FORMAT_D16_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::R16_UNorm]                 = DXGI_FORMAT_R16_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::R16_UInt]                  = DXGI_FORMAT_R16_UINT;
		s_dxgiFormatMap[(int)TextureFormat::R16_SNorm]                 = DXGI_FORMAT_R16_SNORM;
		s_dxgiFormatMap[(int)TextureFormat::R16_SInt]                  = DXGI_FORMAT_R16_SINT;

		s_dxgiFormatMap[(int)TextureFormat::R8_Typeless]               = DXGI_FORMAT_R8_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::R8_UNorm]                  = DXGI_FORMAT_R8_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::R8_UInt]                   = DXGI_FORMAT_R8_UINT;
		s_dxgiFormatMap[(int)TextureFormat::R8_SNorm]                  = DXGI_FORMAT_R8_SNORM;
		s_dxgiFormatMap[(int)TextureFormat::R8_SInt]                   = DXGI_FORMAT_R8_SINT;
		s_dxgiFormatMap[(int)TextureFormat::A8_UNorm]                  = DXGI_FORMAT_A8_UNORM;

		s_dxgiFormatMap[(int)TextureFormat::R1_UNorm]                  = DXGI_FORMAT_R1_UNORM ;
		s_dxgiFormatMap[(int)TextureFormat::RGB9E5_SHAREDEXP]          = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		s_dxgiFormatMap[(int)TextureFormat::RG8_B8G8_UNorm]            = DXGI_FORMAT_R8G8_B8G8_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::G8R8_G8B8_UNorm]           = DXGI_FORMAT_G8R8_G8B8_UNORM;

		s_dxgiFormatMap[(int)TextureFormat::BC1_Typeless]              = DXGI_FORMAT_BC1_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::BC1_UNorm]                 = DXGI_FORMAT_BC1_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::BC1_UNorm_SRGB]            = DXGI_FORMAT_BC1_UNORM_SRGB;
		s_dxgiFormatMap[(int)TextureFormat::BC2_Typeless]              = DXGI_FORMAT_BC2_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::BC2_UNorm]                 = DXGI_FORMAT_BC2_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::BC2_UNorm_SRGB]            = DXGI_FORMAT_BC2_UNORM_SRGB;
		s_dxgiFormatMap[(int)TextureFormat::BC3_Typeless]              = DXGI_FORMAT_BC3_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::BC3_UNorm]                 = DXGI_FORMAT_BC3_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::BC3_UNorm_SRGB]            = DXGI_FORMAT_BC3_UNORM_SRGB;
		s_dxgiFormatMap[(int)TextureFormat::BC4_Typeless]              = DXGI_FORMAT_BC4_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::BC4_UNorm]                 = DXGI_FORMAT_BC4_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::BC4_SNorm]                 = DXGI_FORMAT_BC4_SNORM;
		s_dxgiFormatMap[(int)TextureFormat::BC5_Typeless]              = DXGI_FORMAT_BC5_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::BC5_UNorm]                 = DXGI_FORMAT_BC5_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::BC5_SNorm]                 = DXGI_FORMAT_BC5_SNORM;

		s_dxgiFormatMap[(int)TextureFormat::B5G6R5_UNorm]              = DXGI_FORMAT_B5G6R5_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::B5G5R5A1_UNorm]            = DXGI_FORMAT_B5G5R5A1_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::BGRA8_UNorm]               = DXGI_FORMAT_B8G8R8A8_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::BGRX8_UNorm]               = DXGI_FORMAT_B8G8R8X8_UNORM;

		s_dxgiFormatMap[(int)TextureFormat::R10G10B10_XR_BIAS_A2_UNorm]= DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

		s_dxgiFormatMap[(int)TextureFormat::BGRA8_Typeless]            = DXGI_FORMAT_B8G8R8A8_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::BGRA8_UNorm_SRGB]          = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		s_dxgiFormatMap[(int)TextureFormat::BGRX8_Typeless]            = DXGI_FORMAT_B8G8R8X8_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::BGRX8_UNorm_SRGB]          = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

		s_dxgiFormatMap[(int)TextureFormat::BC6H_Typeless]             = DXGI_FORMAT_BC6H_TYPELESS;
		s_dxgiFormatMap[(int)TextureFormat::BC6H_UF16]                 = DXGI_FORMAT_BC6H_UF16;
		s_dxgiFormatMap[(int)TextureFormat::BC6H_SF16]                 = DXGI_FORMAT_BC6H_SF16;
		s_dxgiFormatMap[(int)TextureFormat::BC7_Typeless]              = DXGI_FORMAT_BC7_TYPELESS ;
		s_dxgiFormatMap[(int)TextureFormat::BC7_UNorm]                 = DXGI_FORMAT_BC7_UNORM;
		s_dxgiFormatMap[(int)TextureFormat::BC7_UNorm_SRGB]            = DXGI_FORMAT_BC7_UNORM_SRGB;

		s_formatMapInitialized = true;
	}

	return s_dxgiFormatMap[(int)format];
}

DXGI_SAMPLE_DESC GetDXGISampleDesc(SampleDesc sampleDesc)
{
	return {
		.Count = (UINT)sampleDesc.count,
		.Quality = (UINT)sampleDesc.quality
	};
}

DXGI_RATIONAL GetDXGIRational(int32_t numerator, int32_t denominator)
{
	return {
		.Numerator = (UINT)numerator,
		.Denominator = (UINT)denominator
	};
}

D3D12GraphicsPipelineStateDesc GetD3D12PipelineStateDesc(const GraphicsPipelineStateDesc& desc, const D3D12ShaderCompilerOutput& vs, const D3D12ShaderCompilerOutput& ps, std::vector<Ref<TextureView>> rtvs, TextureView* dsv)
{
	PE_ASSERT(
		desc.primitiveTopologyType != Render::TopologyType::LineStrip &&
		desc.primitiveTopologyType != Render::TopologyType::LineStripAdj &&
		desc.primitiveTopologyType != Render::TopologyType::TriangleStrip &&
		desc.primitiveTopologyType != Render::TopologyType::TriangleStripAdj,
		"We don't support strips for now, check IBStripCutValue below");

	std::array<D3D12_RENDER_TARGET_BLEND_DESC, 8> renderTargetBlends;
	for (int32_t i = 0; i < (int32_t)renderTargetBlends.size(); ++i)
		renderTargetBlends[i] = GetD3D12RenderTargetBlendDesc(desc.blendState.renderTargetBlendDescs[i]);

	D3D12InputLayout inputLayout = GetD3D12InputLayoutFromVertexShader(vs);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12Desc = {
		.pRootSignature = D3D12RenderDevice::Get().GetGlobalRootSignature(PipelineStateType::Graphics)->GetD3D12RootSignature(),
		.VS = GetD3D12ShaderBytecode(vs.bytecode.Get()),
		.PS = GetD3D12ShaderBytecode(ps.bytecode.Get()),
		.DS = {}, // We don't support this shader for now
		.HS = {}, // We don't support this shader for now
		.GS = {}, // We don't support this shader for now
		.BlendState = {
			.AlphaToCoverageEnable = desc.blendState.alphaToCoverageEnable,
			.IndependentBlendEnable = desc.blendState.independentBlendEnable,
			.RenderTarget = {}
		},
		.SampleMask = desc.sampleMask,
		.RasterizerState = {
			.FillMode = GetD3D12FillMode(desc.rasterizerState.fillMode),
			.CullMode = GetD3D12CullMode(desc.rasterizerState.cullMode),
			.FrontCounterClockwise = desc.rasterizerState.frontCounterClockwise,
			.DepthBias = desc.rasterizerState.depthBias,
			.DepthBiasClamp = desc.rasterizerState.depthBiasClamp,
			.SlopeScaledDepthBias = desc.rasterizerState.slopeScaledDepthBias,
			.DepthClipEnable = desc.rasterizerState.depthClipEnable,
			.MultisampleEnable = desc.rasterizerState.antialiasedLineEnable, // Multisample enabled only when antialiased line is enabled
			.AntialiasedLineEnable = desc.rasterizerState.antialiasedLineEnable,
			.ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		},
		.DepthStencilState = {
			.DepthEnable = desc.depthStencilState.depthEnable,
			.DepthWriteMask = desc.depthStencilState.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
			.DepthFunc = GetD3D12ComparisionFunc(desc.depthStencilState.depthFunc),
			.StencilEnable = desc.depthStencilState.stencilEnable,
			.StencilReadMask = desc.depthStencilState.stencilReadMask,
			.StencilWriteMask = desc.depthStencilState.stencilWriteMask,
			.FrontFace = GetD3D12DepthStencilOpDesc(desc.depthStencilState.frontFace),
			.BackFace = GetD3D12DepthStencilOpDesc(desc.depthStencilState.backFace)
		},
		.InputLayout = {
			.pInputElementDescs = inputLayout.inputLayout.data(),
			.NumElements = (UINT)inputLayout.inputLayout.size()
		},
		.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED, // TODO: When using strips you have to pass index buffer format to PSO 0xFFFF for 16 bit and 0xFFFFFFFF for 32 bit indices, we don't support strips for now
		.PrimitiveTopologyType = GetD3D12PrimitiveTopologyType(desc.primitiveTopologyType),
		.NumRenderTargets = (UINT)rtvs.size(),
		.RTVFormats = {DXGI_FORMAT_UNKNOWN},
		.DSVFormat = dsv ? GetDXGIFormat(dsv->GetViewDesc().format) : DXGI_FORMAT_UNKNOWN,
		.SampleDesc = GetDXGISampleDesc(desc.sampleDesc)
	};

	for (int32_t i = 0; i < (int32_t)_countof(d3d12Desc.BlendState.RenderTarget); ++i)
		d3d12Desc.BlendState.RenderTarget[i] = GetD3D12RenderTargetBlendDesc(desc.blendState.renderTargetBlendDescs[i]);
	for (int32_t i = 0; i < rtvs.size(); ++i)
		d3d12Desc.RTVFormats[i] = rtvs[i] ? GetDXGIFormat(rtvs[i]->GetViewDesc().format) : DXGI_FORMAT_UNKNOWN;

	return {
		.psoDesc = d3d12Desc,
		.inputLayout = std::move(inputLayout)
	};
}

D3D12_COMPUTE_PIPELINE_STATE_DESC GetD3D12PipelineStateDesc(const ComputePipelineStateDesc& desc, const D3D12ShaderCompilerOutput& cs)
{
	return {
		.pRootSignature = D3D12RenderDevice::Get().GetGlobalRootSignature(PipelineStateType::Compute)->GetD3D12RootSignature(),
		.CS = GetD3D12ShaderBytecode(cs.bytecode.Get())
	};
}

CD3DX12_SHADER_BYTECODE GetD3D12ShaderBytecode(IDxcBlob* shaderBytecode)
{
	PE_ASSERT(shaderBytecode);
	return {shaderBytecode->GetBufferPointer(), shaderBytecode->GetBufferSize()};
}

D3D12_SHADER_VISIBILITY GetD3D12ShaderVisibility(ShaderType shaderType)
{
	switch (shaderType)
	{
	case ShaderType::VertexShader:
		return D3D12_SHADER_VISIBILITY_VERTEX;
	case ShaderType::PixelShader:
		return D3D12_SHADER_VISIBILITY_PIXEL;
	case ShaderType::DomainShader:
		return D3D12_SHADER_VISIBILITY_DOMAIN;
	case ShaderType::HullShader:
		return D3D12_SHADER_VISIBILITY_HULL;
	case ShaderType::GeometryShader:
		return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case ShaderType::AmplificationShader:
		return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
	case ShaderType::MeshShader:
		return D3D12_SHADER_VISIBILITY_MESH;
	case ShaderType::ComputeShader:
		return D3D12_SHADER_VISIBILITY_ALL;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_FILL_MODE GetD3D12FillMode(FillMode fillMode)
{
	switch (fillMode)
	{
	case FillMode::Wireframe:
		return D3D12_FILL_MODE_WIREFRAME;
	case FillMode::Solid:
		return D3D12_FILL_MODE_SOLID;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_CULL_MODE GetD3D12CullMode(CullMode cullMode)
{
	switch (cullMode)
	{
	case CullMode::None:
		return D3D12_CULL_MODE_NONE;
	case CullMode::Front:
		return D3D12_CULL_MODE_FRONT;
	case CullMode::Back:
		return D3D12_CULL_MODE_BACK;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_COMPARISON_FUNC GetD3D12ComparisionFunc(ComparisionFunction comparisionFunction)
{
	switch (comparisionFunction)
	{
	case ComparisionFunction::Never:
		return D3D12_COMPARISON_FUNC_NEVER;
	case ComparisionFunction::Less:
		return D3D12_COMPARISON_FUNC_LESS;
	case ComparisionFunction::Equal:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case ComparisionFunction::LessEqual:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case ComparisionFunction::Greater:
		return D3D12_COMPARISON_FUNC_GREATER;
	case ComparisionFunction::NotEqual:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case ComparisionFunction::GreaterEqual:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case ComparisionFunction::Always:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_DEPTH_STENCILOP_DESC GetD3D12DepthStencilOpDesc(DepthStencilOperationDesc depthStencilOperationDesc)
{
	return {
		.StencilFailOp = GetD3D12StencilOp(depthStencilOperationDesc.stencilFail),
		.StencilDepthFailOp = GetD3D12StencilOp(depthStencilOperationDesc.stencilDepthFail),
		.StencilPassOp = GetD3D12StencilOp(depthStencilOperationDesc.stencilPass),
		.StencilFunc = GetD3D12ComparisionFunc(depthStencilOperationDesc.stencilFunction)
	};
}

D3D12_STENCIL_OP GetD3D12StencilOp(StencilOperation stencilOperation)
{
	switch (stencilOperation)
	{
	case StencilOperation::Keep:
		return D3D12_STENCIL_OP_KEEP;
	case StencilOperation::Zero:
		return D3D12_STENCIL_OP_ZERO;
	case StencilOperation::Replace:
		return D3D12_STENCIL_OP_REPLACE;
	case StencilOperation::IncrSat:
		return D3D12_STENCIL_OP_INCR_SAT;
	case StencilOperation::DecrSat:
		return D3D12_STENCIL_OP_DECR_SAT;
	case StencilOperation::Invert:
		return D3D12_STENCIL_OP_INVERT;
	case StencilOperation::IncrWrap:
		return D3D12_STENCIL_OP_INCR;
	case StencilOperation::DecrWrap:
		return D3D12_STENCIL_OP_DECR;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(TopologyType topologyType)
{
	using namespace Render;

	static D3D12_PRIMITIVE_TOPOLOGY s_d3d12TopologyTypes[(int)TopologyType::NumTopologyTypes];
	static bool s_inited = false;
	if (!s_inited)
	{
		s_d3d12TopologyTypes[(int)TopologyType::Undefined] = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		s_d3d12TopologyTypes[(int)TopologyType::TriangleList] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		s_d3d12TopologyTypes[(int)TopologyType::TriangleStrip] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		s_d3d12TopologyTypes[(int)TopologyType::TriangleListAdj] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
		s_d3d12TopologyTypes[(int)TopologyType::TriangleStripAdj] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
		s_d3d12TopologyTypes[(int)TopologyType::PointList] = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		s_d3d12TopologyTypes[(int)TopologyType::LineList] = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		s_d3d12TopologyTypes[(int)TopologyType::LineStrip] = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		s_d3d12TopologyTypes[(int)TopologyType::LineListAdj] = D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
		s_d3d12TopologyTypes[(int)TopologyType::LineStripAdj] = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;

		D3D_PRIMITIVE_TOPOLOGY patchValue = D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
		for (int32_t i = (int32_t)TopologyType::ControlPointPatchlist1; i <= (int32_t)TopologyType::ControlPointPatchlist32; ++i)
		{
			s_d3d12TopologyTypes[i] = patchValue;
			++(int32_t&)patchValue;
		}

		s_inited = true;
	}

	return s_d3d12TopologyTypes[(int)topologyType];
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType(TopologyType topologyType)
{
	using namespace Render;

	static D3D12_PRIMITIVE_TOPOLOGY_TYPE s_d3d12TopologyTypes[(int)TopologyType::NumTopologyTypes];
	static bool s_inited = false;
	if (!s_inited)
	{
		s_d3d12TopologyTypes[(int)TopologyType::Undefined] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		s_d3d12TopologyTypes[(int)TopologyType::TriangleList] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		s_d3d12TopologyTypes[(int)TopologyType::TriangleStrip] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		s_d3d12TopologyTypes[(int)TopologyType::TriangleListAdj] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		s_d3d12TopologyTypes[(int)TopologyType::TriangleStripAdj] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		s_d3d12TopologyTypes[(int)TopologyType::PointList] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		s_d3d12TopologyTypes[(int)TopologyType::LineList] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		s_d3d12TopologyTypes[(int)TopologyType::LineStrip] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		s_d3d12TopologyTypes[(int)TopologyType::LineListAdj] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		s_d3d12TopologyTypes[(int)TopologyType::LineStripAdj] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

		for (int32_t i = (int32_t)TopologyType::ControlPointPatchlist1; i <= (int32_t)TopologyType::ControlPointPatchlist32; ++i)
			s_d3d12TopologyTypes[i] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

		s_inited = true;
	}

	return s_d3d12TopologyTypes[(int)topologyType];
}

D3D12_RENDER_TARGET_BLEND_DESC GetD3D12RenderTargetBlendDesc(RenderTargetBlendDesc renderTargetBlendDesc)
{
	return {
		.BlendEnable = renderTargetBlendDesc.blendEnable,
		.LogicOpEnable = renderTargetBlendDesc.logicOperationEnable,
		.SrcBlend = GetD3D12Blend(renderTargetBlendDesc.srcBlend),
		.DestBlend = GetD3D12Blend(renderTargetBlendDesc.destBlend),
		.BlendOp = GetD3D12BlendOp(renderTargetBlendDesc.blendOperation),
		.SrcBlendAlpha = GetD3D12Blend(renderTargetBlendDesc.srcBlendAlpha),
		.DestBlendAlpha = GetD3D12Blend(renderTargetBlendDesc.destBlendAlpha),
		.BlendOpAlpha = GetD3D12BlendOp(renderTargetBlendDesc.blendOperationAlpha),
		.LogicOp = GetD3D12LogicOp(renderTargetBlendDesc.logicOperation),
		.RenderTargetWriteMask = (UINT8)renderTargetBlendDesc.renderTargetWriteMask
	};
}

D3D12_BLEND GetD3D12Blend(BlendFactor blendFactor)
{
	switch (blendFactor)
	{
	case BlendFactor::Zero:
		return D3D12_BLEND_ZERO;
	case BlendFactor::One:
		return D3D12_BLEND_ONE;
	case BlendFactor::SrcColor:
		return D3D12_BLEND_SRC_COLOR;
	case BlendFactor::InvSrcColor:
		return D3D12_BLEND_INV_SRC_COLOR;
	case BlendFactor::SrcAlpha:
		return D3D12_BLEND_SRC_ALPHA;
	case BlendFactor::InvSrcAlpha:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case BlendFactor::DestAlpha:
		return D3D12_BLEND_DEST_ALPHA;
	case BlendFactor::InvDestAlpha:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case BlendFactor::DestColor:
		return D3D12_BLEND_DEST_COLOR;
	case BlendFactor::InvDestColor:
		return D3D12_BLEND_INV_DEST_COLOR;
	case BlendFactor::SrcAlphaSat:
		return D3D12_BLEND_SRC_ALPHA_SAT;
	case BlendFactor::ConstantBlendFactor:
		return D3D12_BLEND_BLEND_FACTOR;
	case BlendFactor::InvConstantBlendFactor:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	case BlendFactor::Src1Color:
		return D3D12_BLEND_SRC1_COLOR;
	case BlendFactor::InvSrc1Color:
		return D3D12_BLEND_INV_SRC1_COLOR;
	case BlendFactor::Src1Alpha:
		return D3D12_BLEND_SRC1_ALPHA;
	case BlendFactor::InvSrc1Alpha:
		return D3D12_BLEND_INV_SRC1_ALPHA;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_BLEND_OP GetD3D12BlendOp(BlendOperation blendOperation)
{
	switch (blendOperation)
	{
	case BlendOperation::Add:
		return D3D12_BLEND_OP_ADD;
	case BlendOperation::Subtract:
		return D3D12_BLEND_OP_SUBTRACT;
	case BlendOperation::RevSubtract:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case BlendOperation::Min:
		return D3D12_BLEND_OP_MIN;
	case BlendOperation::Max:
		return D3D12_BLEND_OP_MAX;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_LOGIC_OP GetD3D12LogicOp(LogicOperation logicOperation)
{
	switch (logicOperation)
	{
	case LogicOperation::Clear:
		return D3D12_LOGIC_OP_CLEAR;
	case LogicOperation::Set:
		return D3D12_LOGIC_OP_SET;
	case LogicOperation::Copy:
		return D3D12_LOGIC_OP_COPY;
	case LogicOperation::CopyInverted:
		return D3D12_LOGIC_OP_COPY_INVERTED;
	case LogicOperation::Noop:
		return D3D12_LOGIC_OP_NOOP;
	case LogicOperation::Invert:
		return D3D12_LOGIC_OP_INVERT;
	case LogicOperation::And:
		return D3D12_LOGIC_OP_AND;
	case LogicOperation::Nand:
		return D3D12_LOGIC_OP_NAND;
	case LogicOperation::Or:
		return D3D12_LOGIC_OP_OR;
	case LogicOperation::Nor:
		return D3D12_LOGIC_OP_NOR;
	case LogicOperation::Xor:
		return D3D12_LOGIC_OP_XOR;
	case LogicOperation::Equiv:
		return D3D12_LOGIC_OP_EQUIV;
	case LogicOperation::AndReverse:
		return D3D12_LOGIC_OP_AND_REVERSE;
	case LogicOperation::AndInverted:
		return D3D12_LOGIC_OP_AND_INVERTED;
	case LogicOperation::OrReverse:
		return D3D12_LOGIC_OP_OR_REVERSE;
	case LogicOperation::OrInverted:
		return D3D12_LOGIC_OP_OR_INVERTED;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

DXGI_FORMAT GetD3D12InputElementDescFormat(const D3D12_SIGNATURE_PARAMETER_DESC& paramDesc)
{
	int32_t compNum = glm::bitCount(paramDesc.Mask);

	switch (paramDesc.ComponentType)
	{
	case D3D_REGISTER_COMPONENT_UINT32:
		switch (compNum)
		{
		case 1:
			return DXGI_FORMAT_R32_UINT;
		case 2:
			return DXGI_FORMAT_R32G32_UINT;
		case 3:
			return DXGI_FORMAT_R32G32B32_UINT;
		case 4:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		default:
			PE_ASSERT_NO_ENTRY();
		}
		break;
	case D3D_REGISTER_COMPONENT_SINT32:
		switch (compNum)
		{
		case 1:
			return DXGI_FORMAT_R32_SINT;
		case 2:
			return DXGI_FORMAT_R32G32_SINT;
		case 3:
			return DXGI_FORMAT_R32G32B32_SINT;
		case 4:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		default:
			PE_ASSERT_NO_ENTRY();
		}
		break;
	case D3D_REGISTER_COMPONENT_FLOAT32:
		switch (compNum)
		{
		case 1:
			return DXGI_FORMAT_R32_FLOAT;
		case 2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case 3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case 4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default:
			PE_ASSERT_NO_ENTRY();
		}
		break;
	default:
		PE_ASSERT_NO_ENTRY();
	}

	return {};
}

D3D12InputLayout GetD3D12InputLayoutFromVertexShader(const D3D12ShaderCompilerOutput& vertexShader)
{
	// TODO: Add a check for shader type
	D3D12InputLayout layout;

	auto* reflection = vertexShader.reflection.Get();
	D3D12_SHADER_DESC shaderDesc;
	PE_ASSERT_HR(reflection->GetDesc(&shaderDesc));

	layout.inputLayout.reserve(shaderDesc.InputParameters);
	layout.inputLayoutSemanticNames.reserve(shaderDesc.InputParameters);

	for (int32_t i = 0; i < (int32_t)shaderDesc.InputParameters; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
		PE_ASSERT_HR(reflection->GetInputParameterDesc(i, &paramDesc));

		layout.inputLayoutSemanticNames.emplace_back(paramDesc.SemanticName);

		layout.inputLayout.push_back({
			.SemanticName = layout.inputLayoutSemanticNames.back().c_str(),
			.SemanticIndex = paramDesc.SemanticIndex,
			.Format = GetD3D12InputElementDescFormat(paramDesc),
			.InputSlot = 0u,
			.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0u
		});
	}

	return layout;
}

D3D12_RESOURCE_DESC1 GetD3D12ResourceDesc(const BufferDesc& bufferDesc)
{
	return CD3DX12_RESOURCE_DESC1::Buffer(
		bufferDesc.size,
		GetD3D12ResourceFlags(bufferDesc.bindFlags)
	);
}

D3D12_RESOURCE_DESC1 GetD3D12ResourceDesc(const TextureDesc& textureDesc)
{
	return {
		.Dimension = GetD3D12ResourceDimension(textureDesc.dimension),
		.Width = (UINT64)textureDesc.GetWidth(),
		.Height = (UINT)textureDesc.GetHeight(),
		.DepthOrArraySize = (UINT16)textureDesc.GetDepthOrArraySize(),
		.MipLevels = (UINT16)textureDesc.mipLevels,
		.Format = GetDXGIFormat(textureDesc.format),
		.SampleDesc = GetDXGISampleDesc(textureDesc.sampleDesc),
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = GetD3D12ResourceFlags(textureDesc.bindFlags)
	};
}

D3D12_RESOURCE_STATES GetD3D12ResourceStatesSingleBit(ResourceStateFlags flag)
{
	PE_ASSERT(glm::isPowerOfTwo((std::underlying_type_t<ResourceStateFlags>)flag), "Only single bit must be set");

	switch (flag)
	{
	case ResourceStateFlags::Undefined:			return D3D12_RESOURCE_STATE_COMMON;
	case ResourceStateFlags::VertexBuffer:		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case ResourceStateFlags::ConstantBuffer:	return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case ResourceStateFlags::IndexBuffer:		return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	case ResourceStateFlags::RenderTarget:		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case ResourceStateFlags::UnorderedAccess:	return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case ResourceStateFlags::DepthWrite:		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case ResourceStateFlags::DepthRead:			return D3D12_RESOURCE_STATE_DEPTH_READ;
	case ResourceStateFlags::ShaderResource:	return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case ResourceStateFlags::StreamOut:			return D3D12_RESOURCE_STATE_STREAM_OUT;
	case ResourceStateFlags::IndirectArgument:	return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case ResourceStateFlags::CopyDest:			return D3D12_RESOURCE_STATE_COPY_DEST;
	case ResourceStateFlags::CopySource:		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case ResourceStateFlags::ResolveDest:		return D3D12_RESOURCE_STATE_RESOLVE_DEST;
	case ResourceStateFlags::ResolveSource:		return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
	case ResourceStateFlags::InputAttachment:	return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case ResourceStateFlags::Present:			return D3D12_RESOURCE_STATE_PRESENT;
	case ResourceStateFlags::Common:			return D3D12_RESOURCE_STATE_COMMON;

	default:
		PE_ASSERT_NO_ENTRY();
		return (D3D12_RESOURCE_STATES)0;
	}
}

D3D12_RESOURCE_STATES GetD3D12ResourceStates(Flags<ResourceStateFlags> states)
{
	static std::array<D3D12_RESOURCE_STATES, (int)ResourceStateFlags::NumStates> s_d3d12ResourceStatesMap;
	static bool s_initialized = false;
	if (!s_initialized)
	{
		for (int32_t i = 0; i < (int)ResourceStateFlags::NumStates; ++i)
			s_d3d12ResourceStatesMap[i] = GetD3D12ResourceStatesSingleBit(static_cast<ResourceStateFlags>(1 << i));
	}

	D3D12_RESOURCE_STATES output = {};

	auto bits = (std::underlying_type_t<ResourceStateFlags>)states;
	int32_t index = 0;
	while (bits != 0)
	{
		auto bit = bits & 1;
		if (bit == 1)
			output |= s_d3d12ResourceStatesMap[index];

		bits >>= 1;
		++index;
	}

	return output;
}

D3D12_RESOURCE_DIMENSION GetD3D12ResourceDimension(ResourceDimension dimension)
{
	switch (dimension)
	{
	case ResourceDimension::Undefined:
		return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	case ResourceDimension::Buffer:
		return D3D12_RESOURCE_DIMENSION_BUFFER;
	case ResourceDimension::Tex1D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	case ResourceDimension::Tex2D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case ResourceDimension::Tex3D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	case ResourceDimension::TexCube:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(Flags<BindFlags> bindFlags)
{
	D3D12_RESOURCE_FLAGS d3d12Flags = D3D12_RESOURCE_FLAG_NONE;
	if (bindFlags.HasAnyFlags(BindFlags::RenderTarget))
		d3d12Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	if (bindFlags.HasAnyFlags(BindFlags::DepthStencil))
		d3d12Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	if (bindFlags.HasAnyFlags(BindFlags::UnorderedAccess))
		d3d12Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	if (!bindFlags.HasAnyFlags(BindFlags::ShaderResource))
		d3d12Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

	return d3d12Flags;
}

CD3DX12_RESOURCE_BARRIER GetD3D12ResourceBarrier(StateTransitionDesc desc)
{
	ID3D12Resource* d3d12Resource = nullptr;
	if (desc.resource->GetResourceType() == ResourceType::Buffer)
		d3d12Resource = static_cast<D3D12Buffer*>(desc.resource)->GetD3D12Resource();
	else
		d3d12Resource = static_cast<D3D12Texture*>(desc.resource)->GetD3D12Resource();

	return CD3DX12_RESOURCE_BARRIER::Transition(
		d3d12Resource,
		GetD3D12ResourceStates(desc.oldState),
		GetD3D12ResourceStates(desc.newState));
}

CD3DX12_BUFFER_BARRIER GetD3D12BufferBarrier(BufferBarrier barrier)
{
	return D3D12_BUFFER_BARRIER{
		.SyncBefore = GetD3D12BarrierSync(barrier.syncBefore),
		.SyncAfter = GetD3D12BarrierSync(barrier.syncAfter),
		.AccessBefore = GetD3D12BarrierAccess(barrier.accessBefore),
		.AccessAfter = GetD3D12BarrierAccess(barrier.accessAfter),
		.pResource = static_cast<D3D12Buffer*>(barrier.buffer)->GetD3D12Resource(),
		.Offset = (UINT64)barrier.offset,
		.Size = (UINT64)barrier.size
	};
}

CD3DX12_TEXTURE_BARRIER GetD3D12TextureBarrier(TextureBarrier barrier)
{
	return D3D12_TEXTURE_BARRIER{
		.SyncBefore = GetD3D12BarrierSync(barrier.syncBefore),
		.SyncAfter = GetD3D12BarrierSync(barrier.syncAfter),
		.AccessBefore = GetD3D12BarrierAccess(barrier.accessBefore),
		.AccessAfter = GetD3D12BarrierAccess(barrier.accessAfter),
		.LayoutBefore = GetD3D12BarrierLayout(barrier.layoutBefore),
		.LayoutAfter = GetD3D12BarrierLayout(barrier.layoutAfter),
		.pResource = static_cast<D3D12Texture*>(barrier.texture)->GetD3D12Resource(),
		.Subresources = GetD3D12BarrierSubresourceRange(barrier.subresourceRange),
		.Flags = GetD3D12TextureBarrierFlags(barrier.flags)
	};
}

D3D12_BARRIER_SYNC GetD3D12BarrierSync(Flags<BarrierSync> barrierSync)
{
	D3D12_BARRIER_SYNC d3d12BarrierSync = D3D12_BARRIER_SYNC_NONE;
	if (barrierSync.HasAllFlags(BarrierSync::All))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_ALL;
	if (barrierSync.HasAllFlags(BarrierSync::Draw))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_DRAW;
	if (barrierSync.HasAllFlags(BarrierSync::IndexInput))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_INDEX_INPUT;
	if (barrierSync.HasAllFlags(BarrierSync::VertexShading))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
	if (barrierSync.HasAllFlags(BarrierSync::PixelShading))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
	if (barrierSync.HasAllFlags(BarrierSync::DepthStencil))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
	if (barrierSync.HasAllFlags(BarrierSync::RenderTarget))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_RENDER_TARGET;
	if (barrierSync.HasAllFlags(BarrierSync::ComputeShading))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
	if (barrierSync.HasAllFlags(BarrierSync::Raytracing))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_RAYTRACING;
	if (barrierSync.HasAllFlags(BarrierSync::Copy))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_COPY;
	if (barrierSync.HasAllFlags(BarrierSync::Resolve))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_RESOLVE;
	if (barrierSync.HasAllFlags(BarrierSync::ExecuteIndirect))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
	if (barrierSync.HasAllFlags(BarrierSync::Predication))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_PREDICATION;
	if (barrierSync.HasAllFlags(BarrierSync::AllShading))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_ALL_SHADING;
	if (barrierSync.HasAllFlags(BarrierSync::NonPixelShading))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_NON_PIXEL_SHADING;
	if (barrierSync.HasAllFlags(BarrierSync::EmitRaytracingAccelerationStructurePostbuildInfo))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO;
	if (barrierSync.HasAllFlags(BarrierSync::ClearUnorderedAccessView))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW;
	if (barrierSync.HasAllFlags(BarrierSync::VideoDecode))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_VIDEO_DECODE;
	if (barrierSync.HasAllFlags(BarrierSync::VideoProcess))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_VIDEO_PROCESS;
	if (barrierSync.HasAllFlags(BarrierSync::VideoEncode))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_VIDEO_ENCODE;
	if (barrierSync.HasAllFlags(BarrierSync::BuildRaytracingAccelerationStructure))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
	if (barrierSync.HasAllFlags(BarrierSync::CopyRaytracingAccelerationStructure))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE;
	if (barrierSync.HasAllFlags(BarrierSync::Split))
		d3d12BarrierSync |= D3D12_BARRIER_SYNC_SPLIT;

	return d3d12BarrierSync;
}

D3D12_BARRIER_ACCESS GetD3D12BarrierAccess(Flags<BarrierAccess> barrierAccess)
{
	D3D12_BARRIER_ACCESS d3d12BarrierAccess = D3D12_BARRIER_ACCESS_COMMON;
	if (barrierAccess.HasAllFlags(BarrierAccess::VertexBuffer))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
	if (barrierAccess.HasAllFlags(BarrierAccess::ConstantBuffer))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
	if (barrierAccess.HasAllFlags(BarrierAccess::IndexBuffer))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
	if (barrierAccess.HasAllFlags(BarrierAccess::RenderTarget))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
	if (barrierAccess.HasAllFlags(BarrierAccess::UnorderedAccess))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
	if (barrierAccess.HasAllFlags(BarrierAccess::DepthStencilWrite))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
	if (barrierAccess.HasAllFlags(BarrierAccess::DepthStencilRead))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
	if (barrierAccess.HasAllFlags(BarrierAccess::ShaderResource))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
	if (barrierAccess.HasAllFlags(BarrierAccess::StreamOutput))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_STREAM_OUTPUT;
	if (barrierAccess.HasAllFlags(BarrierAccess::IndirectArgument))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
	if (barrierAccess.HasAllFlags(BarrierAccess::CopyDest))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_COPY_DEST;
	if (barrierAccess.HasAllFlags(BarrierAccess::CopySource))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
	if (barrierAccess.HasAllFlags(BarrierAccess::ResolveDest))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
	if (barrierAccess.HasAllFlags(BarrierAccess::ResolveSource))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
	if (barrierAccess.HasAllFlags(BarrierAccess::RaytracingAccelerationStructureRead))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	if (barrierAccess.HasAllFlags(BarrierAccess::RaytracingAccelerationStructureWrite))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
	if (barrierAccess.HasAllFlags(BarrierAccess::ShadingRateSource))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE;
	if (barrierAccess.HasAllFlags(BarrierAccess::VideoDecodeRead))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_VIDEO_DECODE_READ;
	if (barrierAccess.HasAllFlags(BarrierAccess::VideoDecodeWrite))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_VIDEO_DECODE_WRITE;
	if (barrierAccess.HasAllFlags(BarrierAccess::VideoProcessRead))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_VIDEO_PROCESS_READ;
	if (barrierAccess.HasAllFlags(BarrierAccess::VideoProcessWrite))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_VIDEO_PROCESS_WRITE;
	if (barrierAccess.HasAllFlags(BarrierAccess::VideoEncodeRead))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_VIDEO_ENCODE_READ;
	if (barrierAccess.HasAllFlags(BarrierAccess::VideoEncodeWrite))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_VIDEO_ENCODE_WRITE;
	if (barrierAccess.HasAllFlags(BarrierAccess::NoAccess))
		d3d12BarrierAccess |= D3D12_BARRIER_ACCESS_NO_ACCESS;

	return d3d12BarrierAccess;
}

D3D12_BARRIER_LAYOUT GetD3D12BarrierLayout(BarrierLayout barrierLayout)
{
	switch (barrierLayout)
	{
	case BarrierLayout::Undefined:
		return D3D12_BARRIER_LAYOUT_UNDEFINED;
	case BarrierLayout::Common: // same as BarrierLayout::Present
		return D3D12_BARRIER_LAYOUT_COMMON; // same as D3D12_BARRIER_LAYOUT_PRESENT
	case BarrierLayout::GenericRead:
		return D3D12_BARRIER_LAYOUT_GENERIC_READ;
	case BarrierLayout::RenderTarget:
		return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
	case BarrierLayout::UnorderedAccess:
		return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
	case BarrierLayout::DepthStencilWrite:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
	case BarrierLayout::DepthStencilRead:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
	case BarrierLayout::ShaderResource:
		return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
	case BarrierLayout::CopySource:
		return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
	case BarrierLayout::CopyDest:
		return D3D12_BARRIER_LAYOUT_COPY_DEST;
	case BarrierLayout::ResolveSource:
		return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
	case BarrierLayout::ResolveDest:
		return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
	case BarrierLayout::ShadingRateSource:
		return D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE;
	case BarrierLayout::VideoDecodeRead:
		return D3D12_BARRIER_LAYOUT_VIDEO_DECODE_READ;
	case BarrierLayout::VideoDecodeWrite:
		return D3D12_BARRIER_LAYOUT_VIDEO_DECODE_WRITE;
	case BarrierLayout::VideoProcessRead:
		return D3D12_BARRIER_LAYOUT_VIDEO_PROCESS_READ;
	case BarrierLayout::VideoProcessWrite:
		return D3D12_BARRIER_LAYOUT_VIDEO_PROCESS_WRITE;
	case BarrierLayout::VideoEncodeRead:
		return D3D12_BARRIER_LAYOUT_VIDEO_ENCODE_READ;
	case BarrierLayout::VideoEncodeWrite:
		return D3D12_BARRIER_LAYOUT_VIDEO_ENCODE_WRITE;
	case BarrierLayout::DirectQueueCommon:
		return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON;
	case BarrierLayout::DirectQueueGenericRead:
		return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ;
	case BarrierLayout::DirectQueueUnorderedAccess:
		return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS;
	case BarrierLayout::DirectQueueShaderResource:
		return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE;
	case BarrierLayout::DirectQueueCopySource:
		return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE;
	case BarrierLayout::DirectQueueCopyDest:
		return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST;
	case BarrierLayout::ComputeQueueCommon:
		return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON;
	case BarrierLayout::ComputeQueueGenericRead:
		return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_GENERIC_READ;
	case BarrierLayout::ComputeQueueUnorderedAccess:
		return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS;
	case BarrierLayout::ComputeQueueShaderResource:
		return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE;
	case BarrierLayout::ComputeQueueCopySource:
		return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE;
	case BarrierLayout::ComputeQueueCopyDest:
		return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST;
	case BarrierLayout::VideoQueueCommon:
		return D3D12_BARRIER_LAYOUT_VIDEO_QUEUE_COMMON;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_BARRIER_SUBRESOURCE_RANGE GetD3D12BarrierSubresourceRange(SubresourceRange subresourceRange)
{
	if (subresourceRange.numMipLevels == -1)
	{
		return {
			.IndexOrFirstMipLevel = 0xffffffff
		};
	}
	else
	{
		return {
			.IndexOrFirstMipLevel = (UINT)subresourceRange.firstMipLevel,
			.NumMipLevels = (UINT)subresourceRange.numMipLevels,
			.FirstArraySlice = (UINT)subresourceRange.firstArraySlice,
			.NumArraySlices = (UINT)subresourceRange.numArraySlices,
			.FirstPlane = 0,
			.NumPlanes = 0 
		};
	}
}

D3D12_TEXTURE_BARRIER_FLAGS GetD3D12TextureBarrierFlags(Flags<TextureBarrierFlags> textureBarrierFlags)
{
	D3D12_TEXTURE_BARRIER_FLAGS d3d12TextureBarrierFlags = D3D12_TEXTURE_BARRIER_FLAG_NONE;

	if (textureBarrierFlags.HasAllFlags(TextureBarrierFlags::Discard))
		d3d12TextureBarrierFlags |= D3D12_TEXTURE_BARRIER_FLAG_DISCARD;

	return d3d12TextureBarrierFlags;
}

D3D12_CLEAR_VALUE GetD3D12ClearValue(ClearValue clearValue)
{
	if (std::holds_alternative<RenderTargetClearValue>(clearValue))
	{
		auto& [format, color] = std::get<RenderTargetClearValue>(clearValue);

		return {
			.Format = GetDXGIFormat(format),
			.Color = {color.r, color.g, color.b, color.a}
		};
	}
	else if (std::holds_alternative<DepthStencilClearValue>(clearValue))
	{
		auto& [format, depthStencil] = std::get<DepthStencilClearValue>(clearValue);

		return {
			.Format = GetDXGIFormat(format),
			.DepthStencil = {
				.Depth = depthStencil.depth,
				.Stencil = depthStencil.stencil
			} 
		};
	}

	PE_ASSERT_NO_ENTRY();
	return {};
}

D3D12_DESCRIPTOR_HEAP_TYPE GetD3D12DescriptorHeapType(BufferViewType bufferViewType)
{
	switch (bufferViewType)
	{
	case BufferViewType::CBV:
	case BufferViewType::SRV:
	case BufferViewType::UAV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_DESCRIPTOR_HEAP_TYPE GetD3D12DescriptorHeapType(TextureViewType textureViewType)
{
	switch (textureViewType)
	{
	case TextureViewType::SRV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	case TextureViewType::UAV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	case TextureViewType::RTV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	case TextureViewType::DSV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_CONSTANT_BUFFER_VIEW_DESC GetD3D12ConstantBufferViewDesc(Buffer* buffer, BufferViewDesc viewDesc)
{
	return GetD3D12ConstantBufferViewDesc(static_cast<D3D12Buffer*>(buffer)->GetD3D12Resource(), viewDesc);
}

D3D12_CONSTANT_BUFFER_VIEW_DESC GetD3D12ConstantBufferViewDesc(ID3D12Resource* buffer, BufferViewDesc viewDesc)
{
	uint64_t location = buffer->GetGPUVirtualAddress() + viewDesc.offset;
	return {
		.BufferLocation = location,
		.SizeInBytes = (uint32_t)viewDesc.size
	};
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetD3D12ShaderResourceViewDesc(BufferViewDesc desc)
{
	PE_ASSERT(desc.type == BufferViewType::SRV);
	PE_ASSERT(desc.elementSize > 0, "Element size must be set when creating an SRV of a buffer");
	PE_ASSERT(desc.offset % desc.elementSize == 0, "Offset must be a multiple of an element size");
	PE_ASSERT(desc.size % desc.elementSize == 0, "Buffer size must be a multiple of an element size");

	return {
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer = {
			.FirstElement = (UINT64)(desc.offset / desc.elementSize),
			.NumElements = (UINT)(desc.size / desc.elementSize),
			.StructureByteStride = (UINT)desc.elementSize,
			// TODO: Add support for raw buffers
			.Flags = D3D12_BUFFER_SRV_FLAG_NONE
		}
	};
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetD3D12ShaderResourceViewDesc(TextureViewDesc desc)
{
	PE_ASSERT(desc.type == TextureViewType::SRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC d3d12SRVDesc = {
		.Format = GetDXGIFormat(desc.format),
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
	};

	// We ignore buffer type here because we don't want there to be an option to create a buffer SRV from texture view
	switch (desc.dimension)
	{
	case ResourceDimension::Tex1D:
		{
			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				d3d12SRVDesc.Texture1D = {
					.MostDetailedMip = (UINT)desc.subresourceRange.firstMipLevel,
					.MipLevels = (UINT)desc.subresourceRange.numMipLevels,
					.ResourceMinLODClamp = 0.f
				};
			}
			else
			{
				d3d12SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				d3d12SRVDesc.Texture1DArray = {
					.MostDetailedMip = (UINT)desc.subresourceRange.firstMipLevel,
					.MipLevels = (UINT)desc.subresourceRange.numMipLevels,
					.FirstArraySlice = (UINT)desc.subresourceRange.firstArraySlice,
					.ArraySize = (UINT)desc.subresourceRange.numArraySlices,
					.ResourceMinLODClamp = 0.f
				};
			}
			break;
		}
	case ResourceDimension::Tex2D:
		{
			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				d3d12SRVDesc.Texture2D = {
					.MostDetailedMip = (UINT)desc.subresourceRange.firstMipLevel,
					.MipLevels = (UINT)desc.subresourceRange.numMipLevels,
					// TODO: should probably revisit this
					.PlaneSlice = GetPlaneSlice(desc.format),
					// TODO: add support for this
					.ResourceMinLODClamp = 0.f
				};
			}
			else
			{
				d3d12SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				d3d12SRVDesc.Texture2DArray = {
					.MostDetailedMip = (UINT)desc.subresourceRange.firstMipLevel,
					.MipLevels = (UINT)desc.subresourceRange.numMipLevels,
					.FirstArraySlice = (UINT)desc.subresourceRange.firstArraySlice,
					.ArraySize = (UINT)desc.subresourceRange.numArraySlices,
					.PlaneSlice = GetPlaneSlice(desc.format),
					.ResourceMinLODClamp = 0.f
				};
			}
			break;
		}
	case ResourceDimension::Tex3D:
		{
			PE_ASSERT(desc.subresourceRange.firstArraySlice == 0 && desc.subresourceRange.numArraySlices == 1, "3D textures cannot be arrays");

			d3d12SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			d3d12SRVDesc.Texture3D = {
				.MostDetailedMip = (UINT)desc.subresourceRange.firstMipLevel,
				.MipLevels = (UINT)desc.subresourceRange.numMipLevels,
				.ResourceMinLODClamp = 0.f
			};
			break;
		}
	case ResourceDimension::TexCube:
		{
			PE_ASSERT(desc.IsCube());

			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				d3d12SRVDesc.TextureCube = {
					.MostDetailedMip = (UINT)desc.subresourceRange.firstMipLevel,
					.MipLevels = (UINT)desc.subresourceRange.numMipLevels,
					.ResourceMinLODClamp = 0.f
				};
			}
			else
			{
				d3d12SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				d3d12SRVDesc.TextureCubeArray = {
					.MostDetailedMip = (UINT)desc.subresourceRange.firstMipLevel,
					.MipLevels = (UINT)desc.subresourceRange.numMipLevels,
					.First2DArrayFace = (UINT)desc.subresourceRange.firstArraySlice,
					.NumCubes = (UINT)desc.subresourceRange.numArraySlices / 6,
					.ResourceMinLODClamp = 0.f
				};
			}
			break;
		}
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}

	return d3d12SRVDesc;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC GetD3D12UnorderedAccessViewDesc(BufferViewDesc desc)
{
	PE_ASSERT(desc.type == BufferViewType::UAV);
	PE_ASSERT(desc.elementSize > 0, "Element size must be set when creating an UAV of a buffer");
	PE_ASSERT(desc.offset % desc.elementSize == 0, "Offset must be a multiple of an element size");
	PE_ASSERT(desc.size % desc.elementSize == 0, "Buffer size must be a multiple of an element size");

	return {
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
		.Buffer = {
			.FirstElement = (UINT64)(desc.offset / desc.elementSize),
			.NumElements = (UINT)(desc.size / desc.elementSize),
			.StructureByteStride = (UINT)desc.elementSize,
			.CounterOffsetInBytes = 0,
			// TODO: Add support for raw buffers
			.Flags = D3D12_BUFFER_UAV_FLAG_NONE
		}
	};
}

D3D12_UNORDERED_ACCESS_VIEW_DESC GetD3D12UnorderedAccessViewDesc(TextureViewDesc desc)
{
	PE_ASSERT(desc.type == TextureViewType::UAV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC d3d12UAVDesc = {
		.Format = GetDXGIFormat(desc.format)
	};

	switch (desc.dimension)
	{
	case ResourceDimension::Tex1D:
		{
			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
				d3d12UAVDesc.Texture1D = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel
				};
			}
			else
			{
				d3d12UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
				d3d12UAVDesc.Texture1DArray = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel,
					.FirstArraySlice = (UINT)desc.subresourceRange.firstArraySlice,
					.ArraySize = (UINT)desc.subresourceRange.numArraySlices
				};
			}
			break;
		}
	case ResourceDimension::Tex2D:
		{
			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				d3d12UAVDesc.Texture2D = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel
				};
			}
			else
			{
				d3d12UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				d3d12UAVDesc.Texture2DArray = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel,
					.FirstArraySlice = (UINT)desc.subresourceRange.firstArraySlice,
					.ArraySize = (UINT)desc.subresourceRange.numArraySlices
				};
			}
			break;
		}
	case ResourceDimension::Tex3D:
		{
			PE_ASSERT(desc.subresourceRange.firstArraySlice == 0 && desc.subresourceRange.numArraySlices == 1, "3D textures cannot be arrays");

			d3d12UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			d3d12UAVDesc.Texture3D = {
				.MipSlice = (UINT)desc.subresourceRange.firstMipLevel,
				.FirstWSlice = (UINT)desc.subresourceRange.firstArraySlice,
				.WSize = (UINT)desc.subresourceRange.numArraySlices
			};
			break;
		}
	case ResourceDimension::TexCube:
		{
			PE_ASSERT_NO_ENTRY("Use Tex2D array for UAVs");
			return {};
		}
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}

	return d3d12UAVDesc;
}

D3D12_RENDER_TARGET_VIEW_DESC GetD3D12RenderTargetViewDesc(TextureViewDesc desc)
{
	PE_ASSERT(desc.type == TextureViewType::RTV);

	D3D12_RENDER_TARGET_VIEW_DESC d3d12RTVDesc = {
		.Format = GetDXGIFormat(desc.format)
	};

	switch (desc.dimension)
	{
	case ResourceDimension::Tex1D:
		{
			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
				d3d12RTVDesc.Texture1D = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel
				};
			}
			else
			{
				d3d12RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
				d3d12RTVDesc.Texture1DArray = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel,
					.FirstArraySlice = (UINT)desc.subresourceRange.firstArraySlice,
					.ArraySize = (UINT)desc.subresourceRange.numArraySlices
				};
			}
			break;
		}
	case ResourceDimension::Tex2D:
		{
			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				d3d12RTVDesc.Texture2D = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel
				};
			}
			else
			{
				d3d12RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				d3d12RTVDesc.Texture2DArray = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel,
					.FirstArraySlice = (UINT)desc.subresourceRange.firstArraySlice,
					.ArraySize = (UINT)desc.subresourceRange.numArraySlices
				};
			}
			break;
		}
	case ResourceDimension::Tex3D:
		{
			d3d12RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			d3d12RTVDesc.Texture3D = {
				.MipSlice = (UINT)desc.subresourceRange.firstMipLevel,
				.FirstWSlice = (UINT)desc.subresourceRange.firstArraySlice,
				.WSize = (UINT)desc.subresourceRange.numArraySlices
			};
			break;
		}
	case ResourceDimension::TexCube:
		{
			PE_ASSERT_NO_ENTRY("Use Tex2D array for RTVs");
			return {};
		}
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}

	return d3d12RTVDesc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC GetD3D12DepthStencilViewDesc(TextureViewDesc desc)
{
	PE_ASSERT(desc.type == TextureViewType::DSV);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3d12DSVDesc = {
		.Format = GetDXGIFormat(desc.format),
	};

	switch (desc.dimension)
	{
	case ResourceDimension::Tex1D:
		{
			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
				d3d12DSVDesc.Texture1D = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel
				};
			}
			else
			{
				d3d12DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
				d3d12DSVDesc.Texture1DArray = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel,
					.FirstArraySlice = (UINT)desc.subresourceRange.firstArraySlice,
					.ArraySize = (UINT)desc.subresourceRange.numArraySlices
				};
			}
			break;
		}
	case ResourceDimension::Tex2D:
		{
			if (!ShouldD3D12ViewBeCreatedAsArray(desc))
			{
				d3d12DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				d3d12DSVDesc.Texture2D = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel
				};
			}
			else
			{
				d3d12DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				d3d12DSVDesc.Texture2DArray = {
					.MipSlice = (UINT)desc.subresourceRange.firstMipLevel,
					.FirstArraySlice = (UINT)desc.subresourceRange.firstArraySlice,
					.ArraySize = (UINT)desc.subresourceRange.numArraySlices
				};
			}
			break;
		}
	case ResourceDimension::TexCube:
		{
			PE_ASSERT_NO_ENTRY("Use Tex2D array for DSVs");
			return {};
		}
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}

	return d3d12DSVDesc;
}

D3D12_VIEWPORT GetD3D12Viewport(Viewport viewport)
{
	return {
		.TopLeftX = viewport.topLeft.x,
		.TopLeftY = viewport.topLeft.y,
		.Width = viewport.size.x,
		.Height = viewport.size.y,
		.MinDepth = viewport.depthRange.x,
		.MaxDepth = viewport.depthRange.y
	};
}

D3D12_RECT GetD3D12Rect(Scissor scissor)
{
	return {
		.left = scissor.topLeft.x,
		.top = scissor.topLeft.y,
		.right = scissor.size.x,
		.bottom = scissor.size.y
	};
}

D3D12_BOX GetD3D12Box(Box box, Texture* texture, int32_t subresource)
{
	return {
		.left = (UINT)box.location.x,
		.top = (UINT)box.location.y,
		.front = (UINT)box.location.z,
		.right = (UINT)(box.size.x == -1 ? max(1, texture->GetTextureDesc().GetWidth() >> subresource) - box.location.x : box.location.x + box.size.x),
		.bottom = (UINT)(box.size.y == -1 ? max(1, texture->GetTextureDesc().GetHeight() >> subresource) - box.location.y : (UINT)box.location.y + box.size.y),
		.back = (UINT)(box.size.z == -1 ? max(1, texture->GetTextureDesc().GetDepth() >> subresource) - box.location.z : (UINT)box.location.z + box.size.z)
	};
}

D3D12_CLEAR_FLAGS GetD3D12ClearFlags(Flags<ClearFlags> clearFlags)
{
	D3D12_CLEAR_FLAGS d3d12Flags = {};
	if (clearFlags.HasAnyFlags(ClearFlags::ClearDepth))
		d3d12Flags |= D3D12_CLEAR_FLAG_DEPTH;
	if (clearFlags.HasAnyFlags(ClearFlags::ClearStencil))
		d3d12Flags |= D3D12_CLEAR_FLAG_STENCIL;

	return d3d12Flags;
}

DXGI_FORMAT GetIndexBufferDXGIFormat(IndexBufferFormat format)
{
	switch (format)
	{
	case IndexBufferFormat::Uint16:
		return DXGI_FORMAT_R16_UINT;
	case IndexBufferFormat::Uint32:
		return DXGI_FORMAT_R32_UINT;
	default:
		PE_ASSERT_NO_ENTRY();
	}

	return {};
}

UINT GetPlaneSlice(TextureFormat format)
{
	// Stencil is in plane 1
	return format == TextureFormat::X32_Typeless_G8X24_UInt || format == TextureFormat::X24_Typeless_G8_UInt ? 1 : 0;
}

TextureFormat GetTextureFormat(DXGI_FORMAT dxgiFormat)
{
	static TextureFormat s_textureFormatMap[DXGI_FORMAT_B4G4R4A4_UNORM + 1] = { TextureFormat::Unknown };
	static bool s_initialized = false;
	if (!s_initialized)
	{
		for (TextureFormat format = TextureFormat::Unknown;
			format < TextureFormat::NumFormats;
			++(std::underlying_type_t<TextureFormat>&)format)
		{
			DXGI_FORMAT dxgiFmt = GetDXGIFormat(format);
			s_textureFormatMap[dxgiFmt] = format;
		}
	}

	return s_textureFormatMap[dxgiFormat];
}

SampleDesc GetSampleDesc(DXGI_SAMPLE_DESC dxgiSampleDesc)
{
	return {
		.count = (int32_t)dxgiSampleDesc.Count,
		.quality = (int32_t)dxgiSampleDesc.Quality
	};
}

Flags<BindFlags> GetBindFlags(D3D12_RESOURCE_FLAGS resourceFlags)
{
	Flags<BindFlags> bindFlags;
	if ((resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0)
		bindFlags |= BindFlags::RenderTarget;
	if ((resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0)
		bindFlags |= BindFlags::DepthStencil;
	if ((resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0)
		bindFlags |= BindFlags::UnorderedAccess;
	if ((resourceFlags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0)
		bindFlags |= BindFlags::ShaderResource;

	return bindFlags;
}

ResourceDimension GetResourceDimension(D3D12_RESOURCE_DIMENSION d3d12Dimension, bool isCube)
{
	switch (d3d12Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_UNKNOWN:
		return ResourceDimension::Undefined;
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		return ResourceDimension::Buffer;

	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		return ResourceDimension::Tex1D;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (isCube)
			return ResourceDimension::TexCube;
		else
			return ResourceDimension::Tex2D;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		return ResourceDimension::Tex3D;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

TextureDesc GetTextureDesc(const D3D12_RESOURCE_DESC& d3d12ResDesc,
						   const std::wstring& name,
						   ResourceUsage usage,
						   std::optional<ClearValue> optimizedClearValue,
						   bool isCubeTexture)
{
	PE_ASSERT(!isCubeTexture || d3d12ResDesc.DepthOrArraySize == 6);

	return {
		.textureName = name,
		.width = (int32_t)d3d12ResDesc.Width,
		.height = (int32_t)d3d12ResDesc.Height,
		.depthOrArraySize = (int32_t)d3d12ResDesc.DepthOrArraySize,
		.mipLevels = (int32_t)d3d12ResDesc.MipLevels,
		.dimension = GetResourceDimension(d3d12ResDesc.Dimension, isCubeTexture),
		.format = GetTextureFormat(d3d12ResDesc.Format),
		.sampleDesc = GetSampleDesc(d3d12ResDesc.SampleDesc),
		.bindFlags = GetBindFlags(d3d12ResDesc.Flags),
		.usage = usage,
		.optimizedClearValue = optimizedClearValue
	};
}

D3D12_SRV_DIMENSION GetD3D12SRVDimension(D3D_SRV_DIMENSION dimension)
{
	switch (dimension)
	{
	case D3D_SRV_DIMENSION_BUFFER:
		return D3D12_SRV_DIMENSION_BUFFER;
	case D3D_SRV_DIMENSION_TEXTURE1D:
		return D3D12_SRV_DIMENSION_TEXTURE1D;
	case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
		return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
	case D3D_SRV_DIMENSION_TEXTURE2D:
		return D3D12_SRV_DIMENSION_TEXTURE2D;
	case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
		return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	case D3D_SRV_DIMENSION_TEXTURE2DMS:
		return D3D12_SRV_DIMENSION_TEXTURE2DMS;
	case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
		return D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
	case D3D_SRV_DIMENSION_TEXTURE3D:
		return D3D12_SRV_DIMENSION_TEXTURE3D;
	case D3D_SRV_DIMENSION_TEXTURECUBE:
		return D3D12_SRV_DIMENSION_TEXTURECUBE;
	case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
		return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	default: 
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_UAV_DIMENSION GetD3D12UAVDimension(D3D_SRV_DIMENSION dimension)
{
	switch (dimension)
	{
	case D3D_SRV_DIMENSION_BUFFER:
		return D3D12_UAV_DIMENSION_BUFFER;
	case D3D_SRV_DIMENSION_TEXTURE1D:
		return D3D12_UAV_DIMENSION_TEXTURE1D;
	case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
		return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
	case D3D_SRV_DIMENSION_TEXTURE2D:
		return D3D12_UAV_DIMENSION_TEXTURE2D;
	case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
		return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	case D3D_SRV_DIMENSION_TEXTURE3D:
		return D3D12_UAV_DIMENSION_TEXTURE3D;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}
}
