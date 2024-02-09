#include "pcpch.h"
#include "D3D12TypeConversions.h"

#include "RenderAPI/D3D12/D3D12RenderAPI.h"
#include "RenderAPI/D3D12/D3D12RootSignature.h"
#include "RenderAPI/D3D12/D3D12ShaderImpl.h"

namespace Prism::D3D12
{
DXGI_FORMAT GetDXGIFormat(Render::TextureFormat format)
{
	using namespace Render;

	static DXGI_FORMAT dxgiFormatMap[(int)TextureFormat::NumFormats] = { DXGI_FORMAT_UNKNOWN };
	static bool s_formatMapInitialized = false;

	if (!s_formatMapInitialized)
	{
		dxgiFormatMap[(int)TextureFormat::Unknown]                   = DXGI_FORMAT_UNKNOWN;

		dxgiFormatMap[(int)TextureFormat::RGBA32_Typeless]           = DXGI_FORMAT_R32G32B32A32_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::RGBA32_Float]              = DXGI_FORMAT_R32G32B32A32_FLOAT;
		dxgiFormatMap[(int)TextureFormat::RGBA32_UInt]               = DXGI_FORMAT_R32G32B32A32_UINT;
		dxgiFormatMap[(int)TextureFormat::RGBA32_SInt]               = DXGI_FORMAT_R32G32B32A32_SINT;

		dxgiFormatMap[(int)TextureFormat::RGB32_Typeless]            = DXGI_FORMAT_R32G32B32_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::RGB32_Float]               = DXGI_FORMAT_R32G32B32_FLOAT;
		dxgiFormatMap[(int)TextureFormat::RGB32_UInt]                = DXGI_FORMAT_R32G32B32_UINT;
		dxgiFormatMap[(int)TextureFormat::RGB32_SInt]                = DXGI_FORMAT_R32G32B32_SINT;

