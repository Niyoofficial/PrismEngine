//#include "BRDF.hlsli"

cbuffer Resources
{
	int g_skybox;
	int g_outputTexture
}

static const float PI = 3.14159265359f;
static const float PI_OVER_TWO = PI / 2.f;
static const float TWO_PI = PI * 2.f;

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

[numthreads(32, 32, 1)]
void main(int3 groupID : SV_GroupID,
		  int3 dispatchThreadID : SV_DispatchThreadID,
		  int groupIndex : SV_GroupIndex)
{
	Texture2DArray skybox = ResourceDescriptorHeap[g_skybox];
	RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[g_outputTexture];
}
