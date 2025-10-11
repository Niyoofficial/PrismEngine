#include "Common.hlsli"

cbuffer Resources
{
	int g_shadowSceneBuffer;
	int g_primitiveBuffer;
}

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
	float3 tangentlLocal : TANGENT;
	float3 bitangentlLocal : BITANGENT;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

struct PixelInput
{
	float4 positionClip : SV_POSITION;
};

PixelInput vsmain(VertexInput vin)
{
	ConstantBuffer<ShadowSceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_shadowSceneBuffer];
	ConstantBuffer<PrimitiveBuffer> primitiveBuffer = ResourceDescriptorHeap[g_primitiveBuffer];

	PixelInput vout;
	
	float4 posWorld = mul(primitiveBuffer.world, float4(vin.positionLocal, 1.f));
	vout.positionClip = mul(sceneBuffer.lightViewProj, posWorld);
	
	return vout;
}

void psmain(PixelInput pin)
{
}