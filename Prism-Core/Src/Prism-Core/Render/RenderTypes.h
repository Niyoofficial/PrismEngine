#pragma once

namespace Prism::Render
{
enum class TextureFormat
{
    Unknown = 0,
    RGBA32_Typeless,
    RGBA32_Float,
    RGBA32_UInt,
    RGBA32_SInt,
    RGB32_Typeless,
    RGB32_Float,
    RGB32_UInt,
    RGB32_SInt,
    RGBA16_Typeless,
    RGBA16_Float,
    RGBA16_UNorm,
    RGBA16_UInt,
    RGBA16_SNorm,
    RGBA16_SInt,
    RG32_Typeless,
    RG32_Float,
    RG32_UInt,
    RG32_SInt,
    R32G8X24_Typeless,
    D32_Float_S8X24_UInt,
    R32_Float_X8X24_Typeless,
    X32_Typeless_G8X24_UInt,
    RGB10A2_Typeless,
    RGB10A2_UNorm,
    RGB10A2_UInt,
    R11G11B10_Float,
    RGBA8_Typeless,
    RGBA8_UNorm,
    RGBA8_UNorm_SRGB,
    RGBA8_UInt,
    RGBA8_SNorm,
    RGBA8_SInt,
    RG16_Typeless,
    RG16_Float,
    RG16_UNorm,
    RG16_UInt,
    RG16_SNorm,
    RG16_SInt,
    R32_Typeless,
    D32_Float,
    R32_Float,
    R32_UInt,
    R32_SInt,
    R24G8_Typeless,
    D24_UNorm_S8_UInt,
    R24_UNorm_X8_Typeless,
    X24_Typeless_G8_UInt,
    RG8_Typeless,
    RG8_UNorm,
    RG8_UInt,
    RG8_SNorm,
    RG8_SInt,
    R16_Typeless,
    R16_Float,
    D16_UNorm,
    R16_UNorm,
    R16_UInt,
    R16_SNorm,
    R16_SInt,
    R8_Typeless,
    R8_UNorm,
    R8_UInt,
    R8_SNorm,
    R8_SInt,
    A8_UNorm,
    R1_UNorm,
    RGB9E5_SHAREDEXP,
    RG8_B8G8_UNorm,
    G8R8_G8B8_UNorm,
    BC1_Typeless,
    BC1_UNorm,
    BC1_UNorm_SRGB,
    BC2_Typeless,
    BC2_UNorm,
    BC2_UNorm_SRGB,
    BC3_Typeless,
    BC3_UNorm,
    BC3_UNorm_SRGB,
    BC4_Typeless,
    BC4_UNorm,
    BC4_SNorm,
    BC5_Typeless,
    BC5_UNorm,
    BC5_SNorm,
    B5G6R5_UNorm,
    B5G5R5A1_UNorm,
    BGRA8_UNorm,
    BGRX8_UNorm,
    R10G10B10_XR_BIAS_A2_UNorm,
    BGRA8_Typeless,
    BGRA8_UNorm_SRGB,
    BGRX8_Typeless,
    BGRX8_UNorm_SRGB,
    BC6H_Typeless,
    BC6H_UF16,
    BC6H_SF16,
    BC7_Typeless,
    BC7_UNorm,
    BC7_UNorm_SRGB,

    NumFormats
};

enum class BlendFactor
{
	// Undefined blend factor
	Undefined = 0,

	// The blend factor is zero.
	// Direct3D counterpart: D3D11_BLEND_ZERO/D3D12_BLEND_ZERO. OpenGL counterpart: GL_ZERO.
	Zero,

	// The blend factor is one.
	// Direct3D counterpart: D3D11_BLEND_ONE/D3D12_BLEND_ONE. OpenGL counterpart: GL_ONE.
	One,

