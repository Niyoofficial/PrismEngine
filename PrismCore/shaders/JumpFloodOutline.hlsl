#include "Common.hlsli"

cbuffer Resources
{
	int g_sceneBuffer;
	int g_primitiveBuffer;
	int g_inputMask;
	int g_outputMask;
	int g_jumpFloodSettings;
}

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

struct JumpFloodSettings
{
	int stepWidth;
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
	ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[g_sceneBuffer];
	ConstantBuffer<PrimitiveBuffer> primitiveBuffer = ResourceDescriptorHeap[g_primitiveBuffer];

	PixelInput vout;
	
	float4 posWorld = mul(primitiveBuffer.world, float4(vin.positionLocal, 1.f));
	vout.positionClip = mul(sceneBuffer.camera.viewProj, posWorld);
	
	return vout;
}

float PsMask(PixelInput pin) : SV_TARGET
{
	return 1.f;
}

[numthreads(4, 4, 1)]
void CsInitMask(int3 dispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float> inputMask = ResourceDescriptorHeap[g_inputMask];
	RWTexture2D<float2> outputMask = ResourceDescriptorHeap[g_outputMask];

	uint2 texSize;
	inputMask.GetDimensions(texSize.x, texSize.y);

	if (inputMask.Load(int3(dispatchThreadID.xy, 0)).r)
	{
		outputMask[dispatchThreadID.xy] = 0.f;
		return;
	}

	half3x3 values;
	for (int u = 0; u < 3; ++u)
	{
		for (int v = 0; v < 3; ++v)
		{
			int2 sampleUV = dispatchThreadID.xy + int2(u - 1, v - 1);
			values[u][v] = (half)inputMask.Load(int3(sampleUV, 0)).r;
		}
	}

	float2 dir = -float2(
		values[0][0] + values[0][1] * 2.0 + values[0][2] - values[2][0] - values[2][1] * 2.0 - values[2][2],
		values[0][0] + values[1][0] * 2.0 + values[2][0] - values[0][2] - values[1][2] * 2.0 - values[2][2]
	);

	if (abs(dir.x) <= 0.005f && abs(dir.y) <= 0.005f)
	{
		outputMask[dispatchThreadID.xy] = 0.f;
		return;
	}

	dir = normalize(dir);

	outputMask[dispatchThreadID.xy] = ((float2)dispatchThreadID.xy + dir) / texSize;
}

[numthreads(4, 4, 1)]
void CsJumpFlood(int3 dispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float2> inputMask = ResourceDescriptorHeap[g_inputMask];
	RWTexture2D<float2> outputMask = ResourceDescriptorHeap[g_outputMask];
	ConstantBuffer<JumpFloodSettings> jumpFloodSettings = ResourceDescriptorHeap[g_jumpFloodSettings];

	uint2 texSize;
	inputMask.GetDimensions(texSize.x, texSize.y);

	// initialize best distance at infinity
	float bestDist = 1.#INF;
	float2 bestCoord;

	// jump samples
	for (int u = -1; u <= 1; ++u)
	{
		for (int v = -1; v <= 1; ++v)
		{
			// calculate offset sample position
			int2 offsetUV = dispatchThreadID.xy + int2(u, v) * jumpFloodSettings.stepWidth;
			
			// decode position from buffer
			float2 offsetPos = inputMask.Load(int3(offsetUV, 0)).rg * texSize;

			// the offset from current position
			float2 disp = (float2)dispatchThreadID.xy - offsetPos;

			// square distance
			float dist = dot(disp, disp);

			// if offset position isn't a null position or is closer than the best
			// set as the new best and store the position
			if (offsetPos.y != 0.f && dist < bestDist)
			{
				bestDist = dist;
				bestCoord = offsetPos;
			}
		}
	}

	// if not valid best distance output null position, otherwise output encoded position
	outputMask[dispatchThreadID.xy] = isinf(bestDist) ? float2(0.f, 0.f) : bestCoord / texSize;
}
