#include "Common.hlsli"

cbuffer Resources
{
	int g_bloomSettings;
	int g_inputTexture;
	int g_outputTexture;
	int g_accumulationTexture;
}

struct BloomSettings
{
	float threshold;
	float knee;
	int lod;
};

// Quadratic color thresholding
// curve = (threshold - knee, knee * 2, 0.25 / knee)
float3 QuadraticThreshold(float3 color, float threshold, float3 curve)
{
	// Maximum pixel brightness
	float brightness = max(max(color.r, color.g), color.b);
	// Quadratic curve
	float rq = clamp(brightness - curve.x, 0.f, curve.y);
	rq = (rq * rq) * curve.z;
	color *= max(rq, brightness - threshold) / max(brightness, 1e-4);
	return color;
}

float3 DownsampleBox13Tap(Texture2D tex, SamplerState sam, float lod, float2 uv, float2 texelSize)
{
	float3 a = tex.SampleLevel(sam, uv + texelSize * float2(-1.0f, -1.0f), lod).rgb;
	float3 b = tex.SampleLevel(sam, uv + texelSize * float2( 0.0f, -1.0f), lod).rgb;
	float3 c = tex.SampleLevel(sam, uv + texelSize * float2( 1.0f, -1.0f), lod).rgb;
	float3 d = tex.SampleLevel(sam, uv + texelSize * float2(-0.5f, -0.5f), lod).rgb;
	float3 e = tex.SampleLevel(sam, uv + texelSize * float2( 0.5f, -0.5f), lod).rgb;
	float3 f = tex.SampleLevel(sam, uv + texelSize * float2(-1.0f,  0.0f), lod).rgb;
	float3 g = tex.SampleLevel(sam, uv + texelSize						 , lod).rgb;
	float3 h = tex.SampleLevel(sam, uv + texelSize * float2( 1.0f,  0.0f), lod).rgb;
	float3 i = tex.SampleLevel(sam, uv + texelSize * float2(-0.5f,  0.5f), lod).rgb;
	float3 j = tex.SampleLevel(sam, uv + texelSize * float2( 0.5f,  0.5f), lod).rgb;
	float3 k = tex.SampleLevel(sam, uv + texelSize * float2(-1.0f,  1.0f), lod).rgb;
	float3 l = tex.SampleLevel(sam, uv + texelSize * float2( 0.0f,  1.0f), lod).rgb;
	float3 m = tex.SampleLevel(sam, uv + texelSize * float2( 1.0f,  1.0f), lod).rgb;
		
	float2 div = float2(0.5f, 0.125f);

	float3 output = (d + e + i + j) * div.x;
		output += (a + b + g + f) * div.y;
		output += (b + c + h + g) * div.y;
		output += (f + g + l + k) * div.y;
		output += (g + h + m + l) * div.y;
	
	// 4 samples each
	output /= 4;
	
	return output;
}

// 9-tap bilinear upsampler (tent filter)
float3 UpsampleTent9Tap(Texture2D tex, SamplerState sam, float2 uv, float lod, float2 texelSize, float4 sampleScale)
{
	float4 offset = texelSize.xyxy * float4(1.0, 1.0, -1.0, 0.0) * sampleScale;

	float3 color;
	color =  tex.SampleLevel(sam, uv - offset.xy, lod).rgb;
	color += tex.SampleLevel(sam, uv - offset.wy, lod).rgb * 2.f;
	color += tex.SampleLevel(sam, uv - offset.zy, lod).rgb;

	color += tex.SampleLevel(sam, uv + offset.zw, lod).rgb * 2.f;
	color += tex.SampleLevel(sam, uv			 , lod).rgb * 4.f;
	color += tex.SampleLevel(sam, uv + offset.xw, lod).rgb * 2.f;

	color += tex.SampleLevel(sam, uv + offset.zy, lod).rgb;
	color += tex.SampleLevel(sam, uv + offset.wy, lod).rgb * 2.f;
	color += tex.SampleLevel(sam, uv + offset.xy, lod).rgb;

	return color * (1.f / 16.f);
}

[numthreads(4, 4, 1)]
void Prefilter(int3 dispatchThreadID : SV_DispatchThreadID)
{	
	ConstantBuffer<BloomSettings> bloomSettings = ResourceDescriptorHeap[g_bloomSettings];
	Texture2D inputTexture = ResourceDescriptorHeap[g_inputTexture];
	RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[g_outputTexture];
	
	float2 texRes;
	outputTexture.GetDimensions(texRes.x, texRes.y);
	float2 texCoords = float2((float)dispatchThreadID.x / texRes.x, (float)dispatchThreadID.y / texRes.y);
	texCoords += (1.f / texRes) * 0.5f;
	
	float3 color = DownsampleBox13Tap(inputTexture, g_samLinearClamp, 0, texCoords, 1.f / texRes);
	const float clampValue = 20.f;
	color = clamp(color, float3(0.f, 0.f, 0.f), float3(clampValue, clampValue, clampValue));
	float3 curve = float3(bloomSettings.threshold - bloomSettings.knee, bloomSettings.knee * 2.f, 0.25f / bloomSettings.knee);
	color = QuadraticThreshold(color, bloomSettings.threshold, curve);
	
	outputTexture[dispatchThreadID.xy] = float4(color, 1.f);
}

[numthreads(4, 4, 1)]
void Downsample(int3 dispatchThreadID : SV_DispatchThreadID)
{
	ConstantBuffer<BloomSettings> bloomSettings = ResourceDescriptorHeap[g_bloomSettings];
	Texture2D inputTexture = ResourceDescriptorHeap[g_inputTexture];
	RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[g_outputTexture];
	
	float2 texRes;
	outputTexture.GetDimensions(texRes.x, texRes.y);
	float2 texCoords = float2((float)dispatchThreadID.x / texRes.x, (float)dispatchThreadID.y / texRes.y);
	texCoords += (1.f / texRes) * 0.5f;
	
	float3 color = DownsampleBox13Tap(inputTexture, g_samLinearClamp, bloomSettings.lod, texCoords, 1.f / texRes);
	
	outputTexture[dispatchThreadID.xy] = float4(color, 1.f);
}

[numthreads(4, 4, 1)]
void Upsample(int3 dispatchThreadID : SV_DispatchThreadID)
{
	ConstantBuffer<BloomSettings> bloomSettings = ResourceDescriptorHeap[g_bloomSettings];
	Texture2D inputTexture = ResourceDescriptorHeap[g_inputTexture];
	Texture2D accumulationTexture = ResourceDescriptorHeap[g_accumulationTexture];
	RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[g_outputTexture];
	
	float2 texRes;
	outputTexture.GetDimensions(texRes.x, texRes.y);
	float2 texCoords = float2((float)dispatchThreadID.x / texRes.x, (float)dispatchThreadID.y / texRes.y);
	texCoords += (1.f / texRes) * 0.5f;
	
	float2 accTexRes;
	uint mipLevels;
	inputTexture.GetDimensions(bloomSettings.lod + 1, accTexRes.x, accTexRes.y, mipLevels);
	float3 color = UpsampleTent9Tap(accumulationTexture, g_samLinearClamp, texCoords, bloomSettings.lod + 1, 1.f / accTexRes, 1.f);
	color += inputTexture.SampleLevel(g_samLinearClamp, texCoords, bloomSettings.lod).rgb;
	
	outputTexture[dispatchThreadID.xy] = float4(color, 1.f);
}
