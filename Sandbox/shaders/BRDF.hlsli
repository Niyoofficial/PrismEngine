#include "Common.hlsli"

// Dielectrics have a constant reflectivity of 0.04
static const float3 DIELECTRIC_F0 = 0.04f;

struct BRDFSurface
{
	float3 albedo;
	float metallic;
	float roughness;
	float3 normal;
};

float3 LambertDiffuse(float3 albedo)
{
	return albedo / PI;
}

// The Shlick approximation of the Fresnel equation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.f - F0) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max((float)(1.f - roughness), F0) - F0) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
}

// The Trowbridge-Reitz GGX NDF function
float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.f);
	float NdotH2 = NdotH * NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.f) + 1.f);
	denom = PI * denom * denom;
	
	return num / denom;
}

// The Schlick-GGX geometry function
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.f);
	float k = (r * r) / 8.f;

	float num = NdotV;
	float denom = NdotV * (1.f - k) + k;
	
	return num / denom;
}

// The Smith's geometry function combines Schlick-GGX for both the view and light directions
float GeometrySmith(float3 normal, float3 toCamera, float3 toLight, float roughness)
{
	float NdotV = max(dot(normal, toCamera), 0.f);
	float NdotL = max(dot(normal, toLight), 0.f);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	
	return ggx1 * ggx2;
}

// Importance sample function based on Epic Games implementation
float3 HemisphereSample_GGX(float2 Xi, float roughness, float3 normal)
{
	// Instead of uniformly or randomly (Monte Carlo) generating sample vectors over the integral's hemisphere, we'll generate 
	// sample vectors biased towards the general reflection orientation of the microsurface halfway vector based on the surface's roughness. 
	// This gives us a sample vector somewhat oriented around the expected microsurface's halfway vector based on some input roughness 
	// and the low-discrepancy sequence value Xi. Note that Epic Games uses the squared roughness for better visual results as based on 
	// Disney's original PBR research.
	// https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
	// https://www.tobias-franke.eu/log/2014/03/30/notes_on_importance_sampling.html
	
	float a = roughness * roughness;
	
	float phi = Xi.y * 2.f * PI;
	float cosTheta = sqrt((1.f - Xi.y) / (1.f + (a * a - 1.f) * Xi.y));
	float sinTheta = sqrt(1.f - cosTheta * cosTheta);
	
	float3 sample = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	float3 tangent, bitangent;
	ComputeBasisVectors(normal, tangent, bitangent);
	return TangentToWorldSpace(sample);
}

float CalcAttenuation(float distance)
{
	return 1.f / (distance * distance); // Inverse square law
}

// The Cook-Torrance specular BRDF and the Lambertian diffuse BRDF
float3 BRDF(BRDFSurface surface, float3 toLight, float3 toCamera)
{
	float3 halfVector = normalize(toCamera + toLight);
	float NdotH = saturate(dot(surface.normal, halfVector));
	float VdotH = saturate(dot(toCamera, halfVector));
	float NdotV = saturate(dot(surface.normal, toCamera));
	float NdotL = saturate(dot(surface.normal, toLight));

	// Metals have a reflectivity equal to their albedo
	float3 F0 = lerp(DIELECTRIC_F0, surface.albedo, surface.metallic);

	// DGF functions of the Cook-Torrance specular BRDF
	float distribution = DistributionGGX(surface.normal, halfVector, surface.roughness);
	float geometry = GeometrySmith(surface.normal, toCamera, toLight, surface.roughness);
	float3 fresnel = FresnelSchlick(max(dot(halfVector, toCamera), 0.f), F0);

	float3 numerator = distribution * geometry * fresnel;
	float denominator = 4.f * NdotV * NdotL + 0.0001f; // Small addition to avoid division by zero
	float3 specular = numerator / denominator;

	// Lambertian diffuse BRDF
	float3 kS = fresnel;
	float3 kD = 1.f - kS;
	// Metallic surfaces don't have diffuse lighting
	kD *= 1.f - surface.metallic;
	float3 diffuse = LambertDiffuse(surface.albedo) * kD;
	
	return diffuse + specular;
}

float3 EnvironmentBRDF(BRDFSurface surface, float3 toCamera)
{
	// Metals have a reflectivity equal to their albedo
	float3 F0 = lerp(DIELECTRIC_F0, surface.albedo, surface.metallic);
	
	float3 fresnel = FresnelSchlickRoughness(max(dot(surface.normal, toCamera), 0.f), F0, surface.roughness);
	
	float3 kS = fresnel;
	float3 kD = 1.f - kS;
	// Metallic surfaces don't have diffuse lighting
	kD *= 1.f - surface.metallic;
	
	return LambertDiffuse(surface.albedo) * kD;
}
