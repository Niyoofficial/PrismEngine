#include "BRDF.hlsli"

cbuffer Resources
{
	int g_sceneBuffer;
	int g_irradiance;
	int g_envMap;
	int g_brdfLUT;
	int g_modelBuffer;

	int g_albedoTexture;
	int g_metallicTexture;
	int g_roughnessTexture;
	int g_aoTexture;
	int g_normalTexture;
}

struct VertexInput
{
	float3 positionLocal : POSITION;
	float3 normalLocal : NORMAL;
	float3 tangentlLocal : TANGENT;
	float3 bitangentlLocal : BITANGENT;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

struct PixelInput
{
	float4 positionClip : SV_POSITION;
	float3 positionWorld : POSITION;
	float3 normalWorld : NORMAL;
	float3 tangentWorld : TANGENT;
	float3 bitangentWorld : BITANGENT;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

float3 CalcLight(BRDFSurface surface, float3 lightColor, float3 toLight, float3 toCamera, float attenuation)
{
	float NdotL = saturate(dot(surface.normal, toLight));
	return BRDF(surface, toLight, toCamera) * lightColor * attenuation * NdotL;
}

float3 NormalSampleToWorldSpace(float3 normalSample, float4x4 normalMatrix, float3 normal, float3 tangent, float3 bitangent)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalSampleUncomp = 2.f * normalSample - 1.f;
	
	normal = normalize(mul((float3x3)normalMatrix, normal));
	tangent = normalize(mul((float3x3)normalMatrix, tangent));
	bitangent = normalize(mul((float3x3)normalMatrix, bitangent));
	
	float3x3 TBN = float3x3(tangent, bitangent, normal);

	return normalize(mul(normalSampleUncomp, TBN));
}

PixelInput vsmain(VertexInput vin)
{
	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];
	ConstantBuffer<ModelBuffer> modelBuffer = ResourceDescriptorHeap[g_modelBuffer];

	PixelInput vout;
	
	float4 posWorld = mul(modelBuffer.world, float4(vin.positionLocal, 1.f));
	vout.positionWorld = (float3)posWorld;
	vout.positionClip = mul(sceneBuffer.camera.viewProj, posWorld);
	
	vout.normalWorld = vin.normalLocal;
	vout.tangentWorld = vin.tangentlLocal;
	vout.bitangentWorld = vin.bitangentlLocal;
	
	vout.texCoords = vin.texCoords;

	return vout;
}

float4 spheremain(PixelInput pin) : SV_TARGET
{
   	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];
	ConstantBuffer<ModelBuffer> modelBuffer = ResourceDescriptorHeap[g_modelBuffer];
	ConstantBuffer<SceneIrradiance> sceneIrradiance = ResourceDescriptorHeap[g_irradiance];
	TextureCube envMap = ResourceDescriptorHeap[g_envMap];
	
	float3 toCamera = normalize(sceneBuffer.camera.camPos - pin.positionWorld);
	float3 normal = normalize(pin.normalWorld);
	
	float3 L = normalize(2.f * dot(toCamera, normal) * normal - toCamera);
	L = 2.f * normal * dot(toCamera, normal) - toCamera;
	float a = 0.f;
	switch (modelBuffer.mipLevel)
	{
	case 0: a = 0.0f; break;
	case 1: a = 0.2f; break;
	case 2: a = 0.4f; break;
	case 3: a = 0.6f; break;
	case 4: a = 0.8f; break;
	case 5: a = 1.f; break;
	}
	a = a * a;
	L = lerp(normal, L, (1.f - a) * (sqrt(1.f - a)) + a);

	float3 color = envMap.SampleLevel(g_samLinearWrap, L, modelBuffer.mipLevel).rgb;
	
	// Gamma correction
	color = color / (color + 1.f);
	color = pow(color, 1.f / 2.2f);
	
	//color = float3(1.f, 0.f, 0.f);
	
	return float4(color, 1.f);
}

