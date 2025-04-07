#include "BRDF.hlsli"

cbuffer Resources
{
	int g_skybox;
	int g_coefficients;
}

[numthreads(1, 1, 1)]
void main(int3 groupID : SV_GroupID,
		  int3 dispatchThreadID : SV_DispatchThreadID,
		  int groupIndex : SV_GroupIndex)
{
	TextureCube skybox = ResourceDescriptorHeap[g_skybox];
	RWStructuredBuffer<float4> coefficients = ResourceDescriptorHeap[g_coefficients];

	SH::L2_RGB radianceSH = SH::L2_RGB::Zero();
	
	const int SAMPLE_COUNT = 1024;
	
	// We are doing 2 samples every iteration, one for the top hemisphere and one for the bottom hemisphere
	for (int i = 0; i < SAMPLE_COUNT / 2; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT / 2);
		float3 sampleTop = HemisphereSample(Xi, float3(0.f, 1.f, 0.f));
		float3 sampleBottom = HemisphereSample(Xi, float3(0.f, -1.f, 0.f));
		
		float3 colorTop = skybox.SampleLevel(g_samLinearWrap, sampleTop, 0).rgb;
		float3 colorBottom = skybox.SampleLevel(g_samLinearWrap, sampleBottom, 0).rgb;
		radianceSH = radianceSH + SH::ProjectOntoL2(sampleTop, colorTop);
		radianceSH = radianceSH + SH::ProjectOntoL2(sampleBottom, colorBottom);
	}
	
	radianceSH = radianceSH * (1.f / (SAMPLE_COUNT * (1 / (4 * PI))));
	
	for (int i = 0; i < 9; ++i)
		coefficients[i] = float4(radianceSH.C[i], 0.f);
}
