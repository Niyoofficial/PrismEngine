#include "Common.hlsli"
#include "FullscreenTriangleVertexShader.hlsli"

cbuffer Resources
{
	int g_sceneColorTexture;
	int g_bloomTexture;
	int g_outlineTexture;
	int g_outlineSettings;
}

struct OutlineSettings
{
	float outlineWidth;
};

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

float4 psmain(FullscreenVertexOut pin) : SV_Target
{
	Texture2D sceneColorTexture = ResourceDescriptorHeap[g_sceneColorTexture];
	Texture2D bloomTexture = ResourceDescriptorHeap[g_bloomTexture];
	
	float3 bloom = bloomTexture.SampleLevel(g_samLinearClamp, pin.texCoords, 0).rgb;
	
	float3 color = sceneColorTexture.SampleLevel(g_samLinearClamp, pin.texCoords, 0).rgb;
	color += bloom;

	// Selection outline
	if (g_outlineTexture != -1 && g_outlineSettings != -1)
	{
		Texture2D<float2> outlineTexture = ResourceDescriptorHeap[g_outlineTexture];
		ConstantBuffer<OutlineSettings> outlineSettings = ResourceDescriptorHeap[g_outlineSettings];

		uint2 texSize;
		outlineTexture.GetDimensions(texSize.x, texSize.y);
		float2 nearestPos = outlineTexture.Load(int3(pin.texCoords * texSize, 0)).rg * texSize;
		float2 currentPos = pin.texCoords * texSize;

		// distance in pixels to the closest position
		float dist = length(nearestPos - currentPos);

		float outline = saturate(outlineSettings.outlineWidth - dist + 1.f);
		float3 coloredOutline = outline * float3(1.f, 0.45f, 0.f);
		color = (1.f - coloredOutline) * color + coloredOutline;
	}

	// Gamma correction
	color = color / (color + 1.f);
	color = pow(color, 1.f / 2.2f);
	
	return float4(color, 1.f);
}
