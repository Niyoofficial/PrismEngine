#include "BRDF.hlsli"

cbuffer Resources
{
	int g_sceneBuffer;
	int g_irradiance;
	int g_envMap;
	int g_brdfLUT;

	int g_colorTexture;
	int g_normalTexture;
	int g_roughnessMetalAOTexture;
	int g_depthTexture;
}

struct SceneBuffer
{
	CameraInfo camera;
};

struct SceneIrradiance
{
	SH::L2_RGB irradianceSH;
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

float4 psmain(VertexOut pin) : SV_Target
{
	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];
	ConstantBuffer<SceneIrradiance> sceneIrradiance = ResourceDescriptorHeap[g_irradiance];
	
	Texture2D normalTexture = ResourceDescriptorHeap[g_normalTexture];
	Texture2D roughnessMetalAOTexture = ResourceDescriptorHeap[g_roughnessMetalAOTexture];
	Texture2D albedoTexture = ResourceDescriptorHeap[g_colorTexture];
	Texture2D<float> depthTexture = ResourceDescriptorHeap[g_depthTexture];
	TextureCube prefilteredEnvMap = ResourceDescriptorHeap[g_envMap];
	Texture2D brdfLUT = ResourceDescriptorHeap[g_brdfLUT];

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
	
	float3 r = reflect(-toCamera, surface.normal);
	const int MAX_MIP_LEVEL = 5;
	float3 prefilteredSpecularColor = prefilteredEnvMap.Sample(g_samLinearWrap, r, surface.roughness * MAX_MIP_LEVEL).xyz;
	float2 F0ScaleBias = brdfLUT.SampleLevel(g_samLinearWrap, float2(dot(surface.normal, toCamera), surface.roughness), 0).xy;
	float3 diffuseIrradiance = SH::CalculateIrradiance(sceneIrradiance.irradianceSH, surface.normal); // Does the cosine lobe scale
	float3 envLight = EnvironmentBRDF(surface, toCamera, diffuseIrradiance, prefilteredSpecularColor, F0ScaleBias);
	
	float3 color = envLight * 0.25f;

	return float4(color, 1.f);
}
