#pragma once
#include <variant>
#include <array>

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

enum class TextureModifier
{
	Float,
	UInt,
	SInt,
	UNorm,
	SNorm,
	Typeless
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
	bool operator==(const RenderTargetBlendDesc& other) const;

	bool blendEnable = false;
	bool logicOperationEnable = false;
	BlendFactor srcBlend = BlendFactor::One;
	BlendFactor destBlend = BlendFactor::Zero;
	BlendOperation blendOperation = BlendOperation::Add;
	BlendFactor srcBlendAlpha = BlendFactor::One;
	BlendFactor destBlendAlpha = BlendFactor::Zero;
	BlendOperation blendOperationAlpha = BlendOperation::Add;
	LogicOperation logicOperation = LogicOperation::Noop;
	ColorMask renderTargetWriteMask = ColorMask::All;
};

struct BlendStateDesc
{
	bool operator==(const BlendStateDesc& other) const;

	bool alphaToCoverageEnable = false;
	bool independentBlendEnable = false;
	std::array<RenderTargetBlendDesc, 8> renderTargetBlendDescs = {};
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
	bool operator==(const RasterizerStateDesc& other) const;

	FillMode fillMode = FillMode::Solid;
	CullMode cullMode = CullMode::Back;
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
	bool operator==(const DepthStencilOperationDesc& other) const;

	StencilOperation stencilFail = StencilOperation::Keep;
	StencilOperation stencilDepthFail = StencilOperation::Keep;
	StencilOperation stencilPass = StencilOperation::Keep;
	ComparisionFunction stencilFunction = ComparisionFunction::Always;
};

struct DepthStencilStateDesc
{
	bool operator==(const DepthStencilStateDesc& other) const;

	bool depthEnable = true;
	bool depthWriteEnable = true;
	ComparisionFunction depthFunc = ComparisionFunction::Less;

	bool stencilEnable = false;
	uint8_t stencilReadMask = 0;
	uint8_t stencilWriteMask = 0;
	DepthStencilOperationDesc frontFace;
	DepthStencilOperationDesc backFace;
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
	bool operator==(const SampleDesc& other) const;

	int32_t count = 1;
	int32_t quality = 0;
};

enum class BindFlags : uint16_t
{
	// Undefined binding.
	None = 0,

	// A buffer can be bound as a vertex buffer.
	VertexBuffer = 1u << 0u,

	// A buffer can be bound as an index buffer.
	IndexBuffer = 1u << 1u,

	// A buffer can be bound as a constant (uniform) buffer.
	//
	// This flag may NOT be combined with any other bind flag.
	ConstantBuffer = 1u << 2u,

	// A buffer or a texture can be bound as a shader resource.
	ShaderResource = 1u << 3u,

	// A buffer can be bound as a target for stream output stage.
	StreamOutput = 1u << 4u,

	// A texture can be bound as a render target.
	RenderTarget = 1u << 5u,

	// A texture can be bound as a depth-stencil target.
	DepthStencil = 1u << 6u,

	// A buffer or a texture can be bound as an unordered access view.
	UnorderedAccess = 1u << 7u,

	// A buffer can be bound as the source buffer for indirect draw commands.
	IndirectDrawArgs = 1u << 8u,
};

enum class CPUAccess
{
	None = 0,
	Read = 1 << 0,
	Write = 1 << 1
};

enum class ResourceUsage
{
	// A resource that requires read and write access by the GPU and can also be occasionally
	// written by the CPU.
	// D3D11 Counterpart: D3D11_USAGE_DEFAULT. OpenGL counterpart: GL_DYNAMIC_DRAW.
	// Default buffers do not allow CPU access and must use CPUAccess::None flag.
	Default = 0,

	// A resource that can be read by the GPU and written at least once per frame by the CPU.
	// D3D11 Counterpart: D3D11_USAGE_DYNAMIC. OpenGL counterpart: GL_STREAM_DRAW
	// Dynamic buffers must use CPUAccess::Write flag.
	Dynamic,

	// A resource that facilitates transferring data between GPU and CPU.
	// D3D11 Counterpart: D3D11_USAGE_STAGING. OpenGL counterpart: GL_STATIC_READ or
	// GL_STATIC_COPY depending on the CPU access flags.
	// Staging buffers must use exactly one of CPUAccess::Write or CPUAccess::Read flags.
	Staging
};

