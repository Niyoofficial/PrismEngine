#include "SH.hlsli"

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

struct Material
{
	float3 albedo;
	float metallic;
	float roughness;
	float ao;
};

// CBuffer structs
struct SceneBuffer
{
	float environmentDiffuseScale;
	CameraInfo camera;

	DirectionalLight directionalLights[MAX_LIGHT_COUNT];
	PointLight pointLights[MAX_LIGHT_COUNT];
};

struct SceneIrradiance
{
	SH::L2_RGB irradianceSH;
};

struct ModelBuffer
{
	float4x4 world;
	float4x4 normalMatrix;

	Material material;
};

// Given a normal returns the tangent and bitangent vectors
void ComputeBasisVectors(float3 normal, out float3 tangent, out float3 bitangent)
{
	float3 up = abs(normal.y) < 0.999f ? float3(0.f, 1.f, 0.f) : float3(1.f, 0.f, 0.f);

	tangent = normalize(cross(normal, up));
	
	bitangent = normalize(cross(normal, tangent));
}

float3 TangentToWorldSpace(float3 value, float3 normal, float3 tangent, float3 bitangent)
{
	return bitangent * value.x + tangent * value.y + normal * value.z;
}

float RadicalInverse_VdC(uint bits) 
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// http://holger.dammertz.org:80/stuff/notes_HammersleyOnHemisphere.html
float2 Hammersley(uint i, uint total)
{
	return float2(float(i) / float(total), RadicalInverse_VdC(i));
}

float3 HemisphereSample(float2 Xi, float3 normal)
{
	float phi = Xi.y * 2.f * PI;
	float cosTheta = 1.f - Xi.x;
	float sinTheta = sqrt(1.f - cosTheta * cosTheta);
	float3 sample = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
	
	float3 tangent, bitangent;
	ComputeBasisVectors(normal, tangent, bitangent);
	return TangentToWorldSpace(sample, normal, tangent, bitangent);
}
