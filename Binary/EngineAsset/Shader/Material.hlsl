
#include "Share.hlsli"

VS_OUTPUT_TEX MaterialVS(VS_INPUT_TEX input)
{
    VS_OUTPUT_TEX output = (VS_OUTPUT_TEX) 0;
    
    output.Pos = mul(float4(input.Pos, 1.f), cbWVP);
    output.UV = input.UV;
   
    return output;
}

PS_OUTPUT_COLOR2D DefaultMaterialPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR2D output = (PS_OUTPUT_COLOR2D) 0;
    
    float2 uv = input.UV;
    
    //if (cbUVScrollEnable == 1)
    //{
    //    uv += cbUVScrollSpeed * cbUVScrollTime;
    //}
      
    uv = UVScrolling(uv);
  
    float4 TextureColor = tbTexture.Sample(sbSampler, uv);
    
    ChromaKey(TextureColor.rgb);
    TextureColor = Dissolve(TextureColor, uv);
    TextureColor.rgb = HitEffect(TextureColor.rgb);
    //크로마키가 활성화 되어있을때
    //if (cbChromakeyEnable == 1)
    //{
    //    float dist = distance(TextureColor.rgb, cbChromaKeyColor);
       
    //    //discard
    //    //픽셀을 그리지않고 넘어간다.
    //    //clip(0미만값) discard랑 동일한 역할을 한다.
    
    //    if (dist < cbChromaKeyThreshold)
    //    {
    //        discard;
    //    }
    //}
    
    ////디졸브
    ////텍스처 하나를 더 이용해서 구멍이 뚤리듯이 사라지는 효과
    //if (cbDissolveEnable == 1)
    //{
    //    float noise = tbDissolveTexture.Sample(sbSampler, uv).r;
    //    //디졸브효과를 적용하기위해서 텍스처의 r값이 progress보다 작으면 discard
    //    if (noise < cbDissolveProgress)
    //    {
    //        discard;
    //    }
    //    else if (noise < cbDissolveProgress + cbDissolveEdgeWidth)
    //    {
    //        TextureColor.rgb = cbDissolveEdgeColor.rgb;
    //    }
    //}
    
    ////Hit Effect
    ////lerp : 보간함수
    ////t라는 비율이 있고 값이 x0,x1
    ////y = x0(1-t) + x1*t;
    //if (cbHitEffectEnable == 1)
    //{
    //    TextureColor.rgb = lerp(TextureColor.rgb, cbHitColor.rgb, cbHitIntensity);
    //}
   
    output.Color.rgb = TextureColor.rgb * cbMatBaseColor.rgb;
    output.Color.a = TextureColor.a * cbMatOpacity;

    //output.Color = float4(1.f, 0.f, 0.f, 1.f);

    return output;
}

PS_OUTPUT_COLOR2D TexturePS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR2D output = (PS_OUTPUT_COLOR2D) 0;

    float4 TextureColor = tbTexture.Sample(sbSampler, input.UV);

    output.Color.rgb = TextureColor.rgb * cbMatBaseColor.rgb;
    output.Color.a = TextureColor.a * cbMatOpacity;
    
    return output;
}