enum class ResourceStateFlags : uint32_t
{
	Unknown = 0,

	// The resource state is known to the engine, but is undefined. A resource is typically in an undefined state right after initialization.
	Undefined = 1u << 0,

	// The resource is accessed as a vertex buffer
	// Supported contexts: graphics.
	VertexBuffer = 1u << 1,

	// The resource is accessed as a uniform (constant) buffer
	// Supported contexts: graphics, compute.
	ConstantBuffer = 1u << 2,

	// The resource is accessed as an index buffer
	// Supported contexts: graphics.
	IndexBuffer = 1u << 3,

	// The resource is accessed as a render target
	// Supported contexts: graphics.
	RenderTarget = 1u << 4,

	// The resource is used for unordered access
	// Supported contexts: graphics, compute.
	UnorderedAccess = 1u << 5,

	// The resource is used in a writable depth-stencil view or in clear operation
	// Supported contexts: graphics.
	DepthWrite = 1u << 6,

	// The resource is used in a read-only depth-stencil view
	// Supported contexts: graphics.
	DepthRead = 1u << 7,

	// The resource is accessed from a shader
	// Supported contexts: graphics, compute.
	ShaderResource = 1u << 8,

	// The resource is used as the destination for stream output
	StreamOut = 1u << 9,

	// The resource is used as an indirect draw/dispatch arguments buffer
	// Supported contexts: graphics, compute.
	IndirectArgument = 1u << 10,

	// The resource is used as the destination in a copy operation
	// Supported contexts: graphics, compute, transfer.
	CopyDest = 1u << 11,

	// The resource is used as the source in a copy operation
	// Supported contexts: graphics, compute, transfer.
	CopySource = 1u << 12,

	// The resource is used as the destination in a resolve operation
	// Supported contexts: graphics.
	ResolveDest = 1u << 13,

	// The resource is used as the source in a resolve operation
	// Supported contexts: graphics.
	ResolveSource = 1u << 14,

	// The resource is used as an input attachment in a render pass subpass
	// Supported contexts: graphics.
	InputAttachment = 1u << 15,

	// The resource is used for present
	// Supported contexts: graphics.
	Present = 1u << 16,

	// The resource state is used for read operations, but access to the resource may be slower compared to the specialized state.
	// A transition to the COMMON state is always a pipeline stall and can often induce a cache flush and render target decompress operation.
	// In D3D12 backend, a resource must be in COMMON state for transition between graphics/compute queue and copy queue.
	// Supported contexts: graphics, compute, transfer.
	Common = 1u << 17,

	NumStates = 18,
	MaxBit = Common,

	GenericRead = VertexBuffer |
				  ConstantBuffer |
				  IndexBuffer |
				  ShaderResource |
				  IndirectArgument |
				  CopySource
};

enum class ResourceDimension
{
	Undefined = 0,	// Texture type undefined
	Buffer,			// Buffer
	Tex1D,			// One-dimensional texture
	Tex2D,			// Two-dimensional texture
	Tex3D,			// Three-dimensional texture
	TexCube,		// Cube-map texture
	NumDimensions
};

struct RenderTargetClearValue
{
	TextureFormat format;
	glm::float4 color;
};

struct DepthStencilValue
{
	float depth = 1.f;
	uint8_t stencil = 0;
};

struct DepthStencilClearValue
{
	TextureFormat format;
	DepthStencilValue depthStencil;
};

using ClearValue = std::variant<RenderTargetClearValue, DepthStencilClearValue>;

struct Viewport
{
	glm::float2 topLeft;
	glm::float2 size;
	glm::float2 depthRange;
};

struct Scissor
{
	glm::int2 topLeft;
	glm::int2 size;
};

enum class ClearFlags
{
	ClearDepth = 1 << 0,
	ClearStencil = 1 << 1
};

struct SubresourceRange
{
	int32_t firstMipLevel = 0;
	// -1 indicates all mip levels
	int32_t numMipLevels = -1;
	int32_t firstArraySlice = 0;
	int32_t numArraySlices = 1;
};

