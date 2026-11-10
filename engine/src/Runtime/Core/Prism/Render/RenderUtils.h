#pragma once
#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/Shader.h"

namespace Prism::Render
{
int32_t GetSubresourceIndex(int32_t mipLevel, int32_t numMipLevels, int32_t arraySlice, int32_t numArraySlices);

float ReadPixelChannelValueAsFloat(void* data, int32_t numBits, TextureModifier modifier);
uint32_t ReadPixelChannelValueAsUInt(void* data, int32_t numBits, TextureModifier modifier);
int32_t ReadPixelChannelValueAsSInt(void* data, int32_t numBits, TextureModifier modifier);

float ReadBitsAsFloat(void* data, int32_t numBits);
int32_t ReadBitsAsInt(void* data, int32_t numBits);
uint32_t ReadBitsAsUInt(void* data, int32_t numBits);

void DrawFullscreenPixelShader(class RenderContext* renderContext, glm::float2 screenSize, ShaderDesc ps, BlendStateDesc* blendState = nullptr,
							   RasterizerStateDesc* rasterizerState = nullptr, DepthStencilStateDesc* depthStencilState = nullptr);
}
