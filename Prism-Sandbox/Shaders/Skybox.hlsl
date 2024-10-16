#include "Common.hlsl"

TextureCube g_skybox;

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

PixelInputCube vsmain(VertexInput vin)
{
    PixelInputCube vout;
	
    vout.positionLocal = vin.positionLocal;
	
    vout.positionClip = mul(g_camera.viewProj, float4(vin.positionLocal + g_camera.camPos, 1.f)).xyww;
	
    return vout;
}

float4 psmain(PixelInputCube pin) : SV_TARGET
{
    float4 color = g_skybox.Sample(g_samLinearWrap, pin.positionLocal);
	
	// Gamma correction
    color = color / (color + 1.f);
    color = pow(color, 1.f / 2.2f);

    return color;
}
