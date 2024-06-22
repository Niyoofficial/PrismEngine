Texture2D g_texture : register(t0);

SamplerState g_samPointWrap : register(s0);
SamplerState g_samPointClamp : register(s1);
SamplerState g_samLinearWrap : register(s2);
SamplerState g_samLinearClamp : register(s3);
SamplerState g_samAnisotropicWrap : register(s4);
SamplerState g_samAnisotropicClamp : register(s5);
SamplerComparisonState g_samShadow : register(s6);

cbuffer TestBuffer
{
    float4x4 g_view;
    float4x4 g_proj;
    float4x4 g_viewProj;
};

struct VertexInput
{
	float3 positionLocal : POSITION;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

struct PixelInput
{
	float4 positionClip : SV_POSITION;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

PixelInput vsmain(VertexInput vin)
{
	PixelInput vout;

    vout.positionClip = mul(g_viewProj, float4(vin.positionLocal, 1.f));
	
    vout.texCoords = vin.texCoords;

	return vout;
}

float4 psmain(PixelInput pin) : SV_TARGET
{
    return g_texture.Sample(g_samLinearWrap, pin.texCoords);
}
