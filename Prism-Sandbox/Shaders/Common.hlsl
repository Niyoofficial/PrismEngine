#include "SH.hlsl"

#define MAX_LIGHT_COUNT 16

static const float PI = 3.14159265359f;

SamplerState g_samPointWrap : register(s0);
SamplerState g_samPointClamp : register(s1);
SamplerState g_samLinearWrap : register(s2);
SamplerState g_samLinearClamp : register(s3);
SamplerState g_samAnisotropicWrap : register(s4);
SamplerState g_samAnisotropicClamp : register(s5);
SamplerComparisonState g_samShadow : register(s6);

struct CameraInfo
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;

    float3 camPos;
};

struct DirectionalLight
{
    float3 direction;
    float3 lightColor;
};

struct PointLight
{
    float3 position;
    float3 lightColor;
};

cbuffer SceneBuffer : register(b0, space1)
{
    CameraInfo g_camera;

    DirectionalLight g_directionalLights[MAX_LIGHT_COUNT];
    PointLight g_pointLights[MAX_LIGHT_COUNT];
};

cbuffer SceneIrradiance : register(b1, space1)
{
	SH::L2_RGB g_irradianceSH;
}

struct Material
{
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
};

cbuffer ModelBuffer : register(b2, space1)
{
    float4x4 g_world;
	
    bool g_useAlbedoTexture;
    bool g_useMetallicTexture;
    bool g_useRoughnessTexture;
    bool g_useAoTexture;
    bool g_useNormalTexture;

    Material g_material;
};
