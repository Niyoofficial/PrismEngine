Texture2D g_texture : register(t0);

SamplerState g_samPointWrap : register(s0);
SamplerState g_samPointClamp : register(s1);
SamplerState g_samLinearWrap : register(s2);
SamplerState g_samLinearClamp : register(s3);
SamplerState g_samAnisotropicWrap : register(s4);
SamplerState g_samAnisotropicClamp : register(s5);
SamplerComparisonState g_samShadow : register(s6);

cbuffer CameraBuffer : register(b0)
{
    float4x4 g_view;
    float4x4 g_proj;
    float4x4 g_viewProj;
};

cbuffer ModelBuffer : register(b1)
{
    float4x4 g_world;
};

struct VertexInput
{
	float3 positionLocal : POSITION;
	float3 normalLocal : NORMAL;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

struct PixelInput
{
	float4 positionClip : SV_POSITION;
	float4 positionWorld : POSITION;
	float3 normalWorld : NORMAL;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

PixelInput vsmain(VertexInput vin)
{
	PixelInput vout;
	
    float4 posWorld = mul(g_world, float4(vin.positionLocal, 1.f));
    vout.positionWorld = posWorld;
    vout.positionClip = mul(g_viewProj, posWorld);
	
    vout.normalWorld = mul((float3x3)g_world, vin.normalLocal);
	
    vout.texCoords = vin.texCoords;

	return vout;
}

float4 monkeypsmain(PixelInput pin) : SV_TARGET
{
    return float4(pin.normalWorld, 0.f);
    return g_texture.Sample(g_samLinearWrap, pin.texCoords);
}

float4 floorpsmain(PixelInput pin) : SV_TARGET
{
    return float4(0.3f, 0.4f, 0.8f, 1.f);
}