float4 psmain(PixelInput pin) : SV_TARGET
{
	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];
	ConstantBuffer<ModelBuffer> modelBuffer = ResourceDescriptorHeap[g_modelBuffer];
	ConstantBuffer<SceneIrradiance> sceneIrradiance = ResourceDescriptorHeap[g_irradiance];
	
	TextureCube prefilteredEnvMap = ResourceDescriptorHeap[g_envMap];
	Texture2D brdfLUT = ResourceDescriptorHeap[g_brdfLUT];

	float3 normal = normalize(pin.normalWorld);
	float3 tangent = normalize(pin.tangentWorld);
	float3 bitangent = normalize(pin.bitangentWorld);
	float3 toCamera = normalize(sceneBuffer.camera.camPos - pin.positionWorld);
	
	float3 albedo = modelBuffer.material.albedo;
	float metallic = modelBuffer.material.metallic;
	float roughness = modelBuffer.material.roughness;
	float ao = modelBuffer.material.ao;
	if (g_albedoTexture != -1)
	{
		Texture2D albedoTexture = ResourceDescriptorHeap[g_albedoTexture];
		albedo *= albedoTexture.Sample(g_samLinearWrap, pin.texCoords).rgb;
	}
	if (g_metallicTexture != -1)
	{
		Texture2D metallicTexture = ResourceDescriptorHeap[g_metallicTexture];
		metallic *= metallicTexture.Sample(g_samLinearWrap, pin.texCoords).r;
	}
	if (g_roughnessTexture != -1)
	{
		Texture2D roughnessTexture = ResourceDescriptorHeap[g_roughnessTexture];
		roughness *= roughnessTexture.Sample(g_samLinearWrap, pin.texCoords).g;
	}
	if (g_aoTexture != -1)
	{
		Texture2D aoTexture = ResourceDescriptorHeap[g_aoTexture];
		ao *= aoTexture.Sample(g_samLinearWrap, pin.texCoords).r;
	}
	if (g_normalTexture != -1 && false)
	{
		Texture2D normalTexture = ResourceDescriptorHeap[g_normalTexture];
		normal = NormalSampleToWorldSpace(normalTexture.Sample(g_samLinearWrap, pin.texCoords).rgb, modelBuffer.normalMatrix, normal, tangent, bitangent);
	}
	
	BRDFSurface surface = { albedo, metallic, roughness, normal };

	float3 analyticLight = 0.f;
	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		if (length(sceneBuffer.pointLights[i].position) <= 0.f)
			break;

		float3 lightPosition = sceneBuffer.pointLights[i].position;
		float distance = length(lightPosition - pin.positionWorld);
		float3 toLight = normalize(lightPosition - pin.positionWorld);

		analyticLight += CalcLight(surface, sceneBuffer.pointLights[i].lightColor,
						toLight, toCamera, CalcAttenuation(distance));
	}
	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		if (length(sceneBuffer.directionalLights[i].direction) <= 0.f)
			break;

		float3 toLight = normalize(-sceneBuffer.directionalLights[i].direction);

		analyticLight += CalcLight(surface, sceneBuffer.directionalLights[i].lightColor,
						toLight, toCamera, 1.f);
	}
	
	float3 r = reflect(-toCamera, normal);
	float3 prefilteredSpecularColor = prefilteredEnvMap.Sample(g_samLinearWrap, r, roughness * 5).xyz;
	float2 F0ScaleBias = brdfLUT.SampleLevel(g_samLinearWrap, float2(dot(normal, toCamera), roughness), 0).xy;
	float3 diffuseIrradiance = SH::CalculateIrradiance(sceneIrradiance.irradianceSH, normal); // Does the cosine lobe scale
	float3 envLight = EnvironmentBRDF(surface, toCamera, diffuseIrradiance, prefilteredSpecularColor, F0ScaleBias);
	
	float3 Lo = analyticLight + envLight;
	
	float3 color = Lo;

	//float3 kS = FresnelSchlick(max(dot(normal, toCamera), 0.f), F0);
	//float3 kD = 1.f - kS; // TODO: More accurate energy conservation http://jbit.net/~sparky/academic/mm_brdf.pdf
	//float3 diffuse = irradiance * albedo;
	//float3 ambient = kD * diffuse * ao;
	
	/*float3 Lo = 0.f;
	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		if (length(sceneBuffer.pointLights[i].position) <= 0.f)
			break;

		float3 lightPosition = sceneBuffer.pointLights[i].position;
		float distance = length(lightPosition - pin.positionWorld);
		float3 toLight = normalize(lightPosition - pin.positionWorld);

		Lo += CalcLight(albedo, metallic, roughness, sceneBuffer.pointLights[i].lightColor,
						toLight, toCamera, normal, CalcAttenuation(distance), F0);
	}
	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		if (length(sceneBuffer.directionalLights[i].direction) <= 0.f)
			break;

		float3 toLight = normalize(-sceneBuffer.directionalLights[i].direction);

		Lo += CalcLight(albedo, metallic, roughness, sceneBuffer.directionalLights[i].lightColor,
						toLight, toCamera, normal, 1.f, F0);
	}
	
	float3 kS = FresnelSchlickRoughness(max(dot(normal, toCamera), 0.f), F0, roughness);
	float3 kD = 1.f - kS;
	kD *= 1.f - metallic; // Metallic surfaces don't have diffuse lighting, so the more mettalic the surface is the less diffuse it gets
	float3 irradiance = SH::CalculateIrradiance(sceneIrradiance.irradianceSH, normal) * (1.f / PI);
	float3 diffuse = irradiance * albedo;
	float3 ambient = kD * diffuse * ao;
	
	float3 color = ambient + Lo;*/
	
	// Gamma correction
	color = color / (color + 1.f);
	color = pow(color, 1.f / 2.2f);
	
	return float4(color, 1.f);
}
