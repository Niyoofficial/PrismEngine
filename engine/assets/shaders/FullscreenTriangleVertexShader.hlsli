struct FullscreenVertexOut
{
	float4 positionClip : SV_Position;
	noperspective float2 ndc : POSITION;
	noperspective float2 texCoords : TEXCOORD;
};

FullscreenVertexOut vsmain(uint vertexID : SV_VertexID)
{
	FullscreenVertexOut vout;
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
