
#include "Share.hlsli"

cbuffer CBUIDefault : register(b10)
{
    float4 cbBrushTint;
    float4 cbWidgetColor;
    float2 cbBrushLTUV;
    float2 cbBrushRBUV;
    int cbBrushAnimationEnable;
    int cbBrushTextureEnable;
    int cbBrushPercentEnable;
    float cbBrushPercent;
    int cbBrushBarDir;
    float3 cbBrushEmpty;
}

float2 UpdateAnimationUV(float2 UV)
{
    float2 Result = UV;
    
    if(cbBrushAnimationEnable == 0)
    {
        return Result;
    }
    
    if(UV.x == 0.f)
    {
        Result.x = cbBrushLTUV.x;
    }
    else
    {
        Result.x = cbBrushRBUV.x;
    }
    
    if (UV.y == 0.f)
    {
        Result.y = cbBrushLTUV.y;
    }
    else
    {
        Result.y = cbBrushRBUV.y;
    }
    
    return Result;
}


VS_OUTPUT_TEX UIDefaultVS(VS_INPUT_TEX input)
{
    VS_OUTPUT_TEX output = (VS_OUTPUT_TEX) 0;
    
    float3 Pos = input.Pos - cbPivotSize;
    
    output.Pos = mul(float4(Pos, 1.f), cbWVP);
    output.UV = UpdateAnimationUV(input.UV);
    
    return output;
}

PS_OUTPUT_COLOR2D UIDefaultPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR2D output = (PS_OUTPUT_COLOR2D) 0;
    
    
    //프로그레스바 일경우
    if(cbBrushPercentEnable ==1)
    {
        if(cbBrushBarDir == 0 && input.UV.x > cbBrushPercent)
        {
            discard;
        }
        else if(cbBrushBarDir ==1 && input.UV.y > cbBrushPercent)
        {
            discard;
        }
    }
    
    
    
    float4 Color = float4(1.f, 1.f, 1.f, 1.f);
    
    if(cbBrushTextureEnable == 1)
    {
        Color = tbTexture.Sample(sbSampler, input.UV);
    }
    
    output.Color = Color * cbWidgetColor * cbBrushTint;
    
    return output;
}