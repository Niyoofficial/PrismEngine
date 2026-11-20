#include "Common.hlsli"
#include "BindlessResources.hlsli"

RESOURCE_INDICES(BasePassResources,
	int g_sceneBuffer;
	int g_primitiveBuffer;
	int g_albedoTexture;
	int g_metallicTexture;
	int g_roughnessTexture;
	int g_aoTexture;
	int g_normalTexture;
	int g_emissiveTexture;
);

struct SceneBuffer
{
	CameraInfo camera;
};

struct PrimitiveBuffer
{
	float4x4 world;
	float4x4 normalMatrix;
	
	Material material;
};

struct VertexInput
{
	float3 positionLocal : POSITION;
	float3 normalLocal : NORMAL;
	float2 texCoords : TEXCOORD;
	float3 tangentlLocal : TANGENT;
	float3 bitangentlLocal : BITANGENT;
};

struct PixelInput
{
	float4 positionClip : SV_POSITION;
	float3 positionWorld : POSITION;
	float3 normalWorld : NORMAL;
	float3 tangentWorld : TANGENT;
	float3 bitangentWorld : BITANGENT;
	float2 texCoords : TEXCOORD;
};

PixelInput vsmain(VertexInput vin)
{
    ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[Resources.g_sceneBuffer];
    ConstantBuffer<PrimitiveBuffer> primitiveBuffer = ResourceDescriptorHeap[Resources.g_primitiveBuffer];

	PixelInput vout;
	
	float4 posWorld = mul(primitiveBuffer.world, float4(vin.positionLocal, 1.f));
	vout.positionWorld = (float3)posWorld;
	vout.positionClip = mul(sceneBuffer.camera.viewProj, posWorld);

	vout.normalWorld = mul((float3x3)primitiveBuffer.normalMatrix, vin.normalLocal);
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
	ConstantBuffer<PrimitiveBuffer> primitiveBuffer = ResourceDescriptorHeap[Resources.g_primitiveBuffer];

	float3 normal = normalize(pin.normalWorld);
	float3 tangent = normalize(pin.tangentWorld);
	float3 bitangent = normalize(pin.bitangentWorld);
	
	float3 albedo = primitiveBuffer.material.albedo;
	float metallic = primitiveBuffer.material.metallic;
	float roughness = primitiveBuffer.material.roughness;
	float ao = primitiveBuffer.material.ao;
	float alpha = 1.f;
	if (Resources.g_albedoTexture != -1)
	{
		Texture2D albedoTexture = ResourceDescriptorHeap[Resources.g_albedoTexture];
		albedo *= albedoTexture.Sample(g_samLinearWrap, pin.texCoords).rgb;
		alpha = albedoTexture.Sample(g_samLinearWrap, pin.texCoords).a;
	}
	
	//clip(alpha - 0.1f);
	
	if (Resources.g_metallicTexture != -1)
	{
		Texture2D metallicTexture = ResourceDescriptorHeap[Resources.g_metallicTexture];
		metallic *= metallicTexture.Sample(g_samLinearWrap, pin.texCoords).b;
	}
	if (Resources.g_roughnessTexture != -1)
	{
		Texture2D roughnessTexture = ResourceDescriptorHeap[Resources.g_roughnessTexture];
		roughness *= roughnessTexture.Sample(g_samLinearWrap, pin.texCoords).g;
	}
	if (Resources.g_aoTexture != -1)
	{
		Texture2D aoTexture = ResourceDescriptorHeap[Resources.g_aoTexture];
		ao *= aoTexture.Sample(g_samLinearWrap, pin.texCoords).r;
	}
	if (Resources.g_normalTexture != -1)
	{
		Texture2D normalTexture = ResourceDescriptorHeap[Resources.g_normalTexture];
		normal = NormalSampleToWorldSpace(normalTexture.Sample(g_samLinearWrap, pin.texCoords).rgb, primitiveBuffer.normalMatrix, normal, tangent, bitangent);
	}
    if (Resources.g_emissiveTexture != -1)
    {
        Texture2D emissiveTexture = ResourceDescriptorHeap[Resources.g_normalTexture];
        //albedo = NormalSampleToWorldSpace(emissiveTexture.Sample(g_samLinearWrap, pin.texCoords).rgb, primitiveBuffer.normalMatrix, normal, tangent, bitangent);
    }

	PixelOutput pout;
	pout.color = float4(albedo, 1.f);
	pout.normals = float4(normal, 1.f);
	pout.roughnessMetalAO = float4(roughness, metallic, ao, 1.f);
	return pout;
}
