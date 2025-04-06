#pragma once
#include "Prism/Render/RenderTypes.h"

namespace Prism::Render
{
float ReadPixelChannelValueAsFloat(void* data, int32_t numBits, TextureModifier modifier);
uint32_t ReadPixelChannelValueAsUInt(void* data, int32_t numBits, TextureModifier modifier);
int32_t ReadPixelChannelValueAsSInt(void* data, int32_t numBits, TextureModifier modifier);

float ReadBitsAsFloat(void* data, int32_t numBits);
int32_t ReadBitsAsInt(void* data, int32_t numBits);
uint32_t ReadBitsAsUInt(void* data, int32_t numBits);
}
