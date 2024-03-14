float4 vsmain() : SV_POSITION
{
    return float4(0.f, 0.f, 0.f, 1.f);
}

float4 psmain(float4 input : SV_POSITION) : SV_TARGET
{
    return float4(0.f, 0.f, 0.f, 1.f);
}
