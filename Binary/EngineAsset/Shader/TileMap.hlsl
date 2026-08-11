
#include "Share.hlsli"

cbuffer CBTileMap : register(b10)
{
    float2 cbTileLTUV;
    float2 cbTileRBUV;
}

float2 ComputeTileUV(float2 UV)
{
    float2 Result = (float2) 0.f;
    
    if (UV.x > 0.f)
    {
        Result.x = cbTileRBUV.x;
    }
    else
    {
        Result.x = cbTileLTUV.x;
    }
    
    if (UV.y > 0.f)
    {
        Result.y = cbTileLTUV.y;
    }
    else
    {
        Result.y = cbTileRBUV.y;
    }
    
    return Result;
}

Texture2D tbTileTexture : register(t1);

VS_OUTPUT_TEX TileMapVS(VS_INPUT_TEX input)
{
    VS_OUTPUT_TEX output = (VS_OUTPUT_TEX) 0;

    output.Pos = mul(float4(input.Pos, 1.f), cbWVP);
    output.UV = ComputeTileUV(input.UV);
    
    return output;
}

PS_OUTPUT_COLOR2D TileMapPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR2D output = (PS_OUTPUT_COLOR2D) 0;
    
    output.Color = tbTileTexture.Sample(sbSampler, input.UV);
    
    return output;
}
