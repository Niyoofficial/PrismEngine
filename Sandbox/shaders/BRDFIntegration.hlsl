#include "BRDF.hlsli"

cbuffer Resources
{
	int g_integrationData;
	int g_outputTexture;
}

struct IntegrationData
{
	int resolution;
};

[numthreads(8, 8, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
	ConstantBuffer<IntegrationData> integrationData = ResourceDescriptorHeap[g_integrationData];
	RWTexture2D<float2> outputTexture = ResourceDescriptorHeap[g_outputTexture];
	
	float2 uv = float2(((float)dispatchThreadID.x + 0.5f) / integrationData.resolution, ((float)dispatchThreadID.y + 0.5f) / integrationData.resolution);
	
	float roughness = uv.y;
	float NdotV = uv.x;
	
	float3 toCamera;
	toCamera.x = sqrt(1.0f - NdotV * NdotV); // sin ()
	toCamera.y = 0;
	toCamera.z = NdotV; // cos()
	
	float3 normal = float3(0.f, 0.f, 1.f);
	
	float F0Scale = 0;	// Integral1
	float F0Bias = 0;	// Integral2
	
	const int SAMPLE_COUNT = 1024;
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 halfVector = HemisphereSample_GGX(Xi, roughness, normal);
		float3 toLight = normalize(2.f * dot(toCamera, halfVector) * halfVector - toCamera);
		
		float NdotL = max(toLight.z, 0.f);
		float NdotH = max(halfVector.z, 0.f);
		float VdotH = max(dot(toCamera, halfVector), 0.f);
		
		if (NdotL > 0.f)
		{
			float geometry = GeometrySmith_EnvMap(float3(0.f, 0.f, 1.f), toCamera, toLight, roughness);
			
			// Split Sum Approx : Specular BRDF integration using Quasi Monte Carlo
			// 
			// Microfacet specular = D*G*F / (4*NoL*NoV)
			// pdf = D * NoH / (4 * VoH)
			// G_Vis = Microfacet specular / (pdf * F)
			//
			// We do this to move F0 out of the integral so we can factor F0
			// in during the lighting pass, which just simplifies this LUT
			// into a representation of a scale and a bias to the F0
			// as in (F0 * scale + bias)
			float g_vis = (geometry * VdotH) / (NdotH * NdotV);
			float fc = pow(1.f - VdotH, 5.f);
			
			F0Scale += (1.f - fc) * g_vis;
			F0Bias += fc * g_vis;
		}
	}
	
	outputTexture[dispatchThreadID.xy] = float2(F0Scale, F0Bias) / SAMPLE_COUNT;
}
