#define CUBEMAP_RESOLUTION 1024

SamplerState g_samPointWrap : register(s0);
SamplerState g_samPointClamp : register(s1);
SamplerState g_samLinearWrap : register(s2);
SamplerState g_samLinearClamp : register(s3);
SamplerState g_samAnisotropicWrap : register(s4);
SamplerState g_samAnisotropicClamp : register(s5);
SamplerComparisonState g_samShadow : register(s6);

Texture2D g_environment : register(t0);
RWTexture2DArray<float4> g_skybox : register(u0);

static const float PI = 3.14159265359f;
static const float TwoPI = 2 * PI;

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID,
		  int3 dispatchThreadID : SV_DispatchThreadID)
{
	float x = ((float)dispatchThreadID.x / (CUBEMAP_RESOLUTION - 1.f)) * 2.f - 1.f;
	float y = ((float)dispatchThreadID.y / (CUBEMAP_RESOLUTION - 1.f)) * 2.f - 1.f;

	float3 direction;
	if (dispatchThreadID.z == 0)
		direction = float3(1.f, -y, -x);
	else if (dispatchThreadID.z == 1)
		direction = float3(-1.f, -y, x);
	else if (dispatchThreadID.z == 2)
		direction = float3(x, 1.f, y);
	else if (dispatchThreadID.z == 3)
		direction = float3(x, -1.f, -y);
	else if (dispatchThreadID.z == 4)
		direction = float3(x, -y, 1.f);
	else if (dispatchThreadID.z == 5)
		direction = float3(-x, -y, -1.f);
	
	direction = normalize(direction);
    float phi = atan2(direction.z, direction.x);
    float theta = -asin(direction.y); // Inverted because in DirectX UV(0, 0) is in the _top_ left
    float2 uv = float2(phi / TwoPI, theta / PI) + 0.5f;

    g_skybox[dispatchThreadID] = clamp(g_environment.SampleLevel(g_samLinearClamp, uv, 0), 0, 1);
}