		dxgiFormatMap[(int)TextureFormat::RGBA16_Typeless]           = DXGI_FORMAT_R16G16B16A16_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::RGBA16_Float]              = DXGI_FORMAT_R16G16B16A16_FLOAT;
		dxgiFormatMap[(int)TextureFormat::RGBA16_UNorm]              = DXGI_FORMAT_R16G16B16A16_UNORM;
		dxgiFormatMap[(int)TextureFormat::RGBA16_UInt]               = DXGI_FORMAT_R16G16B16A16_UINT;
		dxgiFormatMap[(int)TextureFormat::RGBA16_SNorm]              = DXGI_FORMAT_R16G16B16A16_SNORM;
		dxgiFormatMap[(int)TextureFormat::RGBA16_SInt]               = DXGI_FORMAT_R16G16B16A16_SINT;

		dxgiFormatMap[(int)TextureFormat::RG32_Typeless]             = DXGI_FORMAT_R32G32_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::RG32_Float]                = DXGI_FORMAT_R32G32_FLOAT;
		dxgiFormatMap[(int)TextureFormat::RG32_UInt]                 = DXGI_FORMAT_R32G32_UINT;
		dxgiFormatMap[(int)TextureFormat::RG32_SInt]                 = DXGI_FORMAT_R32G32_SINT;

		dxgiFormatMap[(int)TextureFormat::R32G8X24_Typeless]         = DXGI_FORMAT_R32G8X24_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::D32_Float_S8X24_UInt]      = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		dxgiFormatMap[(int)TextureFormat::R32_Float_X8X24_Typeless]  = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::X32_Typeless_G8X24_UInt]   = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

		dxgiFormatMap[(int)TextureFormat::RGB10A2_Typeless]          = DXGI_FORMAT_R10G10B10A2_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::RGB10A2_UNorm]             = DXGI_FORMAT_R10G10B10A2_UNORM;
		dxgiFormatMap[(int)TextureFormat::RGB10A2_UInt]              = DXGI_FORMAT_R10G10B10A2_UINT;

		dxgiFormatMap[(int)TextureFormat::R11G11B10_Float]           = DXGI_FORMAT_R11G11B10_FLOAT;

		dxgiFormatMap[(int)TextureFormat::RGBA8_Typeless]            = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::RGBA8_UNorm]               = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgiFormatMap[(int)TextureFormat::RGBA8_UNorm_SRGB]          = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		dxgiFormatMap[(int)TextureFormat::RGBA8_UInt]                = DXGI_FORMAT_R8G8B8A8_UINT;
		dxgiFormatMap[(int)TextureFormat::RGBA8_SNorm]               = DXGI_FORMAT_R8G8B8A8_SNORM;
		dxgiFormatMap[(int)TextureFormat::RGBA8_SInt]                = DXGI_FORMAT_R8G8B8A8_SINT;

		dxgiFormatMap[(int)TextureFormat::RG16_Typeless]             = DXGI_FORMAT_R16G16_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::RG16_Float]                = DXGI_FORMAT_R16G16_FLOAT;
		dxgiFormatMap[(int)TextureFormat::RG16_UNorm]                = DXGI_FORMAT_R16G16_UNORM;
		dxgiFormatMap[(int)TextureFormat::RG16_UInt]                 = DXGI_FORMAT_R16G16_UINT;
		dxgiFormatMap[(int)TextureFormat::RG16_SNorm]                = DXGI_FORMAT_R16G16_SNORM;
		dxgiFormatMap[(int)TextureFormat::RG16_SInt]                 = DXGI_FORMAT_R16G16_SINT;

		dxgiFormatMap[(int)TextureFormat::R32_Typeless]              = DXGI_FORMAT_R32_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::D32_Float]                 = DXGI_FORMAT_D32_FLOAT;
		dxgiFormatMap[(int)TextureFormat::R32_Float]                 = DXGI_FORMAT_R32_FLOAT;
		dxgiFormatMap[(int)TextureFormat::R32_UInt]                  = DXGI_FORMAT_R32_UINT;
		dxgiFormatMap[(int)TextureFormat::R32_SInt]                  = DXGI_FORMAT_R32_SINT;

		dxgiFormatMap[(int)TextureFormat::R24G8_Typeless]            = DXGI_FORMAT_R24G8_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::D24_UNorm_S8_UInt]         = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dxgiFormatMap[(int)TextureFormat::R24_UNorm_X8_Typeless]     = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::X24_Typeless_G8_UInt]      = DXGI_FORMAT_X24_TYPELESS_G8_UINT;

		dxgiFormatMap[(int)TextureFormat::RG8_Typeless]              = DXGI_FORMAT_R8G8_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::RG8_UNorm]                 = DXGI_FORMAT_R8G8_UNORM;
		dxgiFormatMap[(int)TextureFormat::RG8_UInt]                  = DXGI_FORMAT_R8G8_UINT;
		dxgiFormatMap[(int)TextureFormat::RG8_SNorm]                 = DXGI_FORMAT_R8G8_SNORM;
		dxgiFormatMap[(int)TextureFormat::RG8_SInt]                  = DXGI_FORMAT_R8G8_SINT;

		dxgiFormatMap[(int)TextureFormat::R16_Typeless]              = DXGI_FORMAT_R16_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::R16_Float]                 = DXGI_FORMAT_R16_FLOAT;
		dxgiFormatMap[(int)TextureFormat::D16_UNorm]                 = DXGI_FORMAT_D16_UNORM;
		dxgiFormatMap[(int)TextureFormat::R16_UNorm]                 = DXGI_FORMAT_R16_UNORM;
		dxgiFormatMap[(int)TextureFormat::R16_UInt]                  = DXGI_FORMAT_R16_UINT;
		dxgiFormatMap[(int)TextureFormat::R16_SNorm]                 = DXGI_FORMAT_R16_SNORM;
		dxgiFormatMap[(int)TextureFormat::R16_SInt]                  = DXGI_FORMAT_R16_SINT;

		dxgiFormatMap[(int)TextureFormat::R8_Typeless]               = DXGI_FORMAT_R8_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::R8_UNorm]                  = DXGI_FORMAT_R8_UNORM;
		dxgiFormatMap[(int)TextureFormat::R8_UInt]                   = DXGI_FORMAT_R8_UINT;
		dxgiFormatMap[(int)TextureFormat::R8_SNorm]                  = DXGI_FORMAT_R8_SNORM;
		dxgiFormatMap[(int)TextureFormat::R8_SInt]                   = DXGI_FORMAT_R8_SINT;
		dxgiFormatMap[(int)TextureFormat::A8_UNorm]                  = DXGI_FORMAT_A8_UNORM;

		dxgiFormatMap[(int)TextureFormat::R1_UNorm]                  = DXGI_FORMAT_R1_UNORM ;
		dxgiFormatMap[(int)TextureFormat::RGB9E5_SHAREDEXP]          = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		dxgiFormatMap[(int)TextureFormat::RG8_B8G8_UNorm]            = DXGI_FORMAT_R8G8_B8G8_UNORM;
		dxgiFormatMap[(int)TextureFormat::G8R8_G8B8_UNorm]           = DXGI_FORMAT_G8R8_G8B8_UNORM;

		dxgiFormatMap[(int)TextureFormat::BC1_Typeless]              = DXGI_FORMAT_BC1_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::BC1_UNorm]                 = DXGI_FORMAT_BC1_UNORM;
		dxgiFormatMap[(int)TextureFormat::BC1_UNorm_SRGB]            = DXGI_FORMAT_BC1_UNORM_SRGB;
		dxgiFormatMap[(int)TextureFormat::BC2_Typeless]              = DXGI_FORMAT_BC2_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::BC2_UNorm]                 = DXGI_FORMAT_BC2_UNORM;
		dxgiFormatMap[(int)TextureFormat::BC2_UNorm_SRGB]            = DXGI_FORMAT_BC2_UNORM_SRGB;
		dxgiFormatMap[(int)TextureFormat::BC3_Typeless]              = DXGI_FORMAT_BC3_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::BC3_UNorm]                 = DXGI_FORMAT_BC3_UNORM;
		dxgiFormatMap[(int)TextureFormat::BC3_UNorm_SRGB]            = DXGI_FORMAT_BC3_UNORM_SRGB;
		dxgiFormatMap[(int)TextureFormat::BC4_Typeless]              = DXGI_FORMAT_BC4_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::BC4_UNorm]                 = DXGI_FORMAT_BC4_UNORM;
		dxgiFormatMap[(int)TextureFormat::BC4_SNorm]                 = DXGI_FORMAT_BC4_SNORM;
		dxgiFormatMap[(int)TextureFormat::BC5_Typeless]              = DXGI_FORMAT_BC5_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::BC5_UNorm]                 = DXGI_FORMAT_BC5_UNORM;
		dxgiFormatMap[(int)TextureFormat::BC5_SNorm]                 = DXGI_FORMAT_BC5_SNORM;

		dxgiFormatMap[(int)TextureFormat::B5G6R5_UNorm]              = DXGI_FORMAT_B5G6R5_UNORM;
		dxgiFormatMap[(int)TextureFormat::B5G5R5A1_UNorm]            = DXGI_FORMAT_B5G5R5A1_UNORM;
		dxgiFormatMap[(int)TextureFormat::BGRA8_UNorm]               = DXGI_FORMAT_B8G8R8A8_UNORM;
		dxgiFormatMap[(int)TextureFormat::BGRX8_UNorm]               = DXGI_FORMAT_B8G8R8X8_UNORM;

		dxgiFormatMap[(int)TextureFormat::R10G10B10_XR_BIAS_A2_UNorm]= DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

		dxgiFormatMap[(int)TextureFormat::BGRA8_Typeless]            = DXGI_FORMAT_B8G8R8A8_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::BGRA8_UNorm_SRGB]          = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		dxgiFormatMap[(int)TextureFormat::BGRX8_Typeless]            = DXGI_FORMAT_B8G8R8X8_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::BGRX8_UNorm_SRGB]          = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

		dxgiFormatMap[(int)TextureFormat::BC6H_Typeless]             = DXGI_FORMAT_BC6H_TYPELESS;
		dxgiFormatMap[(int)TextureFormat::BC6H_UF16]                 = DXGI_FORMAT_BC6H_UF16;
		dxgiFormatMap[(int)TextureFormat::BC6H_SF16]                 = DXGI_FORMAT_BC6H_SF16;
		dxgiFormatMap[(int)TextureFormat::BC7_Typeless]              = DXGI_FORMAT_BC7_TYPELESS ;
		dxgiFormatMap[(int)TextureFormat::BC7_UNorm]                 = DXGI_FORMAT_BC7_UNORM;
		dxgiFormatMap[(int)TextureFormat::BC7_UNorm_SRGB]            = DXGI_FORMAT_BC7_UNORM_SRGB;

		s_formatMapInitialized = true;
	}

	return dxgiFormatMap[(int)format];
}

