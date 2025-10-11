#include "Common.hlsli"

cbuffer Resources
{
	int g_sceneBuffer;
	int g_skybox;
}

struct SceneBuffer
{
	float environmentDiffuseScale;
	CameraInfo camera;
	
	DirectionalLight directionalLights[MAX_LIGHT_COUNT];
	PointLight pointLights[MAX_LIGHT_COUNT];
};

struct VertexInput
{
	float3 positionLocal : POSITION;
	float3 normalLocal : NORMAL;
	float3 tangentlLocal : TANGENT;
	float3 color : COLOR;
	float2 texCoords : TEXCOORD;
};

struct PixelInputCube
{
	float4 positionClip : SV_POSITION;
	float3 positionLocal : POSITION;
};

PixelInputCube vsmain(VertexInput vin)
{
	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];

	PixelInputCube vout;
	
	vout.positionLocal = vin.positionLocal;
	
	vout.positionClip = mul(sceneBuffer.camera.viewProj, float4(vin.positionLocal + sceneBuffer.camera.camPos, 1)).xyww;
	
	return vout;
}

float4 psmain(PixelInputCube pin) : SV_TARGET
{
	TextureCube skybox = ResourceDescriptorHeap[g_skybox];

	float3 color = skybox.SampleLevel(g_samLinearWrap, pin.positionLocal, 0).rgb;
	
	return float4(color, 1.f);
}
