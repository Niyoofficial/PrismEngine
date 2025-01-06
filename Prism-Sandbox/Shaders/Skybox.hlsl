#include "Common.hlsl"
#include "SH.hlsl"

static const float PI = 3.14159265359f;
static const float A0 = PI;
static const float A1 = 2.094395f;
static const float A2 = 0.785398f;

TextureCube g_skybox;

struct Coefficients
{
	float3 sh[9];
};

RWStructuredBuffer<SH::L2_RGB> g_coeffs : register(u0);

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

Coefficients GetDirectionalSH(float3 dir)
{
	const float x = -dir.x;
	const float y = -dir.y;
	const float z = dir.z;
	
	const float x2 = x * x;
	const float y2 = y * y;
	const float z2 = z * z;
	
	const float sqrtPi = sqrt(PI);
	
	Coefficients result;
	
	result.sh[0] = 1.f / (2.f * sqrtPi);
	
	result.sh[1] = -sqrt(3.0f / (4.0f * PI)) * y;
	result.sh[2] = sqrt(3.0f / (4.0f * PI)) * z;
	result.sh[3] = -sqrt(3.0f / (4.0f * PI)) * x;
	
	result.sh[4] = sqrt(15.0f / (4.0f * PI)) * y * x;
	result.sh[5] = -sqrt(15.0f / (4.0f * PI)) * y * z;
	result.sh[6] = sqrt(5.0f / (16.0f * PI)) * (3.0f * z2 - 1.0f);
	result.sh[7] = -sqrt(15.0f / (4.0f * PI)) * x * z;
	result.sh[8] = sqrt(15.0f / (16.0f * PI)) * (x2 - y2);
	
	return result;
}

float4 psmain(PixelInputCube pin) : SV_TARGET
{
	float3 normal = normalize(pin.positionLocal.rgb);
	
	float3 color = SH::CalculateIrradiance(g_coeffs[0], normal) * (1.f / PI);
	return float4(color, 1.f);
}