DXGI_SAMPLE_DESC GetDXGISampleDesc(Render::SampleDesc sampleDesc)
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

D3D12_GRAPHICS_PIPELINE_STATE_DESC GetD3D12PipelineStateDesc(const Render::GraphicsPipelineStateDesc& desc)
{
	PE_ASSERT(
		desc.primitiveTopologyType != Render::TopologyType::LineStrip &&
		desc.primitiveTopologyType != Render::TopologyType::LineStripAdj &&
		desc.primitiveTopologyType != Render::TopologyType::TriangleList &&
		desc.primitiveTopologyType != Render::TopologyType::TriangleListAdj,
		"We don't support strips for now, check IBStripCutValue below");

	DXGI_FORMAT rtvFormats[8];
	for (int32_t i = 0; i < desc.numRenderTargets; ++i)
		rtvFormats[i] = GetDXGIFormat(desc.renderTargetFormats[i]);

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlends[8] = {};
	for (int32_t i = 0; i < _countof(renderTargetBlends); ++i)
		renderTargetBlends[i] = GetD3D12RenderTargetBlendDesc(desc.blendState.renderTargetBlendDescs[i]);

	std::vector<D3D12_INPUT_ELEMENT_DESC> d3d12InputElements = GetD3D12InputLayoutElements(desc.inputLayout);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12Desc = {
		.pRootSignature = D3D12RenderAPI::Get()->GetRootSignatureCache().GetOrCreateRootSignature(desc)->GetD3D12RootSignature(),
		.VS = GetD3D12ShaderBytecode(desc.vs),
		.PS = GetD3D12ShaderBytecode(desc.ps),
		.DS = {}, // We don't support this shader for now
		.HS = {}, // We don't support this shader for now
		.GS = {}, // We don't support this shader for now
		.BlendState = {
			.AlphaToCoverageEnable = desc.blendState.alphaToCoverageEnable,
			.IndependentBlendEnable = desc.blendState.independentBlendEnable,
			.RenderTarget = {*renderTargetBlends}
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
			.MultisampleEnable = desc.rasterizerState.antialiasedLineEnable, // Multisample enabled only when antialiased line is
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
			.pInputElementDescs = d3d12InputElements.data(),
			.NumElements = (UINT)d3d12InputElements.size()
		},
		.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED, // TODO: When using strips you have to pass index buffer format to PSO 0xFFFF for 16 bit and 0xFFFFFFFF for 32 bit indices, we don't support strips for now
		.PrimitiveTopologyType = GetD3D12PrimitiveTopologyType(desc.primitiveTopologyType),
		.NumRenderTargets = (UINT)desc.numRenderTargets,
		.RTVFormats = {*rtvFormats},
		.DSVFormat = GetDXGIFormat(desc.depthStencilFormat),
		.SampleDesc = GetDXGISampleDesc(desc.sampleDesc)
	};

	return d3d12Desc;
}

