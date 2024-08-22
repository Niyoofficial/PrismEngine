SamplerState g_samPointWrap : register(s0);
SamplerState g_samPointClamp : register(s1);
SamplerState g_samLinearWrap : register(s2);
SamplerState g_samLinearClamp : register(s3);
SamplerState g_samAnisotropicWrap : register(s4);
SamplerState g_samAnisotropicClamp : register(s5);
SamplerComparisonState g_samShadow : register(s6);

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

cbuffer CameraBuffer : register(b0)
{
	float4x4 g_view;
	float4x4 g_proj;
	float4x4 g_viewProj;

	float3 g_camPos;
};

cbuffer ModelBuffer : register(b1)
{
	float4x4 g_world;
	
	float3 g_albedo;
	float g_metallic;
	float g_roughness;
};

StructuredBuffer<DirectionalLight> g_directionalLights;
StructuredBuffer<PointLight> g_pointLights;

struct VertexInput
{
	float3 positionLocal : POSITION;
	float3 normalLocal : NORMAL;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

struct PixelInput
{
	float4 positionClip : SV_POSITION;
	float3 positionWorld : POSITION;
	float3 normalWorld : NORMAL;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
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

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.f);
	float NdotL = max(dot(N, L), 0.f);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

PixelInput vsmain(VertexInput vin)
{
	PixelInput vout;
	
	float4 posWorld = mul(g_world, float4(vin.positionLocal, 1.f));
	vout.positionWorld = (float3)posWorld;
	vout.positionClip = mul(g_viewProj, posWorld);
	
	vout.normalWorld = mul((float3x3) g_world, vin.normalLocal);
	
	vout.texCoords = vin.texCoords;

	return vout;
}

float4 monkeypsmain(PixelInput pin) : SV_TARGET
{
	float3 albedo = float3(0.3f, 0.2f, 0.8f);
	float metallic = 0.2f;
	float roughness = 0.4f;
	
	float3 normal = normalize(pin.normalWorld);
	float3 toCamera = normalize(g_camPos - pin.positionWorld);
	
	float3 Lo = 0.f;
	
	{
		float3 lightPosition = float3(0.f, 2.f, 3.f);
		float3 lightColor = float3(0.9f, 0.9f, 0.9f);

		float3 toLight = normalize(lightPosition - pin.positionWorld);
		float3 halfVector = normalize(toCamera + toLight);

		float distance = length(lightPosition - pin.positionWorld);
		float attenuation = 1.f / (distance * distance); // Inverse square law
		float3 radiance = lightColor * attenuation;
		
		float3 F0 = 0.04f;
		F0 = lerp(F0, albedo, metallic);
		float3 fresnel = FresnelSchlick(max(dot(halfVector, toCamera), 0.f), F0);
		
		float distribution = DistributionGGX(normal, halfVector, roughness);
		float geometry = GeometrySmith(normal, toCamera, toLight, roughness);
		
		float3 numerator = distribution * geometry * fresnel;
		float denominator = 4.0 * max(dot(normal, toCamera), 0.0) * max(dot(normal, toLight), 0.f) + 0.0001;
		float3 specular = numerator / denominator;
		
		float3 kS = fresnel;
		float3 kD = (1.f - kS) * (1.f - metallic);
		
		const float PI = 3.14159265359f;
		
		float NdotL = max(dot(normal, toLight), 0.f);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	float ambient = 0.005f;
	float3 color = ambient * albedo + Lo;
	
	color = color / (color + 1.f);
	color = pow(color, 1.f / 2.2f);
	
	return float4(color, 1.f);
}

float4 floorpsmain(PixelInput pin) : SV_TARGET
{
	float3 albedo = float3(0.8f, 0.3f, 0.2f);
	float metallic = 0.2f;
	float roughness = 0.4f;
	
	float3 normal = normalize(pin.normalWorld);
	float3 toCamera = normalize(g_camPos - pin.positionWorld);
	
	float3 Lo = 0.f;
	
	{
		float3 lightPosition = float3(0.f, 2.f, 3.f);
		float3 lightColor = float3(0.9f, 0.9f, 0.9f);

		float3 toLight = normalize(lightPosition - pin.positionWorld);
		float3 halfVector = normalize(toCamera + toLight);

		float distance = length(lightPosition - pin.positionWorld);
		float attenuation = 1.f / (distance * distance); // Inverse square law
		float3 radiance = lightColor * attenuation;
		
		float3 F0 = 0.04f;
		F0 = lerp(F0, albedo, metallic);
		float3 fresnel = FresnelSchlick(max(dot(halfVector, toCamera), 0.f), F0);
		
		float distribution = DistributionGGX(normal, halfVector, roughness);
		float geometry = GeometrySmith(normal, toCamera, toLight, roughness);
		
		float3 numerator = distribution * geometry * fresnel;
		float denominator = 4.0 * max(dot(normal, toCamera), 0.0) * max(dot(normal, toLight), 0.0) + 0.0001;
		float3 specular = numerator / denominator;
		
		float3 kS = fresnel;
		float3 kD = (1.f - kS) * (1.f - metallic);
		
		const float PI = 3.14159265359f;
		
		float NdotL = max(dot(normal, toLight), 0.f);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	float ambient = 0.005f;
	float3 color = ambient * albedo + Lo;
	
	color = color / (color + 1.f);
	color = pow(color, 1.f / 2.2f);
	
	return float4(color, 1.f);
}