	// The blend factor is RGB data from a pixel shader.
	// Direct3D counterpart: D3D11_BLEND_SRC_COLOR/D3D12_BLEND_SRC_COLOR. OpenGL counterpart: GL_SRC_COLOR.
	SrcColor,

	// The blend factor is 1-RGB, where RGB is the data from a pixel shader.
	// Direct3D counterpart: D3D11_BLEND_INV_SRC_COLOR/D3D12_BLEND_INV_SRC_COLOR. OpenGL counterpart: GL_ONE_MINUS_SRC_COLOR.
	InvSrcColor,

	// The blend factor is alpha (A) data from a pixel shader.
	// Direct3D counterpart: D3D11_BLEND_SRC_ALPHA/D3D12_BLEND_SRC_ALPHA. OpenGL counterpart: GL_SRC_ALPHA.
	SrcAlpha,

	// The blend factor is 1-A, where A is alpha data from a pixel shader.
	// Direct3D counterpart: D3D11_BLEND_INV_SRC_ALPHA/D3D12_BLEND_INV_SRC_ALPHA. OpenGL counterpart: GL_ONE_MINUS_SRC_ALPHA.
	InvSrcAlpha,

	// The blend factor is alpha (A) data from a render target.
	// Direct3D counterpart: D3D11_BLEND_DEST_ALPHA/D3D12_BLEND_DEST_ALPHA. OpenGL counterpart: GL_DST_ALPHA.
	DestAlpha,

	// The blend factor is 1-A, where A is alpha data from a render target.
	// Direct3D counterpart: D3D11_BLEND_INV_DEST_ALPHA/D3D12_BLEND_INV_DEST_ALPHA. OpenGL counterpart: GL_ONE_MINUS_DST_ALPHA.
	InvDestAlpha,

	// The blend factor is RGB data from a render target.
	// Direct3D counterpart: D3D11_BLEND_DEST_COLOR/D3D12_BLEND_DEST_COLOR. OpenGL counterpart: GL_DST_COLOR.
	DestColor,

	// The blend factor is 1-RGB, where RGB is the data from a render target.
	// Direct3D counterpart: D3D11_BLEND_INV_DEST_COLOR/D3D12_BLEND_INV_DEST_COLOR. OpenGL counterpart: GL_ONE_MINUS_DST_COLOR.
	InvDestColor,

	// The blend factor is (f,f,f,1), where f = min(As, 1-Ad),
	// As is alpha data from a pixel shader, and Ad is alpha data from a render target.
	// Direct3D counterpart: D3D11_BLEND_SRC_ALPHA_SAT/D3D12_BLEND_SRC_ALPHA_SAT. OpenGL counterpart: GL_SRC_ALPHA_SATURATE.
	SrcAlphaSat,

	// The blend factor is the constant blend factor set with IDeviceContext::SetBlendFactors().
	// Direct3D counterpart: D3D11_BLEND_BLEND_FACTOR/D3D12_BLEND_BLEND_FACTOR. OpenGL counterpart: GL_CONSTANT_COLOR.
	ConstantBlendFactor,

	// The blend factor is one minus constant blend factor set with IDeviceContext::SetBlendFactors().
	// Direct3D counterpart: D3D11_BLEND_INV_BLEND_FACTOR/D3D12_BLEND_INV_BLEND_FACTOR. OpenGL counterpart: GL_ONE_MINUS_CONSTANT_COLOR.
	InvConstantBlendFactor,

	// The blend factor is the second RGB data output from a pixel shader.
	// Direct3D counterpart: D3D11_BLEND_SRC1_COLOR/D3D12_BLEND_SRC1_COLOR. OpenGL counterpart: GL_SRC1_COLOR.
	Src1Color,

	// The blend factor is 1-RGB, where RGB is the second RGB data output from a pixel shader.
	// Direct3D counterpart: D3D11_BLEND_INV_SRC1_COLOR/D3D12_BLEND_INV_SRC1_COLOR. OpenGL counterpart: GL_ONE_MINUS_SRC1_COLOR.
	InvSrc1Color,

