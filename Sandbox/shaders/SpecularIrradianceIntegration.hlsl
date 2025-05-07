#include "BRDF.hlsli"

cbuffer Resources
{
	int g_skybox;
	int g_outputTexture;
	int g_prefilterData;
}

struct PrefilterData
{
	float roughness;
	int totalResolution;
	int mipResolution;
	int sampleCount;
};

float3 GetSamplingVector(int3 threadID, int resolution)
{
	float x = ((float)threadID.x / (resolution - 1.f)) * 2.f - 1.f;
	float y = ((float)threadID.y / (resolution - 1.f)) * 2.f - 1.f;

	float3 direction;
	if (threadID.z == 0)
		direction = float3(1.f, -y, -x);
	else if (threadID.z == 1)
		direction = float3(-1.f, -y, x);
	else if (threadID.z == 2)
		direction = float3(x, 1.f, y);
	else if (threadID.z == 3)
		direction = float3(x, -1.f, -y);
	else if (threadID.z == 4)
		direction = float3(x, -y, 1.f);
	else if (threadID.z == 5)
		direction = float3(-x, -y, -1.f);
	
	direction = normalize(direction);
	return direction;
}

[numthreads(32, 32, 1)]
void main(int3 groupID : SV_GroupID,
		  int3 dispatchThreadID : SV_DispatchThreadID,
		  int groupIndex : SV_GroupIndex)
{
	TextureCube skybox = ResourceDescriptorHeap[g_skybox];
	RWTexture2DArray<float4> outputTexture = ResourceDescriptorHeap[g_outputTexture];
	ConstantBuffer<PrefilterData> prefilterData = ResourceDescriptorHeap[g_prefilterData];
	
	float3 normal = GetSamplingVector(dispatchThreadID, prefilterData.mipResolution);
	float3 toView = normal;
	
	float3 outputColor = 0.f;
	float totalWeight = 0.f;
	
	int sampleCount = prefilterData.sampleCount;

	for (int i = 0; i < sampleCount; ++i)
	{
		float2 Xi = Hammersley(i, sampleCount);
		float3 halfVector = HemisphereSample_GGX(Xi, prefilterData.roughness, normal);
		float3 toLight = normalize(2.f * dot(toView, halfVector) * halfVector - toView);
		
		float NdotL = max(dot(normal, toLight), 0.f);
		if (NdotL > 0.f)
		{
			float NdotH = saturate(dot(normal, halfVector));
			float VdotH = saturate(dot(toView, halfVector));
			
			float pdf = DistributionGGX(normal, halfVector, prefilterData.roughness) * NdotH / (4.f * VdotH);
			
			float omegaS = 1.f / (sampleCount * pdf);
			float omegaP = 4.f * PI / (6 * prefilterData.totalResolution * prefilterData.totalResolution);
			
			const float MIP_BIAS = 1.f;
			float mipLevel = max(0.5f * log2(omegaS / omegaP) + MIP_BIAS, 0.f);

			outputColor += skybox.SampleLevel(g_samLinearClamp, toLight, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	
	outputColor /= totalWeight;
	
	outputTexture[dispatchThreadID] = float4(outputColor, 1.f);
}