enum class BarrierSync
{
	None = 0,
	All = 1 << 0,
	Draw = 1 << 1,
	IndexInput = 1 << 2,
	VertexShading = 1 << 3,
	PixelShading = 1 << 4,
	DepthStencil = 1 << 5,
	RenderTarget = 1 << 6,
	ComputeShading = 1 << 7,
	Raytracing = 1 << 8,
	Copy = 1 << 9,
	Resolve = 1 << 10,
	ExecuteIndirect = 1 << 11,
	Predication = 1 << 11, // Duplicate of ExecuteIndirect
	AllShading = 1 << 12,
	NonPixelShading = 1 << 13,
	EmitRaytracingAccelerationStructurePostbuildInfo = 1 << 14,
	ClearUnorderedAccessView = 1 << 15,
	VideoDecode = 1 << 20,
	VideoProcess = 1 << 21,
	VideoEncode = 1 << 22,
	BuildRaytracingAccelerationStructure = 1 << 23,
	CopyRaytracingAccelerationStructure = 1 << 24,
	Split = 1 << 31
};

enum class BarrierAccess
{
	Common = 0,
	VertexBuffer = 1 << 0,
	ConstantBuffer = 1 << 1,
	IndexBuffer = 1 << 2,
	RenderTarget = 1 << 3,
	UnorderedAccess = 1 << 4,
	DepthStencilWrite = 1 << 5,
	DepthStencilRead = 1 << 6,
	ShaderResource = 1 << 7,
	StreamOutput = 1 << 8,
	IndirectArgument = 1 << 9,
	Predication = 1 << 9, // Duplicate of IndirectArgument
	CopyDest = 1 << 10,
	CopySource = 1 << 11,
	ResolveDest = 1 << 12,
	ResolveSource = 1 << 13,
	RaytracingAccelerationStructureRead = 1 << 14,
	RaytracingAccelerationStructureWrite = 1 << 15,
	ShadingRateSource = 1 << 16,
	VideoDecodeRead = 1 << 17,
	VideoDecodeWrite = 1 << 18,
	VideoProcessRead = 1 << 19,
	VideoProcessWrite = 1 << 20,
	VideoEncodeRead = 1 << 21,
	VideoEncodeWrite = 1 << 22,
	NoAccess = 1 << 31
};

enum class BarrierLayout
{
	Undefined = -1,
	Common = 0,
	Present = 0,
	GenericRead = 1,
	RenderTarget = 2,
	UnorderedAccess = 3,
	DepthStencilWrite = 4,
	DepthStencilRead = 5,
	ShaderResource = 6,
	CopySource = 7,
	CopyDest = 8,
	ResolveSource = 9,
	ResolveDest = 10,
	ShadingRateSource = 11,
	VideoDecodeRead = 12,
	VideoDecodeWrite = 13,
	VideoProcessRead = 14,
	VideoProcessWrite = 15,
	VideoEncodeRead = 16,
	VideoEncodeWrite = 17,
	DirectQueueCommon = 18,
	DirectQueueGenericRead = 19,
	DirectQueueUnorderedAccess = 20,
	DirectQueueShaderResource = 21,
	DirectQueueCopySource = 22,
	DirectQueueCopyDest = 23,
	ComputeQueueCommon = 24,
	ComputeQueueGenericRead = 25,
	ComputeQueueUnorderedAccess = 26,
	ComputeQueueShaderResource = 27,
	ComputeQueueCopySource = 28,
	ComputeQueueCopyDest = 29,
	VideoQueueCommon = 30
};

struct BufferBarrier
{
	class Buffer* buffer = nullptr;

	Flags<BarrierSync> syncBefore;
	Flags<BarrierSync> syncAfter;
	Flags<BarrierAccess> accessBefore;
	Flags<BarrierAccess> accessAfter;

	int64_t offset = 0;
	// size set to -1 means the barrier will affect the buffer from offset to the end
	int64_t size = -1;
};

enum class TextureBarrierFlags
{
	None = 0,
	Discard = 1 << 0
};

struct TextureBarrier
{
	class Texture* texture = nullptr;

	Flags<BarrierSync> syncBefore;
	Flags<BarrierSync> syncAfter;
	Flags<BarrierAccess> accessBefore;
	Flags<BarrierAccess> accessAfter;
	BarrierLayout layoutBefore;
	BarrierLayout layoutAfter;

	// Leave default for all subresources
	SubresourceRange subresourceRange;

	Flags<TextureBarrierFlags> flags;
};

struct StateTransitionDesc
{
	class RenderResource* resource = nullptr;

	Flags<ResourceStateFlags> oldState;
	Flags<ResourceStateFlags> newState;
};

