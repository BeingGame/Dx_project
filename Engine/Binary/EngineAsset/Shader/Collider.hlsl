
#include "Share.hlsli"


cbuffer CBCollier : register(b10)
{
    float4 cbColliderColor;
}

float4 FrameVS( float3 pos : POSITION) : SV_POSITION
{
    float4 OutputPos = mul(float4(pos, 1.f), cbWVP);
    
    return OutputPos;
}

float4 FramePS() : SV_TARGET
{
    return cbColliderColor;
}