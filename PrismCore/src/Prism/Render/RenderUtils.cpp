#include "pcpch.h"
#include "RenderUtils.h"

#include "Prism/Utilities/MemoryUtils.h"

namespace Prism::Render
{
glm::int4 GetBitsPerChannel(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::Unknown:
		return { 0, 0, 0, 0 };

	case TextureFormat::RGBA32_Typeless:
	case TextureFormat::RGBA32_Float:
	case TextureFormat::RGBA32_UInt:
	case TextureFormat::RGBA32_SInt:
		return { 32, 32, 32, 32 };

	case TextureFormat::RGB32_Typeless:
	case TextureFormat::RGB32_Float:
	case TextureFormat::RGB32_UInt:
	case TextureFormat::RGB32_SInt:
		return { 32, 32, 32, 0 };

	case TextureFormat::RGBA16_Typeless:
	case TextureFormat::RGBA16_Float:
	case TextureFormat::RGBA16_UNorm:
	case TextureFormat::RGBA16_UInt:
	case TextureFormat::RGBA16_SNorm:
	case TextureFormat::RGBA16_SInt:
		return { 16, 16, 16, 16 };

	case TextureFormat::RG32_Typeless:
	case TextureFormat::RG32_Float:
	case TextureFormat::RG32_UInt:
	case TextureFormat::RG32_SInt:
		return { 32, 32, 0, 0 };

	case TextureFormat::R32G8X24_Typeless:
	case TextureFormat::D32_Float_S8X24_UInt:
	case TextureFormat::R32_Float_X8X24_Typeless:
	case TextureFormat::X32_Typeless_G8X24_UInt:
		// 32 bits of depth, 8 bits of stencil, 24 bits padding
		return { 32, 8, 24, 0 };

	case TextureFormat::RGB10A2_Typeless:
	case TextureFormat::RGB10A2_UNorm:
	case TextureFormat::RGB10A2_UInt:
		return { 10, 10, 10, 2 };

	case TextureFormat::R11G11B10_Float:
		// No alpha channel
		return { 11, 11, 10, 0 };

	case TextureFormat::RGBA8_Typeless:
	case TextureFormat::RGBA8_UNorm:
	case TextureFormat::RGBA8_UNorm_SRGB:
	case TextureFormat::RGBA8_UInt:
	case TextureFormat::RGBA8_SNorm:
	case TextureFormat::RGBA8_SInt:
		return { 8, 8, 8, 8 };

	case TextureFormat::RG16_Typeless:
	case TextureFormat::RG16_Float:
	case TextureFormat::RG16_UNorm:
	case TextureFormat::RG16_UInt:
	case TextureFormat::RG16_SNorm:
	case TextureFormat::RG16_SInt:
		return { 16, 16, 0, 0 };

	case TextureFormat::R32_Typeless:
	case TextureFormat::D32_Float:
	case TextureFormat::R32_Float:
	case TextureFormat::R32_UInt:
	case TextureFormat::R32_SInt:
		return { 32, 0, 0, 0 };

	case TextureFormat::R24G8_Typeless:
	case TextureFormat::D24_UNorm_S8_UInt:
		// 24 bits for depth, 8 bits for stencil
		return { 24, 8, 0, 0 };

	case TextureFormat::R24_UNorm_X8_Typeless:
	case TextureFormat::X24_Typeless_G8_UInt:
		// Also often treated as 24 bits for depth, 8 bits for stencil/padding
		return { 24, 8, 0, 0 };

	case TextureFormat::RG8_Typeless:
	case TextureFormat::RG8_UNorm:
	case TextureFormat::RG8_UInt:
	case TextureFormat::RG8_SNorm:
	case TextureFormat::RG8_SInt:
		return { 8, 8, 0, 0 };

	case TextureFormat::R16_Typeless:
	case TextureFormat::R16_Float:
	case TextureFormat::D16_UNorm:
	case TextureFormat::R16_UNorm:
	case TextureFormat::R16_UInt:
	case TextureFormat::R16_SNorm:
	case TextureFormat::R16_SInt:
		return { 16, 0, 0, 0 };

	case TextureFormat::R8_Typeless:
	case TextureFormat::R8_UNorm:
	case TextureFormat::R8_UInt:
	case TextureFormat::R8_SNorm:
	case TextureFormat::R8_SInt:
		return { 8, 0, 0, 0 };

	case TextureFormat::A8_UNorm:
		// Only alpha channel
		return { 0, 0, 0, 8 };

	case TextureFormat::R1_UNorm:
		// Only 1 bit in R channel
		return { 1, 0, 0, 0 };

	case TextureFormat::RGB9E5_SHAREDEXP:
		// Typically: 9 bits each for R/G/B + 5 bits shared exponent
		return { 9, 9, 9, 5 };

	case TextureFormat::RG8_B8G8_UNorm:
	case TextureFormat::G8R8_G8B8_UNorm:
		// Packed 8-bit pairs; often effectively 8 bits each for R/G/B/A
		return { 8, 8, 8, 8 };

		//
		// Block-compressed BC formats don’t map to direct per-channel bits in the usual sense.
		// Typically, we might just return {0,0,0,0}. Adjust if your logic needs something else.
		//
	case TextureFormat::BC1_Typeless:
	case TextureFormat::BC1_UNorm:
	case TextureFormat::BC1_UNorm_SRGB:
	case TextureFormat::BC2_Typeless:
	case TextureFormat::BC2_UNorm:
	case TextureFormat::BC2_UNorm_SRGB:
	case TextureFormat::BC3_Typeless:
	case TextureFormat::BC3_UNorm:
	case TextureFormat::BC3_UNorm_SRGB:
	case TextureFormat::BC4_Typeless:
	case TextureFormat::BC4_UNorm:
	case TextureFormat::BC4_SNorm:
	case TextureFormat::BC5_Typeless:
	case TextureFormat::BC5_UNorm:
	case TextureFormat::BC5_SNorm:
		PE_ASSERT_NO_ENTRY();
		return { 0, 0, 0, 0 };

	case TextureFormat::B5G6R5_UNorm:
		// B=5, G=6, R=5 => in RGBA order, R=5, G=6, B=5, A=0
		return { 5, 6, 5, 0 };

	case TextureFormat::B5G5R5A1_UNorm:
		// B=5, G=5, R=5, A=1 => RGBA => R=5, G=5, B=5, A=1
		return { 5, 5, 5, 1 };

	case TextureFormat::BGRA8_UNorm:
		// B=8, G=8, R=8, A=8 => in RGBA => R=8, G=8, B=8, A=8
		return { 8, 8, 8, 8 };

	case TextureFormat::BGRX8_UNorm:
		// B=8, G=8, R=8, X=8 => no alpha => RGBA => R=8, G=8, B=8, A=0
		return { 8, 8, 8, 0 };

	case TextureFormat::R10G10B10_XR_BIAS_A2_UNorm:
		// R=10, G=10, B=10, A=2
		return { 10, 10, 10, 2 };

	case TextureFormat::BGRA8_Typeless:
	case TextureFormat::BGRA8_UNorm_SRGB:
		// Same as BGRA8_UNorm
		return { 8, 8, 8, 8 };

	case TextureFormat::BGRX8_Typeless:
	case TextureFormat::BGRX8_UNorm_SRGB:
		// Same as BGRX8_UNorm
		return { 8, 8, 8, 0 };

	case TextureFormat::BC6H_Typeless:
	case TextureFormat::BC6H_UF16:
	case TextureFormat::BC6H_SF16:
	case TextureFormat::BC7_Typeless:
	case TextureFormat::BC7_UNorm:
	case TextureFormat::BC7_UNorm_SRGB:
		PE_ASSERT_NO_ENTRY();
		return { 0, 0, 0, 0 };

	default:
		PE_ASSERT_NO_ENTRY();
		return {};
	}

}

