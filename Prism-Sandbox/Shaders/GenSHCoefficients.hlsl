#define CUBEMAP_RESOLUTION 1024
#define THREADS_COUNT 16 * 16 * 1

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
RWStructuredBuffer<Coefficients> g_coefficients : register(u0);

groupshared float3 g_groupResults[THREADS_COUNT][9];
groupshared float g_weights[THREADS_COUNT];

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

void GetSHCoefficients(float3 N, float3 L, float3 coefficients[9])
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
        direction = float3(x, -1.f, y);
    else if (threadID.z == 3)
        direction = float3(x, 1.f, -y);
    else if (threadID.z == 4)
        direction = float3(x, y, 1.f);
    else if (threadID.z == 5)
        direction = float3(-x, y, -1.f);
	
    direction = normalize(direction);
    return direction;
}

[numthreads(16, 16, 1)]
void main(int3 groupID : SV_GroupID,
		  int3 dispatchThreadID : SV_DispatchThreadID,
          int groupIndex : SV_GroupIndex)
{
    float3 light = g_skybox[dispatchThreadID].rgb;
    float3 normal = GetSamplingVector(dispatchThreadID);
    float3 coefficients[9];
    GetSHCoefficients(normal, light, coefficients);
 
    float u = (((float)dispatchThreadID.x + 0.5f) / (CUBEMAP_RESOLUTION)) * 2.f - 1.f;
    float v = (((float)dispatchThreadID.y + 0.5f) / (CUBEMAP_RESOLUTION)) * 2.f - 1.f;

    float temp = 1.f + u * u + v * v;
    float weight = 1.f / (sqrt(temp) * temp);
    g_weights[groupIndex] = weight;
    
    for (int i = 0; i < 9; ++i)
    {
        g_groupResults[groupIndex][i] = coefficients[i] * weight;
    }
    
    /*GroupMemoryBarrierWithGroupSync();

    if (groupIndex == 0)
    {
        float3 result[9];
        float weightSum = 0.f;
        for (int i = 0; i < THREADS_COUNT; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                result[j] += g_groupResults[i][j];
            }
            weightSum += g_weights[i];
        }
        
        int coeffID = groupID.z * 64 * 64 + groupID.y * 64 + groupID.x;
        for (int i = 0; i < 9; ++i)
        {
            result[i] *= 4.f * PI / weightSum;
            g_coefficients[coeffID].sh[i] = result[i];
        }
    }*/
}
