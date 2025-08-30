#include "Common.hlsli"

cbuffer Resources
{
	int g_sceneBuffer;

	int g_modelBuffer;
	int g_albedoTexture;
	int g_metallicTexture;
	int g_roughnessTexture;
	int g_aoTexture;
	int g_normalTexture;
}

struct SceneBuffer
{
	CameraInfo camera;
};

struct ModelBuffer
{
	float4x4 world;
	float4x4 normalMatrix;
	
	float mipLevel;

	Material material;
};

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

struct PixelOutput
{
	float4 color : SV_TARGET0;
	float4 normals : SV_TARGET1;
	float4 roughnessMetalAO : SV_TARGET2;
};

PixelOutput psmain(PixelInput pin)
{
	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];
	ConstantBuffer<ModelBuffer> modelBuffer = ResourceDescriptorHeap[g_modelBuffer];
	
	float3 normal = normalize(pin.normalWorld);
	float3 tangent = normalize(pin.tangentWorld);
	float3 bitangent = normalize(pin.bitangentWorld);
	float3 toCamera = normalize(sceneBuffer.camera.camPos - pin.positionWorld);
	
	float3 albedo = modelBuffer.material.albedo;
	float metallic = modelBuffer.material.metallic;
	float roughness = modelBuffer.material.roughness;
	float ao = modelBuffer.material.ao;
	float alpha = 1.f;
	if (g_albedoTexture != -1)
	{
		Texture2D albedoTexture = ResourceDescriptorHeap[g_albedoTexture];
		albedo *= albedoTexture.Sample(g_samLinearWrap, pin.texCoords).rgb;
		alpha = albedoTexture.Sample(g_samLinearWrap, pin.texCoords).a;
	}
	
	clip(alpha - 0.1f);
	
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
	if (g_normalTexture != -1)
	{
		Texture2D normalTexture = ResourceDescriptorHeap[g_normalTexture];
		normal = NormalSampleToWorldSpace(normalTexture.Sample(g_samLinearWrap, pin.texCoords).rgb, modelBuffer.normalMatrix, normal, tangent, bitangent);
	}
	
	PixelOutput pout;
	pout.color = float4(albedo, 1.f);
	pout.normals = float4(normal, 1.f);
	pout.roughnessMetalAO = float4(roughness, metallic, ao, 1.f);
	return pout;
}