float ReadPixelChannelValueAsFloat(void* data, int32_t numBits, TextureModifier modifier)
{
	return 0;
}

uint32_t ReadPixelChannelValueAsUInt(void* data, int32_t numBits, TextureModifier modifier)
{
	PE_ASSERT(data);
	PE_ASSERT(numBits > 0);

	int32_t numBytes = Align(numBits, 8) / 8;

	uint32_t output = 0;

	// For each byte
	for (int32_t i = 0; i < numBytes; ++i)
	{
		uint8_t byte = *(uint8_t*)data;
		if (i == 0 && numBits % 8 > 0)
			byte &= (1 << (numBits % 8)) - 1;
		if (i == 0 && (modifier == TextureModifier::SInt || modifier == TextureModifier::SNorm))
			byte &= (1 << (numBits % 8 - 1)) - 1;

		output |= byte << (numBytes - i - 1) * 8;
	}

	return output;
}

int32_t ReadPixelChannelValueAsSInt(void* data, int32_t numBits, TextureModifier modifier)
{
	PE_ASSERT(data);
	PE_ASSERT(numBits > 0);

	int32_t numBytes = Align(numBits, 8) / 8;

	uint32_t output = 0;

	// For each byte
	for (int32_t i = 0; i < numBytes; ++i)
	{
		uint8_t byte = *(uint8_t*)data;
		if (i == 0 && numBits % 8 > 0)
			byte &= (1 << (numBits % 8)) - 1;
		if (i == 0 && (modifier == TextureModifier::SInt || modifier == TextureModifier::SNorm))
			byte &= (1 << (numBits % 8 - 1)) - 1;

		output |= byte << (numBytes - i - 1) * 8;
	}

	return output;
}