	// The blend factor is the second alpha (A) data output from a pixel shader.
	// Direct3D counterpart: D3D11_BLEND_SRC1_ALPHA/D3D12_BLEND_SRC1_ALPHA. OpenGL counterpart: GL_SRC1_ALPHA.
	Src1Alpha,

	// The blend factor is 1-A, where A is the second alpha data output from a pixel shader.
	// Direct3D counterpart: D3D11_BLEND_INV_SRC1_ALPHA/D3D12_BLEND_INV_SRC1_ALPHA. OpenGL counterpart: GL_ONE_MINUS_SRC1_ALPHA.
	InvSrc1Alpha,

	// Helper value that stores the total number of blend factors in the enumeration.
	NumFactors
};

enum class BlendOperation
{
	// Undefined blend operation
	Undefined = 0,

	// Add source and destination color components.
	// Direct3D counterpart: D3D11_BLEND_OP_ADD/D3D12_BLEND_OP_ADD. OpenGL counterpart: GL_FUNC_ADD.
	Add,

	// Subtract destination color components from source color components.
	// Direct3D counterpart: D3D11_BLEND_OP_SUBTRACT/D3D12_BLEND_OP_SUBTRACT. OpenGL counterpart: GL_FUNC_SUBTRACT.
	Subtract,

	// Subtract source color components from destination color components.
	// Direct3D counterpart: D3D11_BLEND_OP_REV_SUBTRACT/D3D12_BLEND_OP_REV_SUBTRACT. OpenGL counterpart: GL_FUNC_REVERSE_SUBTRACT.
	RevSubtract,

	// Compute the minimum of source and destination color components.
	// Direct3D counterpart: D3D11_BLEND_OP_MIN/D3D12_BLEND_OP_MIN. OpenGL counterpart: GL_MIN.
	Min,

	// Compute the maximum of source and destination color components.
	// Direct3D counterpart: D3D11_BLEND_OP_MAX/D3D12_BLEND_OP_MAX. OpenGL counterpart: GL_MAX.
	Max,

	// Helper value that stores the total number of blend operations in the enumeration.
	NumOperations
};

enum class LogicOperation
{
	// Clear the render target.
	// Direct3D12 counterpart: D3D12_CLEAR.
	Clear = 0,

	// Set the render target.
	// Direct3D12 counterpart: D3D12_SET.
	Set,

	// Copy the render target.
	// Direct3D12 counterpart: D3D12_COPY.
	Copy,

	// Perform an inverted-copy of the render target.
	// Direct3D12 counterpart: D3D12_COPY_INVERTED.
	CopyInverted,

	// No operation is performed on the render target.
	// Direct3D12 counterpart: D3D12_NOOP.
	Noop,

	// Invert the render target.
	// Direct3D12 counterpart: D3D12_INVERT.
	Invert,

	// Perform a logical AND operation on the render target.
	// Direct3D12 counterpart: D3D12_AND.
	And,

	// Perform a logical NAND operation on the render target.
	// Direct3D12 counterpart: D3D12_NAND.
	Nand,

	// Perform a logical OR operation on the render target.
	// Direct3D12 counterpart: D3D12_OR.
	Or,

	// Perform a logical NOR operation on the render target.
	// Direct3D12 counterpart: D3D12_NOR.
	Nor,

	// Perform a logical XOR operation on the render target.
	// Direct3D12 counterpart: D3D12_XOR.
	Xor,

	// Perform a logical equal operation on the render target.
	// Direct3D12 counterpart: D3D12_EQUIV.
	Equiv,

	// Perform a logical AND and reverse operation on the render target.
	// Direct3D12 counterpart: D3D12_AND_REVERSE.
	AndReverse,

	// Perform a logical AND and invert operation on the render target.
	// Direct3D12 counterpart: D3D12_AND_INVERTED.
	AndInverted,

