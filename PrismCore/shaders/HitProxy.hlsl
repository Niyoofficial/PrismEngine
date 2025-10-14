#include "Common.hlsli"

cbuffer Resources
{
	int g_sceneBuffer;
	int g_primitiveBuffer;

	int g_hitProxiesTexture;
	int g_hitProxyOutputBuffer;
	int g_hitProxyReadSettingsBuffer;
}

struct SceneBuffer
{
	CameraInfo camera;
};

struct PrimitiveBuffer
{
	float4x4 world;
	uint hitProxyID;
};

struct ReadSettingsBuffer
{
	uint2 relMousePos;
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
	nointerpolation uint hitProxyID : HIT_PROXY_ID;
};

PixelInput vsmain(VertexInput vin)
{
	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];
	ConstantBuffer<PrimitiveBuffer> primitiveBuffer = ResourceDescriptorHeap[g_primitiveBuffer];

	PixelInput vout;
	
	float4 posWorld = mul(primitiveBuffer.world, float4(vin.positionLocal, 1.f));
	vout.positionClip = mul(sceneBuffer.camera.viewProj, posWorld);
	vout.hitProxyID = primitiveBuffer.hitProxyID;
	
	return vout;
}

uint psmain(PixelInput pin) : SV_TARGET
{
	return pin.hitProxyID;
}

[numthreads(1, 1, 1)]
void CsReadPixel()
{
	Texture2D<uint> hitProxiesTexture = ResourceDescriptorHeap[g_hitProxiesTexture];
	RWStructuredBuffer<uint> hitProxyOutput = ResourceDescriptorHeap[g_hitProxyOutputBuffer];
	ConstantBuffer<ReadSettingsBuffer> readSettings = ResourceDescriptorHeap[g_hitProxyReadSettingsBuffer];

	uint ID = hitProxiesTexture.Load(int3(readSettings.relMousePos, 0));
	hitProxyOutput[0] = ID;
}
