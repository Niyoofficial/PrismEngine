#include "SH.hlsl"

#define CUBEMAP_RESOLUTION 1024
#define THREADS_COUNT 16 * 16 * 1
#define WARP_SIZE 32
#define THREAD_GROUP_COUNT_X 64
#define THREAD_GROUP_COUNT_Y 64
#define THREAD_GROUP_COUNT_Z 6

SamplerState g_samPointWrap : register(s0);
SamplerState g_samPointClamp : register(s1);
SamplerState g_samLinearWrap : register(s2);
SamplerState g_samLinearClamp : register(s3);
SamplerState g_samAnisotropicWrap : register(s4);
SamplerState g_samAnisotropicClamp : register(s5);
SamplerComparisonState g_samShadow : register(s6);

struct Coefficients
{
	float3 sh[9];
};

Texture2DArray g_skybox : register(t0);
RWStructuredBuffer<SH::L2_RGB> g_coefficients : register(u0);

groupshared Coefficients g_groupResults[THREADS_COUNT / WARP_SIZE];
groupshared float g_groupWeights[THREADS_COUNT / WARP_SIZE];

static const float PI = 3.14159265359f;

void GetSHBasis(float3 N, out float coefficients[9])
{
	// Band 0
	coefficients[0] = 0.282095f;
 
	// Band 1
	coefficients[1] = 0.488603f * N.y;
	coefficients[2] = 0.488603f * N.z;
	coefficients[3] = 0.488603f * N.x;
 
	// Band 2
	coefficients[4] = 1.092548f * N.x * N.y;
	coefficients[5] = 1.092548f * N.y * N.z;
	coefficients[6] = 0.315392f * (3.f * N.z * N.z - 1.f);
	coefficients[7] = 1.092548f * N.x * N.z;
	coefficients[8] = 0.546274f * (N.x * N.x - N.y * N.y);
}

void GetSHCoefficients(float3 N, float3 L, out float3 coefficients[9])
{
	float basis[9];
	GetSHBasis(N, basis);
	
	for (int i = 0; i < 9; ++i)
	{
		coefficients[i] = basis[i] * L;
	}
}

float3 GetSamplingVector(int3 threadID)
{
	float x = ((float)threadID.x / (CUBEMAP_RESOLUTION - 1.f)) * 2.f - 1.f;
	float y = ((float)threadID.y / (CUBEMAP_RESOLUTION - 1.f)) * 2.f - 1.f;

	float3 direction;
	if (threadID.z == 0)
		direction = float3(1.f, y, -x);
	else if (threadID.z == 1)
		direction = float3(-1.f, y, x);
	else if (threadID.z == 2)
		direction = float3(x, 1.f, y);
	else if (threadID.z == 3)
		direction = float3(x, -1.f, -y);
	else if (threadID.z == 4)
		direction = float3(x, y, 1.f);
	else if (threadID.z == 5)
		direction = float3(-x, y, -1.f);
	
	direction = normalize(direction);
	return direction;
}

[numthreads(1, 1, 1)]
void main(int3 groupID : SV_GroupID,
		  int3 dispatchThreadID : SV_DispatchThreadID,
		  int groupIndex : SV_GroupIndex)
{
	SH::L2_RGB radianceSH = SH::L2_RGB::Zero();

	for (int z = 0; z < 6; ++z)
	{
		for (int y = 0; y < CUBEMAP_RESOLUTION; ++y)
		{
			for (int x = 0; x < CUBEMAP_RESOLUTION; ++x)
			{
				float3 normal = GetSamplingVector(int3(x, y, z));
				float3 color = g_skybox[int3(x, y, z)].rgb;
				radianceSH = radianceSH + SH::ProjectOntoL2(normal, color);
			}
		}
	}
	
	radianceSH = radianceSH * (1.f / ((CUBEMAP_RESOLUTION * CUBEMAP_RESOLUTION * 6) * (1 / (4 * PI))));
	
	g_coefficients[0] = radianceSH;
}
