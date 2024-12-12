#include "Common.hlsl"

Texture2D g_albedoTexture;
Texture2D g_metallicTexture;
Texture2D g_roughnessTexture;
Texture2D g_aoTexture;
Texture2D g_normalTexture;
TextureCube g_irradianceMap;

struct VertexInput
{
	float3 positionLocal : POSITION;
	float3 normalLocal : NORMAL;
	float3 tangentlLocal : TANGENT;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

struct PixelInput
{
	float4 positionClip : SV_POSITION;
	float3 positionWorld : POSITION;
	float3 normalWorld : NORMAL;
	float3 tangentWorld : TANGENT;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max((float)(1.f - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	const float PI = 3.14159265359f;

	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.f);
	float NdotH2 = NdotH * NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.f) + 1.f);
	denom = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.f);
	float k = (r * r) / 8.f;

	float num = NdotV;
	float denom = NdotV * (1.f - k) + k;
	
	return num / denom;
}

float GeometrySmith(float3 normal, float3 toCamera, float3 toLight, float roughness)
{
	float NdotV = max(dot(normal, toCamera), 0.f);
	float NdotL = max(dot(normal, toLight), 0.f);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	
	return ggx1 * ggx2;
}

float CalcAttenuation(float distance)
{
	return 1.f / (distance * distance); // Inverse square law
}

float3 CalcLight(float3 albedo, float metallic, float roughness, float3 lightColor, float3 toLight, float3 toCamera, float3 normal, float attenuation, float3 F0)
{
	float3 halfVector = normalize(toCamera + toLight);

	float3 fresnel = FresnelSchlick(max(dot(halfVector, toCamera), 0.f), F0);

	float distribution = DistributionGGX(normal, halfVector, roughness);
	float geometry = GeometrySmith(normal, toCamera, toLight, roughness);

	float3 numerator = distribution * geometry * fresnel;
	float denominator = 4.f * max(dot(normal, toCamera), 0.f) * max(dot(normal, toLight), 0.f) + 0.0001f;
	float3 specular = numerator / denominator;

	float3 kS = fresnel;
	float3 kD = (1.f - kS) * (1.f - metallic);
	
	float3 radiance = lightColor * attenuation;
		
	const float PI = 3.14159265359f;
		
	float NdotL = max(dot(normal, toLight), 0.f);
	return (kD * albedo / PI + specular) * radiance * NdotL;
}

float3 NormalSampleToWorldSpace(float3 normalSample, float3 normal, float3 tangent)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalSampleUncomp = 2.f * normalSample - 1.f;
	
    float3 biTan = cross(normal, tangent);
    return normalize(biTan * normalSampleUncomp.x + tangent * normalSampleUncomp.y + normal * normalSampleUncomp.z);
}

PixelInput vsmain(VertexInput vin)
{
	PixelInput vout;
	
	float4 posWorld = mul(g_world, float4(vin.positionLocal, 1.f));
	vout.positionWorld = (float3)posWorld;
	vout.positionClip = mul(g_camera.viewProj, posWorld);
	
	vout.normalWorld = mul((float3x3) g_world, vin.normalLocal);
	vout.tangentWorld = mul((float3x3) g_world, vin.tangentlLocal);
	
	vout.texCoords = vin.texCoords;

	return vout;
}

float4 psmain(PixelInput pin) : SV_TARGET
{
	float3 normal = normalize(pin.normalWorld);
	float3 tangent = normalize(pin.tangentWorld);
	float3 toCamera = normalize(g_camera.camPos - pin.positionWorld);
	
	float3 albedo = g_material.albedo;
	float metallic = g_material.metallic;
	float roughness = g_material.roughness;
	float ao = g_material.ao;
	if (g_useAlbedoTexture)
		albedo *= g_albedoTexture.Sample(g_samLinearClamp, pin.texCoords).rgb;
	if (g_useMetallicTexture)
		metallic *= g_metallicTexture.Sample(g_samLinearClamp, pin.texCoords).r;
	if (g_useRoughnessTexture)
		roughness *= g_roughnessTexture.Sample(g_samLinearClamp, pin.texCoords).r;
	if (g_useAoTexture)
		ao *= g_aoTexture.Sample(g_samLinearClamp, pin.texCoords).r;
	if (g_useNormalTexture)
		normal = NormalSampleToWorldSpace(g_normalTexture.Sample(g_samLinearClamp, pin.texCoords).rgb, normal, tangent);
	
    float3 F0 = 0.04f;
    F0 = lerp(F0, albedo, metallic);
	
	float3 Lo = 0.f;
	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		if (length(g_pointLights[i].position) <= 0.f)
			break;

		float3 lightPosition = g_pointLights[i].position;
		float distance = length(lightPosition - pin.positionWorld);
		float3 toLight = normalize(lightPosition - pin.positionWorld);
		
		Lo += CalcLight(albedo, metallic, roughness, g_pointLights[i].lightColor, toLight, toCamera, normal, CalcAttenuation(distance), F0);
	}
	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		if (length(g_directionalLights[i].direction) <= 0.f)
			break;

		float3 toLight = normalize(-g_directionalLights[i].direction);
		
		Lo += CalcLight(albedo, metallic, roughness, g_directionalLights[i].lightColor, toLight, toCamera, normal, 1.f, F0);
	}
	
    float3 kS = FresnelSchlickRoughness(max(dot(normal, toCamera), 0.f), F0, roughness);
    float3 kD = 1.f - kS;
    kD *= 1.f - metallic;
    float3 irradiance = g_irradianceMap.Sample(g_samLinearClamp, normal).rgb;
    float3 diffuse = irradiance * albedo;
    float3 ambient = kD * diffuse * ao;
	
	float3 color = ambient + Lo;
	
	// Gamma correction
	color = color / (color + 1.f);
	color = pow(color, 1.f / 2.2f);
	
	return float4(color, 1.f);
}
