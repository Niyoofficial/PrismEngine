#include "Common.hlsl"

static const float PI = 3.14159265359f;
static const float A0 = PI;
static const float A1 = 2.094395f;
static const float A2 = 0.785398f;

TextureCube g_skybox;

struct Coefficients
{
    float3 sh[9];
};

cbuffer CoefficientsInput : register(b3)
{
    Coefficients g_coeffs;
};

struct VertexInput
{
    float3 positionLocal : POSITION;
    float3 normalLocal : NORMAL;
    float3 tangentlLocal : TANGENT;
    float3 color : COLOR;
    float2 texCoords : TEXCOORD;
};

struct PixelInputCube
{
    float4 positionClip : SV_POSITION;
    float3 positionLocal : POSITION;
};

float3 GetLight(float3 N, Coefficients coeffs)
{
    float3 output =
	// Band 0
    coeffs.sh[0] * 0.282095f * A0 +
	// Band 1
    coeffs.sh[1] * 0.488603f * N.y * A1 +
    coeffs.sh[2] * 0.488603f * N.z * A1 +
    coeffs.sh[3] * 0.488603f * N.x * A1 +
	// Band 2
    coeffs.sh[4] * 1.092548f * N.x * N.y * A2 +
    coeffs.sh[5] * 1.092548f * N.y * N.z * A2 +
    coeffs.sh[6] * 0.315392f * (3.f * N.z * N.z - 1.f) * A2 +
    coeffs.sh[7] * 1.092548f * N.x * N.z * A2 +
    coeffs.sh[8] * 0.546274f * (N.x * N.x - N.y * N.y) * A2;
    
    return output;
}

PixelInputCube vsmain(VertexInput vin)
{
    PixelInputCube vout;
	
    vout.positionLocal = vin.positionLocal;
	
    vout.positionClip = mul(g_camera.viewProj, float4(vin.positionLocal + g_camera.camPos, 1.f)).xyww;
	
    return vout;
}

float4 psmain(PixelInputCube pin) : SV_TARGET
{
    float3 color = GetLight(normalize(pin.positionLocal), g_coeffs); //g_skybox.Sample(g_samLinearWrap, pin.positionLocal);
	
	// Gamma correction
    color = color / (color + 1.f);
    color = pow(color, 1.f / 2.2f);

    return float4(color, 1.f);
}