float ReadBitsAsFloat(void* data, int32_t numBits)
{
	PE_ASSERT(data);
	PE_ASSERT(numBits > 0);
	PE_ASSERT(numBits == 16 || numBits == 32);

	if (numBits == 16)
	{
		uint16_t FP16 = *(uint16_t*)data;
		constexpr uint32_t shifted_exp = 0x7c00 << 13;			// exponent mask after shift
		union FP32T
		{
			uint32_t u;
			float f;
		} FP32 = {}, magic = { 113 << 23 };

		FP32.u = (FP16 & 0x7fff) << 13;				// exponent/mantissa bits
		uint32_t exp = shifted_exp & FP32.u;		// just the exponent
		FP32.u += uint32_t(127 - 15) << 23;			// exponent adjust

		// handle exponent special cases
		if (exp == shifted_exp)						// Inf/NaN?
		{
			FP32.u += uint32_t(128 - 16) << 23;		// extra exp adjust
		}
		else if (exp == 0)							// Zero/Denormal?
		{
			FP32.u += 1 << 23;						// extra exp adjust
			FP32.f -= magic.f;						// renormalize
		}

		FP32.u |= (FP16 & 0x8000) << 16;			// sign bit
		return FP32.f;
	}
	else if (numBits == 32)
	{
		return *(float*)data;
	}
	else
	{
		PE_ASSERT_NO_ENTRY();
		return 0.f;
	}
}

int32_t ReadBitsAsInt(void* data, int32_t numBits)
{
	PE_ASSERT(data);
	PE_ASSERT(numBits > 0 && numBits < 32);

	int32_t numBytes = Align(numBits, 8) / 8;

	int32_t output = 0;
	bool positive = true;

	// For each byte
	for (int32_t i = 0; i < numBytes; ++i)
	{
		uint8_t byte = *(uint8_t*)data;
		if (i == 0)
		{
			uint32_t leftmostBitInByte = numBits % 8 > 0 ? numBits % 8 : 8;
			uint32_t valueMask = (1 << (leftmostBitInByte - 1)) - 1;
			uint32_t signMask = 1 << (leftmostBitInByte - 1);

			byte &= valueMask;
			positive = byte & signMask;
		}

		output |= byte << (numBytes - i - 1) * 8;
	}

	return positive ? output : -output;
}

uint32_t ReadBitsAsUInt(void* data, int32_t numBits)
{
	PE_ASSERT(data);
	PE_ASSERT(numBits > 0 && numBits < 32);

	int32_t numBytes = Align(numBits, 8) / 8;

	uint32_t output = 0;

	// For each byte
	for (int32_t i = 0; i < numBytes; ++i)
	{
		uint8_t byte = *(uint8_t*)data;
		if (i == 0 && numBits % 8 > 0)
			byte &= (1 << (numBits % 8)) - 1;

		output |= byte << (numBytes - i - 1) * 8;
	}

	return output;
}
}
