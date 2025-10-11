#include "BRDF.hlsli"

cbuffer Resources
{
	int g_sceneBuffer;
	int g_dirLightPassBuffer;
	int g_shadowMap;

	int g_colorTexture;
	int g_normalTexture;
	int g_roughnessMetalAOTexture;
	int g_depthTexture;
}

struct SceneBuffer
{
	CameraInfo camera;
};

struct DirectionalLightPassBuffer
{
	DirectionalLight dirLight;
	float4x4 shadowViewProj;
};

struct VertexOut
{
	float4 positionClip : SV_Position;
	float2 ndc : POSITION;
	float2 texCoords : TEXCOORD;
};

VertexOut vsmain(uint vertexID : SV_VertexID)
{
	VertexOut vout;
	if (vertexID == 0)   
	{
		float2 vertex = float2(1.f, -1.f);
		vout.positionClip = float4(vertex, 0.f, 1.f);
		vout.ndc = vertex;
		vout.texCoords = float2(1.f, 1.f);
	}
	else if (vertexID == 1)
	{
		float2 vertex = float2(1.f, 3.f);
		vout.positionClip = float4(vertex, 0.f, 1.f);
		vout.ndc = vertex;
		vout.texCoords = float2(1.f, -1.f);
	}
	else if (vertexID == 2)
	{
		float2 vertex = float2(-3.f, -1.f);
		vout.positionClip = float4(vertex, 0.f, 1.f);
		vout.ndc = vertex;
		vout.texCoords = float2(-1.f, 1.f);
	}
	
	return vout;
}

float3 CalcLight(BRDFSurface surface, float3 lightColor, float3 toLight, float3 toCamera, float attenuation)
{
	float NdotL = saturate(dot(surface.normal, toLight));
	return BRDF(surface, toLight, toCamera) * lightColor * attenuation * NdotL;
}

float4 psmain(VertexOut pin) : SV_Target
{
	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];
	ConstantBuffer<DirectionalLightPassBuffer> dirLightPassBuffer = ResourceDescriptorHeap[g_dirLightPassBuffer];
	
	Texture2D normalTexture = ResourceDescriptorHeap[g_normalTexture];
	Texture2D roughnessMetalAOTexture = ResourceDescriptorHeap[g_roughnessMetalAOTexture];
	Texture2D albedoTexture = ResourceDescriptorHeap[g_colorTexture];
	Texture2D<float> depthTexture = ResourceDescriptorHeap[g_depthTexture];
	Texture2D shadowMap = ResourceDescriptorHeap[g_shadowMap];

	BRDFSurface surface;
	surface.albedo = albedoTexture.Sample(g_samLinearWrap, pin.texCoords).rgb;
	float3 rougnessMetalAO = roughnessMetalAOTexture.Sample(g_samLinearWrap, pin.texCoords).rgb;
	surface.roughness = rougnessMetalAO.r;
	surface.metallic = rougnessMetalAO.g;
	surface.normal = normalTexture.Sample(g_samLinearWrap, pin.texCoords).rgb;
	
	float4 projectedPos = float4(pin.ndc.x, pin.ndc.y, depthTexture.Sample(g_samLinearWrap, pin.texCoords), 1.f);
	float4 positionWorld =  mul(sceneBuffer.camera.invViewProj, projectedPos);
	positionWorld.xyz /= positionWorld.w;
	
	float3 toCamera = normalize(sceneBuffer.camera.camPos - positionWorld.xyz);

	// For now only a single directional light has shadow
	float4 shadowPosClip = mul(dirLightPassBuffer.shadowViewProj, float4(positionWorld.xyz, 1.f)); // Hardcoding w = 1 because the shadow map was generated from orthographic projection
																								   //i.e. w has to be 1 while for perspective w = 0..1 depending on distance from camera
	float shadowFactor = CalcShadowFactor(shadowPosClip, 8192, shadowMap, g_samShadow);

	float3 toLight = normalize(-dirLightPassBuffer.dirLight.direction);

	float3 analyticLight = shadowFactor * CalcLight(surface, dirLightPassBuffer.dirLight.lightColor, toLight, toCamera, 1.f);
	
	float3 Lo = analyticLight;
	
	float3 color = Lo;
	
	return float4(color, 1.f);
}
