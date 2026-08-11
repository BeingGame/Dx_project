
#include "Share.hlsli"

VS_OUTPUT_TEX AnimationVS(VS_INPUT_TEX input)
{
    VS_OUTPUT_TEX output = (VS_OUTPUT_TEX) 0;
    
    float3 Pos = input.Pos;
    
    //정점의 크기를 ratio비율만큼 만들어준다.
    //ex)-0.5 ~ 0.5 * ratio(0.3,0.3) = -0.15 ~ 0.15 
    Pos.xy *= cbAnimRatio.xy;
    
    output.Pos = mul(float4(Pos, 1.f), cbWVP);
    output.UV = ComputeAnimation2DUV(input.UV);
    
    //여기있어도 상관은 없다.
    //Pos.xy *= cbAnimRatio.xy;
    
    return output;
}

PS_OUTPUT_COLOR2D AnimationPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR2D output = (PS_OUTPUT_COLOR2D) 0;
    
    float2 uv = input.UV;
    
    uv = UVScrolling(uv);
  
    float4 TextureColor = tbTexture.Sample(sbSampler, uv);
    
    //애니메이션 팔레트 스왑이 이뤄질때
    if (cbPaletteEnable == 1)
    {
        uint Width = 0, Height = 0;
        tbIndexTexture.GetDimensions(Width, Height);
        
        int3 LoadPos = int3(uv.x * Width, uv.y * Height, 0);
        uint Index = tbIndexTexture.Load(LoadPos);
        
        TextureColor = cbPaletteColor[Index];
    }
  
    
    ChromaKey(TextureColor.rgb);
    TextureColor = Dissolve(TextureColor, uv);
    TextureColor.rgb = HitEffect(TextureColor.rgb);
    
    output.Color.rgb = TextureColor.rgb * cbMatBaseColor.rgb;
    output.Color.a = TextureColor.a * cbMatOpacity;
    
    return output;
}