	// Perform a logical OR and reverse operation on the render target.
	// Direct3D12 counterpart: D3D12_OR_REVERSE.
	OrReverse,

	// Perform a logical OR and invert operation on the render target.
	// Direct3D12 counterpart: D3D12_OR_INVERTED.
	OrInverted,

	// Helper value that stores the total number of logical operations in the enumeration.
	NumOperations
};

enum class ColorMask : uint8_t
{
	/// Do not store any components.
	None = 0u,

	/// Allow data to be stored in the red component.
	Red = 1u << 0u,

	/// Allow data to be stored in the green component.
	Green = 1u << 1u,

	/// Allow data to be stored in the blue component.
	Blue = 1u << 2u,

	/// Allow data to be stored in the alpha component.
	Alpha = 1u << 3u,

	/// Allow data to be stored in all RGB components.
	RGB = Red | Green | Blue,

	/// Allow data to be stored in all components.
	All = (RGB | Alpha)
};

struct RenderTargetBlendDesc
{
	bool blendEnable = false;
	bool logicOperationEnable = false;
	BlendFactor srcBlend;
	BlendFactor destBlend;
	BlendOperation blendOperation;
	BlendFactor srcBlendAlpha;
	BlendFactor destBlendAlpha;
	BlendOperation blendOperationAlpha;
	LogicOperation logicOperation;
	ColorMask renderTargetWriteMask = ColorMask::All;
};

struct BlendStateDesc
{
	bool alphaToCoverageEnable = false;
	bool independentBlendEnable = false;
	RenderTargetBlendDesc renderTargetBlendDescs[8] = {};
};

enum class FillMode
{
	Undefined = 0,
	Wireframe,
	Solid,
	NumFillModes
};

enum class CullMode
{
	Undefined = 0,
	None,
	Front,
	Back,
	NumCullModes
};

struct RasterizerStateDesc
{
	FillMode fillMode;
	CullMode cullMode;
	bool frontCounterClockwise = false;
	int32_t depthBias = 0;
	float depthBiasClamp = 0.f;
	float slopeScaledDepthBias = 0.f;
	bool depthClipEnable = true;
	bool antialiasedLineEnable = false;
	int32_t forcedSampleCount = 0;
};

enum class ComparisionFunction
{
	// Unknown comparison function
	Unknown = 0,

	// Comparison never passes. 
	// Direct3D counterpart: D3D11_COMPARISON_NEVER/D3D12_NEVER. OpenGL counterpart: GL_NEVER.
	Never,

	// Comparison passes if the source data is less than the destination data.
	// Direct3D counterpart: D3D11_COMPARISON_LESS/D3D12_LESS. OpenGL counterpart: GL_LESS.
	Less,

	// Comparison passes if the source data is equal to the destination data.
	// Direct3D counterpart: D3D11_COMPARISON_EQUAL/D3D12_EQUAL. OpenGL counterpart: GL_EQUAL.
	Equal,

	// Comparison passes if the source data is less than or equal to the destination data.
	// Direct3D counterpart: D3D11_COMPARISON_LESS_EQUAL/D3D12_LESS_EQUAL. OpenGL counterpart: GL_LEQUAL.
	LessEqual,

	// Comparison passes if the source data is greater than the destination data.
	// Direct3D counterpart: 3D11_COMPARISON_GREATER/D3D12_GREATER. OpenGL counterpart: GL_GREATER.
	Greater,

	// Comparison passes if the source data is not equal to the destination data.
	// Direct3D counterpart: D3D11_COMPARISON_NOT_EQUAL/D3D12_NOT_EQUAL. OpenGL counterpart: GL_NOTEQUAL.
	NotEqual,

	// Comparison passes if the source data is greater than or equal to the destination data.
	// Direct3D counterpart: D3D11_COMPARISON_GREATER_EQUAL/D3D12_GREATER_EQUAL. OpenGL counterpart: GL_GEQUAL.
	GreaterEqual,