CD3DX12_SHADER_BYTECODE GetD3D12ShaderBytecode(Render::Shader* shader)
{
	PE_ASSERT(shader);
	auto compilerOutput = static_cast<D3D12Shader*>(shader)->GetCompilerOutput();
	return {compilerOutput.bytecode.data(), compilerOutput.bytecode.size()};
}

D3D12_FILL_MODE GetD3D12FillMode(Render::FillMode fillMode)
{
	switch (fillMode)
	{
	case Render::FillMode::Wireframe:
		return D3D12_FILL_MODE_WIREFRAME;
	case Render::FillMode::Solid:
		return D3D12_FILL_MODE_SOLID;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_CULL_MODE GetD3D12CullMode(Render::CullMode cullMode)
{
	switch (cullMode)
	{
	case Render::CullMode::None:
		return D3D12_CULL_MODE_NONE;
	case Render::CullMode::Front:
		return D3D12_CULL_MODE_FRONT;
	case Render::CullMode::Back:
		return D3D12_CULL_MODE_BACK;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_COMPARISON_FUNC GetD3D12ComparisionFunc(Render::ComparisionFunction comparisionFunction)
{
	switch (comparisionFunction)
	{
	case Render::ComparisionFunction::Never:
		return D3D12_COMPARISON_FUNC_NEVER;
	case Render::ComparisionFunction::Less:
		return D3D12_COMPARISON_FUNC_LESS;
	case Render::ComparisionFunction::Equal:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case Render::ComparisionFunction::LessEqual:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case Render::ComparisionFunction::Greater:
		return D3D12_COMPARISON_FUNC_GREATER;
	case Render::ComparisionFunction::NotEqual:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case Render::ComparisionFunction::GreaterEqual:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case Render::ComparisionFunction::Always:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_DEPTH_STENCILOP_DESC GetD3D12DepthStencilOpDesc(Render::DepthStencilOperationDesc depthStencilOperationDesc)
{
	return {
		.StencilFailOp = GetD3D12StencilOp(depthStencilOperationDesc.stencilFail),
		.StencilDepthFailOp = GetD3D12StencilOp(depthStencilOperationDesc.stencilDepthFail),
		.StencilPassOp = GetD3D12StencilOp(depthStencilOperationDesc.stencilPass),
		.StencilFunc = GetD3D12ComparisionFunc(depthStencilOperationDesc.stencilFunction)
	};
}

D3D12_STENCIL_OP GetD3D12StencilOp(Render::StencilOperation stencilOperation)
{
	switch (stencilOperation)
	{
	case Render::StencilOperation::Keep:
		return D3D12_STENCIL_OP_KEEP;
	case Render::StencilOperation::Zero:
		return D3D12_STENCIL_OP_ZERO;
	case Render::StencilOperation::Replace:
		return D3D12_STENCIL_OP_REPLACE;
	case Render::StencilOperation::IncrSat:
		return D3D12_STENCIL_OP_INCR_SAT;
	case Render::StencilOperation::DecrSat:
		return D3D12_STENCIL_OP_DECR_SAT;
	case Render::StencilOperation::Invert:
		return D3D12_STENCIL_OP_INVERT;
	case Render::StencilOperation::IncrWrap:
		return D3D12_STENCIL_OP_INCR;
	case Render::StencilOperation::DecrWrap:
		return D3D12_STENCIL_OP_DECR;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType(Render::TopologyType topologyType)
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

D3D12_RENDER_TARGET_BLEND_DESC GetD3D12RenderTargetBlendDesc(Render::RenderTargetBlendDesc renderTargetBlendDesc)
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

D3D12_BLEND GetD3D12Blend(Render::BlendFactor blendFactor)
{
	switch (blendFactor)
	{
	case Render::BlendFactor::Zero:
		return D3D12_BLEND_ZERO;
	case Render::BlendFactor::One:
		return D3D12_BLEND_ONE;
	case Render::BlendFactor::SrcColor:
		return D3D12_BLEND_SRC_COLOR;
	case Render::BlendFactor::InvSrcColor:
		return D3D12_BLEND_INV_SRC_COLOR;
	case Render::BlendFactor::SrcAlpha:
		return D3D12_BLEND_SRC_ALPHA;
	case Render::BlendFactor::InvSrcAlpha:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case Render::BlendFactor::DestAlpha:
		return D3D12_BLEND_DEST_ALPHA;
	case Render::BlendFactor::InvDestAlpha:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case Render::BlendFactor::DestColor:
		return D3D12_BLEND_DEST_COLOR;
	case Render::BlendFactor::InvDestColor:
		return D3D12_BLEND_INV_DEST_COLOR;
	case Render::BlendFactor::SrcAlphaSat:
		return D3D12_BLEND_SRC_ALPHA_SAT;
	case Render::BlendFactor::ConstantBlendFactor:
		return D3D12_BLEND_BLEND_FACTOR;
	case Render::BlendFactor::InvConstantBlendFactor:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	case Render::BlendFactor::Src1Color:
		return D3D12_BLEND_SRC1_COLOR;
	case Render::BlendFactor::InvSrc1Color:
		return D3D12_BLEND_INV_SRC1_COLOR;
	case Render::BlendFactor::Src1Alpha:
		return D3D12_BLEND_SRC1_ALPHA;
	case Render::BlendFactor::InvSrc1Alpha:
		return D3D12_BLEND_INV_SRC1_ALPHA;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_BLEND_OP GetD3D12BlendOp(Render::BlendOperation blendOperation)
{
	switch (blendOperation)
	{
	case Render::BlendOperation::Add:
		return D3D12_BLEND_OP_ADD;
	case Render::BlendOperation::Subtract:
		return D3D12_BLEND_OP_SUBTRACT;
	case Render::BlendOperation::RevSubtract:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case Render::BlendOperation::Min:
		return D3D12_BLEND_OP_MIN;
	case Render::BlendOperation::Max:
		return D3D12_BLEND_OP_MAX;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

D3D12_LOGIC_OP GetD3D12LogicOp(Render::LogicOperation logicOperation)
{
	switch (logicOperation)
	{
	case Render::LogicOperation::Clear:
		return D3D12_LOGIC_OP_CLEAR;
	case Render::LogicOperation::Set:
		return D3D12_LOGIC_OP_SET;
	case Render::LogicOperation::Copy:
		return D3D12_LOGIC_OP_COPY;
	case Render::LogicOperation::CopyInverted:
		return D3D12_LOGIC_OP_COPY_INVERTED;
	case Render::LogicOperation::Noop:
		return D3D12_LOGIC_OP_NOOP;
	case Render::LogicOperation::Invert:
		return D3D12_LOGIC_OP_INVERT;
	case Render::LogicOperation::And:
		return D3D12_LOGIC_OP_AND;
	case Render::LogicOperation::Nand:
		return D3D12_LOGIC_OP_NAND;
	case Render::LogicOperation::Or:
		return D3D12_LOGIC_OP_OR;
	case Render::LogicOperation::Nor:
		return D3D12_LOGIC_OP_NOR;
	case Render::LogicOperation::Xor:
		return D3D12_LOGIC_OP_XOR;
	case Render::LogicOperation::Equiv:
		return D3D12_LOGIC_OP_EQUIV;
	case Render::LogicOperation::AndReverse:
		return D3D12_LOGIC_OP_AND_REVERSE;
	case Render::LogicOperation::AndInverted:
		return D3D12_LOGIC_OP_AND_INVERTED;
	case Render::LogicOperation::OrReverse:
		return D3D12_LOGIC_OP_OR_REVERSE;
	case Render::LogicOperation::OrInverted:
		return D3D12_LOGIC_OP_OR_INVERTED;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}

std::vector<D3D12_INPUT_ELEMENT_DESC> GetD3D12InputLayoutElements(const std::vector<Render::LayoutElement>& layoutElements)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> d3d12InputElements;

	for (auto& element : layoutElements)
	{
		D3D12_INPUT_ELEMENT_DESC inputElement = {
			.SemanticName = element.semanticName,
			.SemanticIndex = (UINT)element.semanticIndex,
			.Format = GetD3D12InputElementDescFormat(element.valueType, element.componentsNum, element.isNormalized),
			.InputSlot = (UINT)element.bufferSlot,
			.AlignedByteOffset = (UINT)element.relativeByteOffset,
			.InputSlotClass = GetD3D12InputClassification(element.frequency),
			.InstanceDataStepRate = (UINT)element.instanceDataStepRate
		};

		d3d12InputElements.push_back(inputElement);
	}

	return d3d12InputElements;
}

DXGI_FORMAT GetD3D12InputElementDescFormat(Render::LayoutValueType valueType, int32_t componentsNum, bool isNormalized)
{
	switch (valueType)
	{
	case Render::LayoutValueType::Float16:
	{
		PE_ASSERT(!isNormalized, "Floating point formats cannot be normalized");
		switch (componentsNum)
		{
		case 1: return DXGI_FORMAT_R16_FLOAT;
		case 2: return DXGI_FORMAT_R16G16_FLOAT;
		case 4: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		default:
			PE_ASSERT_NO_ENTRY();
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	case Render::LayoutValueType::Float32:
	{
		PE_ASSERT(!isNormalized, "Floating point formats cannot be normalized");
		switch (componentsNum)
		{
		case 1: return DXGI_FORMAT_R32_FLOAT;
		case 2: return DXGI_FORMAT_R32G32_FLOAT;
		case 3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default:
			PE_ASSERT_NO_ENTRY();
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	case Render::LayoutValueType::Int32:
	{
		PE_ASSERT(!isNormalized, "32-bit UNORM formats are not supported. Use R32_FLOAT instead");
		switch (componentsNum)
		{
		case 1: return DXGI_FORMAT_R32_SINT;
		case 2: return DXGI_FORMAT_R32G32_SINT;
		case 3: return DXGI_FORMAT_R32G32B32_SINT;
		case 4: return DXGI_FORMAT_R32G32B32A32_SINT;
		default:
			PE_ASSERT_NO_ENTRY();
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	case Render::LayoutValueType::UInt32:
	{
		PE_ASSERT(!isNormalized, "32-bit UNORM formats are not supported. Use R32_FLOAT instead");
		switch (componentsNum)
		{
		case 1: return DXGI_FORMAT_R32_UINT;
		case 2: return DXGI_FORMAT_R32G32_UINT;
		case 3: return DXGI_FORMAT_R32G32B32_UINT;
		case 4: return DXGI_FORMAT_R32G32B32A32_UINT;
		default:
			PE_ASSERT_NO_ENTRY();
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	case Render::LayoutValueType::Int16:
	{
		if (isNormalized)
		{
			switch (componentsNum)
			{
			case 1: return DXGI_FORMAT_R16_SNORM;
			case 2: return DXGI_FORMAT_R16G16_SNORM;
			case 4: return DXGI_FORMAT_R16G16B16A16_SNORM;
			default:
				PE_ASSERT_NO_ENTRY();
				return DXGI_FORMAT_UNKNOWN;
			}
		}
		else
		{
			switch (componentsNum)
			{
			case 1: return DXGI_FORMAT_R16_SINT;
			case 2: return DXGI_FORMAT_R16G16_SINT;
			case 4: return DXGI_FORMAT_R16G16B16A16_SINT;
			default:
				PE_ASSERT_NO_ENTRY();
				return DXGI_FORMAT_UNKNOWN;
			}
		}
	}

	case Render::LayoutValueType::UInt16:
	{
		if (isNormalized)
		{
			switch (componentsNum)
			{
			case 1: return DXGI_FORMAT_R16_UNORM;
			case 2: return DXGI_FORMAT_R16G16_UNORM;
			case 4: return DXGI_FORMAT_R16G16B16A16_UNORM;
			default:
				PE_ASSERT_NO_ENTRY();
				return DXGI_FORMAT_UNKNOWN;
			}
		}
		else
		{
			switch (componentsNum)
			{
			case 1: return DXGI_FORMAT_R16_UINT;
			case 2: return DXGI_FORMAT_R16G16_UINT;
			case 4: return DXGI_FORMAT_R16G16B16A16_UINT;
			default:
				PE_ASSERT_NO_ENTRY();
				return DXGI_FORMAT_UNKNOWN;
			}
		}
	}

	case Render::LayoutValueType::Int8:
	{
		if (isNormalized)
		{
			switch (componentsNum)
			{
			case 1: return DXGI_FORMAT_R8_SNORM;
			case 2: return DXGI_FORMAT_R8G8_SNORM;
			case 4: return DXGI_FORMAT_R8G8B8A8_SNORM;
			default:
				PE_ASSERT_NO_ENTRY();
				return DXGI_FORMAT_UNKNOWN;
			}
		}
		else
		{
			switch (componentsNum)
			{
			case 1: return DXGI_FORMAT_R8_SINT;
			case 2: return DXGI_FORMAT_R8G8_SINT;
			case 4: return DXGI_FORMAT_R8G8B8A8_SINT;
			default:
				PE_ASSERT_NO_ENTRY();
				return DXGI_FORMAT_UNKNOWN;
			}
		}
	}

	case Render::LayoutValueType::UInt8:
	{
		if (isNormalized)
		{
			switch (componentsNum)
			{
			case 1: return DXGI_FORMAT_R8_UNORM;
			case 2: return DXGI_FORMAT_R8G8_UNORM;
			case 4: return DXGI_FORMAT_R8G8B8A8_UNORM;
			default:
				PE_ASSERT_NO_ENTRY();
				return DXGI_FORMAT_UNKNOWN;
			}
		}
		else
		{
			switch (componentsNum)
			{
			case 1: return DXGI_FORMAT_R8_UINT;
			case 2: return DXGI_FORMAT_R8G8_UINT;
			case 4: return DXGI_FORMAT_R8G8B8A8_UINT;
			default:
				PE_ASSERT_NO_ENTRY();
				return DXGI_FORMAT_UNKNOWN;
			}
		}
	}

	default:
		PE_ASSERT_NO_ENTRY();
		return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_INPUT_CLASSIFICATION GetD3D12InputClassification(Render::LayoutElementFrequency frequency)
{
	switch (frequency)
	{
	case Render::LayoutElementFrequency::PerVertex:
		return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	case Render::LayoutElementFrequency::PerInstance:
		return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}
}
}
