#include "BRDF.hlsli"

cbuffer Resources
{
	int g_skybox;
	int g_outputTexture;
	int g_integrationData;
}

struct IntegrationData
{
	float roughness;
	int resolution;
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
	ConstantBuffer<IntegrationData> integrationData = ResourceDescriptorHeap[g_integrationData];
	
	const int SAMPLE_COUNT = 32;
	
	float3 normal = GetSamplingVector(dispatchThreadID, integrationData.resolution);
	float3 V = normal;
	
	float3 outputColor = 0.f;
	float totalWeight = 0.f;
	
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = HemisphereSample_GGX(Xi, integrationData.roughness, normal);
		float3 L = normalize(2.f * dot(V, H) * H - V);
		
		float wt = 4.f * PI / (6 * integrationData.resolution * integrationData.resolution);
		
		float NdotL = max(dot(normal, L), 0.f);
		if (NdotL > 0.f)
		{
			float NdotH = max(dot(normal, H), 0.f);
			
			float pdf = DistributionGGX(normal, H, integrationData.roughness) * 0.25f;
			
			float ws = 1.f / (pdf * SAMPLE_COUNT);
			
			float mipLevel = max(0.5f * log2(ws / wt) + 1.f, 0.f);
			
			//
			//
			// TODO: Skybox doesnt have any mipmaps!!!
			//
			//
			outputColor += skybox.SampleLevel(g_samLinearClamp, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	
	outputColor /= totalWeight;
	
	outputTexture[dispatchThreadID] = float4(outputColor, 1.f);
}