	// Comparison always passes. 
	// Direct3D counterpart: D3D11_COMPARISON_ALWAYS/D3D12_ALWAYS. OpenGL counterpart: GL_ALWAYS.
	Always,

	// Helper value that stores the total number of comparison functions in the enumeration
	NumFunctions
};

enum class StencilOperation
{
	// Undefined operation.
	Undefined = 0,

	// Keep the existing stencil data.
	// Direct3D counterpart: D3D11_KEEP/D3D12_KEEP. OpenGL counterpart: GL_KEEP.
	Keep,

	// Set the stencil data to 0.
	// Direct3D counterpart: D3D11_ZERO/D3D12_ZERO. OpenGL counterpart: GL_ZERO.
	Zero,

	// Set the stencil data to the reference value set by calling IDeviceContext::SetStencilRef().
	// Direct3D counterpart: D3D11_REPLACE/D3D12_REPLACE. OpenGL counterpart: GL_REPLACE.
	Replace,

	// Increment the current stencil value, and clamp to the maximum representable unsigned value.
	// Direct3D counterpart: D3D11_INCR_SAT/D3D12_INCR_SAT. OpenGL counterpart: GL_INCR.
	IncrSat,

	// Decrement the current stencil value, and clamp to 0.
	// Direct3D counterpart: D3D11_DECR_SAT/D3D12_DECR_SAT. OpenGL counterpart: GL_DECR.
	DecrSat,

	// Bitwise invert the current stencil buffer value.
	// Direct3D counterpart: D3D11_INVERT/D3D12_INVERT. OpenGL counterpart: GL_INVERT.
	Invert,

	// Increment the current stencil value, and wrap the value to zero when incrementing
	// the maximum representable unsigned value. 
	// Direct3D counterpart: D3D11_INCR/D3D12_INCR. OpenGL counterpart: GL_INCR_WRAP.
	IncrWrap,

	// Decrement the current stencil value, and wrap the value to the maximum representable
	// unsigned value when decrementing a value of zero.
	// Direct3D counterpart: D3D11_DECR/D3D12_DECR. OpenGL counterpart: GL_DECR_WRAP.
	DecrWrap,

	// Helper value that stores the total number of stencil operations in the enumeration.
	NumOperations
};

struct DepthStencilOperationDesc
{
	StencilOperation stencilFail;
	StencilOperation stencilDepthFail;
	StencilOperation stencilPass;
	ComparisionFunction stencilFunction;
};

struct DepthStencilStateDesc
{
	bool depthEnable = true;
	bool depthWriteEnable = true;
	ComparisionFunction depthFunc;

	bool stencilEnable = false;
	uint8_t stencilReadMask = 0;
	uint8_t stencilWriteMask = 0;
	DepthStencilOperationDesc frontFace;
	DepthStencilOperationDesc backFace;
};

enum class LayoutValueType
{
	// Undefined type
	Undefined = 0,
	// Signed 8-bit integer
	Int8,
	// Signed 16-bit integer
	Int16,
	// Signed 32-bit integer
	Int32,
	// Unsigned 8-bit integer
	UInt8,
	// Unsigned 16-bit integer
	UInt16,
	// Unsigned 32-bit integer
	UInt32,
	// Half-precision 16-bit floating point
	Float16,
	// Full-precision 32-bit floating point
	Float32,
	// Double-precision 64-bit floating point
	Float64,
	// Helper value storing total number of types in the enumeration
	NumValueTypes
};

enum class LayoutElementFrequency
{
	Undefined,
	PerVertex,
	PerInstance,
	NumFrequencies
};

constexpr int32_t LAYOUT_ELEMENT_AUTO_BYTE_OFFSET = -1;

struct LayoutElement
{
	const char* semanticName = nullptr;
	int32_t semanticIndex = -1;
	int32_t bufferSlot = -1;
	LayoutValueType valueType;
	int32_t componentsNum = -1;
	bool isNormalized = false;
	// Use LAYOUT_ELEMENT_AUTO_BYTE_OFFSET for automatic byte offset
	int32_t relativeByteOffset = LAYOUT_ELEMENT_AUTO_BYTE_OFFSET;
	LayoutElementFrequency frequency;
	int32_t instanceDataStepRate = -1;
};

enum class TopologyType
{
	// Undefined topology
	Undefined = 0,

