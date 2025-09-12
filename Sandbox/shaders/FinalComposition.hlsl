#include "Common.hlsli"

cbuffer Resources
{
	int g_sceneColorTexture;
	int g_bloomTexture;
}

struct VertexOut
{
	float4 positionClip : SV_Position;
	float2 ndc : POSITION;
	float2 texCoords : TEXCOORD;
};

// 9-tap bilinear upsampler (tent filter)
float3 UpsampleTent9Tap(Texture2D tex, SamplerState sam, float2 uv, float lod, float2 texelSize, float4 sampleScale)
{
	float4 offset = texelSize.xyxy * float4(1.0, 1.0, -1.0, 0.0) * sampleScale;

	float3 color;
	color = tex.SampleLevel(sam, uv - offset.xy, lod).rgb;
	color += tex.SampleLevel(sam, uv - offset.wy, lod).rgb * 2.f;
	color += tex.SampleLevel(sam, uv - offset.zy, lod).rgb;

	color += tex.SampleLevel(sam, uv + offset.zw, lod).rgb * 2.f;
	color += tex.SampleLevel(sam, uv, lod).rgb * 4.f;
	color += tex.SampleLevel(sam, uv + offset.xw, lod).rgb * 2.f;

	color += tex.SampleLevel(sam, uv + offset.zy, lod).rgb;
	color += tex.SampleLevel(sam, uv + offset.wy, lod).rgb * 2.f;
	color += tex.SampleLevel(sam, uv + offset.xy, lod).rgb;

	return color * (1.f / 16.f);
}

VertexOut vsmain(uint vertexID : SV_VertexID)
{
	VertexOut vout;
	if (vertexID == 0)
	{
		float2 vertex = float2(1.f, -1.f);
		vout.positionClip = float4(vertex, 0.f, 1.f);
		vout.ndc = vertex;
		vout.texCoords = float2(1.f, 1.f);
	}
	else if (vertexID == 1)
	{
		float2 vertex = float2(1.f, 3.f);
		vout.positionClip = float4(vertex, 0.f, 1.f);
		vout.ndc = vertex;
		vout.texCoords = float2(1.f, -1.f);
	}
	else if (vertexID == 2)
	{
		float2 vertex = float2(-3.f, -1.f);
		vout.positionClip = float4(vertex, 0.f, 1.f);
		vout.ndc = vertex;
		vout.texCoords = float2(-1.f, 1.f);
	}
	
	return vout;
}

float4 psmain(VertexOut pin) : SV_Target
{
	Texture2D sceneColorTexture = ResourceDescriptorHeap[g_sceneColorTexture];
	Texture2D bloomTexture = ResourceDescriptorHeap[g_bloomTexture];
	
	float3 bloom = bloomTexture.SampleLevel(g_samLinearClamp, pin.texCoords, 0).rgb;
	
	float3 color = sceneColorTexture.SampleLevel(g_samLinearClamp, pin.texCoords, 0).rgb;
	color += bloom;
	
	// Gamma correction
	color = color / (color + 1.f);
	color = pow(color, 1.f / 2.2f);
	
	return float4(color, 1.f);
}
