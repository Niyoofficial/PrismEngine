#include "Common.hlsli"
#include "BindlessResources.hlsli"

RESOURCE_INDICES(ShadowMapResources,
	int g_shadowSceneBuffer;
	int g_primitiveBuffer;
);

struct ShadowSceneBuffer
{
	float4x4 lightViewProj;
};

struct PrimitiveBuffer
{
	float4x4 world;
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
};

PixelInput vsmain(VertexInput vin)
{
    ConstantBuffer<ShadowSceneBuffer> shadowSceneBuffer = ResourceDescriptorHeap[Resources.g_shadowSceneBuffer];
    ConstantBuffer<PrimitiveBuffer> primitiveBuffer = ResourceDescriptorHeap[Resources.g_primitiveBuffer];

	PixelInput vout;
	
	float4 posWorld = mul(primitiveBuffer.world, float4(vin.positionLocal, 1.f));
	vout.positionClip = mul(shadowSceneBuffer.lightViewProj, posWorld);
	
	return vout;
}

void psmain(PixelInput pin)
{
}