	// Interpret the vertex data as a list of triangles.
	// D3D counterpart: D3D_TRIANGLELIST. OpenGL counterpart: GL_TRIANGLES.
	TriangleList,

	// Interpret the vertex data as a triangle strip.
	// D3D counterpart: D3D_TRIANGLESTRIP. OpenGL counterpart: GL_TRIANGLE_STRIP.
	TriangleStrip,

	// Interpret the vertex data as a list of points.
	// D3D counterpart: D3D_POINTLIST. OpenGL counterpart: GL_POINTS.
	PointList,

	// Interpret the vertex data as a list of lines.
	// D3D counterpart: D3D_LINELIST. OpenGL counterpart: GL_LINES.
	LineList,

	// Interpret the vertex data as a line strip.
	// D3D counterpart: D3D_LINESTRIP. OpenGL counterpart: GL_LINE_STRIP.
	LineStrip,

	// Interpret the vertex data as a list of triangles with adjacency data.
	// D3D counterpart: D3D_TRIANGLELIST_ADJ. OpenGL counterpart: GL_TRIANGLES_ADJACENCY.
	TriangleListAdj,

	// Interpret the vertex data as a triangle strip with adjacency data.
	// D3D counterpart: D3D_TRIANGLESTRIP_ADJ. OpenGL counterpart: GL_TRIANGLE_STRIP_ADJACENCY.
	TriangleStripAdj,

	// Interpret the vertex data as a list of lines with adjacency data.
	// D3D counterpart: D3D_LINELIST_ADJ. OpenGL counterpart: GL_LINES_ADJACENCY.
	LineListAdj,

	// Interpret the vertex data as a line strip with adjacency data.
	// D3D counterpart: D3D_LINESTRIP_ADJ. OpenGL counterpart: GL_LINE_STRIP_ADJACENCY.
	LineStripAdj,

	// Interpret the vertex data as a list of one control point patches.
	// D3D counterpart: D3D_PRIMITIVE_TOPOLOGY_N_CONTROL_POINT_PATCHLIST. OpenGL counterpart: GL_PATCHES.
	ControlPointPatchlist1,
	ControlPointPatchlist2,
	ControlPointPatchlist3,
	ControlPointPatchlist4,
	ControlPointPatchlist5,
	ControlPointPatchlist6,
	ControlPointPatchlist7,
	ControlPointPatchlist8,
	ControlPointPatchlist9,
	ControlPointPatchlist10,
	ControlPointPatchlist11,
	ControlPointPatchlist12,
	ControlPointPatchlist13,
	ControlPointPatchlist14,
	ControlPointPatchlist15,
	ControlPointPatchlist16,
	ControlPointPatchlist17,
	ControlPointPatchlist18,
	ControlPointPatchlist19,
	ControlPointPatchlist20,
	ControlPointPatchlist21,
	ControlPointPatchlist22,
	ControlPointPatchlist23,
	ControlPointPatchlist24,
	ControlPointPatchlist25,
	ControlPointPatchlist26,
	ControlPointPatchlist27,
	ControlPointPatchlist28,
	ControlPointPatchlist29,
	ControlPointPatchlist30,
	ControlPointPatchlist31,
	ControlPointPatchlist32,

	NumTopologyTypes
};

struct SampleDesc
{
	int32_t count = 1;
	int32_t quality = 0;
};
}