struct VertexInput
{
	float3 positionLocal : POSITION;
	float3 color : COLOR;
};

struct PixelInput
{
	float4 positionClip : SV_POSITION;
	float3 color : COLOR;
};

PixelInput vsmain(VertexInput vin)
{
	PixelInput vout;
    vout.positionClip = float4(vin.positionLocal, 1.f);
    vout.color = vin.color;

	return vout;
}

float4 psmain(PixelInput pin) : SV_TARGET
{
    return float4(pin.color, 1.f);
}