struct RawData
{
	const void* data = nullptr;
	int64_t sizeInBytes = 0;
};

struct Box
{
	glm::int3 location = {};
	glm::int3 size = {-1, -1, -1};
};

enum class PipelineStateType
{
	Graphics,
	Compute
};
}

template<>
struct std::hash<Prism::Render::RenderTargetBlendDesc>
{
	size_t operator()(const Prism::Render::RenderTargetBlendDesc& desc) const noexcept
	{
		using namespace Prism::Render;

		static_assert(sizeof(desc) == 36,
					  "If new field was added, add it to the hash function and update this assert");

		return
			std::hash<bool>()(desc.blendEnable) ^
			std::hash<bool>()(desc.logicOperationEnable) ^
			std::hash<BlendFactor>()(desc.srcBlend) ^
			std::hash<BlendFactor>()(desc.destBlend) ^
			std::hash<BlendOperation>()(desc.blendOperation) ^
			std::hash<BlendFactor>()(desc.srcBlendAlpha) ^
			std::hash<BlendFactor>()(desc.destBlendAlpha) ^
			std::hash<BlendOperation>()(desc.blendOperationAlpha) ^
			std::hash<LogicOperation>()(desc.logicOperation) ^
			std::hash<ColorMask>()(desc.renderTargetWriteMask);
	}
};

template<>
struct std::hash<Prism::Render::BlendStateDesc>
{
	size_t operator()(const Prism::Render::BlendStateDesc& desc) const noexcept
	{
		using namespace Prism::Render;

		static_assert(sizeof(BlendStateDesc) == 292,
			"If new field was added, add it to the hash function and update this assert");

		return
			std::hash<bool>()(desc.alphaToCoverageEnable) ^
			std::hash<bool>()(desc.independentBlendEnable) ^
			std::hash<std::array<RenderTargetBlendDesc, 8>>()(desc.renderTargetBlendDescs);
	}
};

template<>
struct std::hash<Prism::Render::RasterizerStateDesc>
{
	size_t operator()(const Prism::Render::RasterizerStateDesc& desc) const noexcept
	{
		using namespace Prism::Render;

		static_assert(sizeof(desc) == 32,
			"If new field was added, add it to the hash function and update this assert");

		return
			std::hash<FillMode>()(desc.fillMode) ^
			std::hash<CullMode>()(desc.cullMode) ^
			std::hash<bool>()(desc.frontCounterClockwise) ^
			std::hash<int32_t>()(desc.depthBias) ^
			std::hash<float>()(desc.depthBiasClamp) ^
			std::hash<float>()(desc.slopeScaledDepthBias) ^
			std::hash<bool>()(desc.depthClipEnable) ^
			std::hash<bool>()(desc.antialiasedLineEnable) ^
			std::hash<int32_t>()(desc.forcedSampleCount);
	}
};

template<>
struct std::hash<Prism::Render::DepthStencilOperationDesc>
{
	size_t operator()(const Prism::Render::DepthStencilOperationDesc& desc) const noexcept
	{
		using namespace Prism::Render;

		static_assert(sizeof(desc) == 16,
			"If new field was added, add it to the hash function and update this assert");

		return
			std::hash<StencilOperation>()(desc.stencilFail) ^
			std::hash<StencilOperation>()(desc.stencilDepthFail) ^
			std::hash<StencilOperation>()(desc.stencilPass) ^
			std::hash<ComparisionFunction>()(desc.stencilFunction);
	}
};

template<>
struct std::hash<Prism::Render::DepthStencilStateDesc>
{
	size_t operator()(const Prism::Render::DepthStencilStateDesc& desc) const noexcept
	{
		using namespace Prism::Render;

		static_assert(sizeof(desc) == 44,
			"If new field was added, add it to the hash function and update this assert");

		return
			std::hash<bool>()(desc.depthEnable) ^
			std::hash<bool>()(desc.depthWriteEnable) ^
			std::hash<ComparisionFunction>()(desc.depthFunc) ^
			std::hash<bool>()(desc.stencilEnable) ^
			std::hash<uint8_t>()(desc.stencilReadMask) ^
			std::hash<uint8_t>()(desc.stencilWriteMask) ^
			std::hash<DepthStencilOperationDesc>()(desc.frontFace) ^
			std::hash<DepthStencilOperationDesc>()(desc.backFace);
	}
};
