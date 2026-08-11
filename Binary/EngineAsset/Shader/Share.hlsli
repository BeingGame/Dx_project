



//텍스처용 구조체를 만들어준다.

struct VS_INPUT_TEX
{
    float3 Pos : POSITION;
    float2 UV : TEXCOORD;
};

struct VS_OUTPUT_TEX
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
};

struct PS_OUTPUT_COLOR2D
{
    //SV_Target 픽셀셰이더의 최종 데이터 출력값이 담기는 레지스터
    float4 Color : SV_TARGET;
};

//레지스터에 등록된 텍스처를 가져온다.
//t레지스터가 텍스처가 등록되는 레지스터.
Texture2D tbTexture : register(t0);

//레지스터에 등록된 샘플러를 가져온다.
//s레지스터가 샘플러가 등록되는 레지스터
SamplerState sbSampler : register(s0);

//상수버퍼
//상수버퍼에 입력한 레지스터는 b 레지스터 뒤에 붙는 숫자는 C++에서 입력한 숫자와 동일해야한다.
//b1, b2
//레지스터에 남아있는값은 자동적으로 초기화되지 않는다.

//HLSL코드에서의 상수버퍼 정의 방법
//C++의 구조체와 동일한 순서대로 데이터를 정의해야된다.
cbuffer CBTransform : register(b0)
{
    matrix cbWorld;
    matrix cbView;
    matrix cbProj;
    matrix cbWV;
    matrix cbWVP;
    matrix cbVP;
    float3 cbPivotSize;
    float cbTransformEmpty;
}

Texture2D tbDissolveTexture : register(t1);
Texture2D<uint> tbIndexTexture : register(t2);

cbuffer CBMaterial : register(b1)
{
    float4 cbMatBaseColor;
    float4 cbDissolveEdgeColor;
    float4 cbHitColor;
    
    int cbUVScrollEnable;
    float2 cbUVScrollSpeed;
    float cbUVScrollTime;
    
    int cbChromakeyEnable;
    float3 cbChromaKeyColor;
    float cbChromaKeyThreshold;
    
    int cbDissolveEnable;
    float cbDissolveProgress;
    float cbDissolveEdgeWidth;
    
    int cbHitEffectEnable;
    float cbHitIntensity;
    
    float cbMatOpacity;
    float cbMatEmpty;
}

//애니메이션 상수버퍼
cbuffer CBAnimation2D : register(b2)
{
    //LeftTop
    float2 cbLTUV;
    //RightBottom
    float2 cbRBUV;
    int cbAnimation2DEnable;
    int cbAnimation2DTextureType;
    int cbTextureSymmetry;
    int cbAnimation2DFrame;
    float2 cbAnimRatio;
    float2 cbAnimOffset;
}

//팔레트 상수버퍼
cbuffer CBPalette : register(b3)
{
    float4 cbPaletteColor[256];
    int cbPaletteEnable;
    float3 cbPaletteEmpty;
}

//HLSL의 함수는 인라인 함수처럼 사용된다.


//애니메이션 UV를 계산하기 위한 함수
float2 ComputeAnimation2DUV(float2 UV)
{
    //반환할 UV 값
    float2 Result = UV;
    
    //스프라이트 시트가 아니거나, AnimationEnable이 false(0)일때
    //그냥 UV를 반환해준다.
    if (cbAnimation2DEnable == 0 || cbAnimation2DTextureType != 0)
    {
        if (cbTextureSymmetry == 1)
        {
            if (UV.x == 1.f)
            {
                Result.x = 0.f;
            }
            else
            {
                Result.x = 1.f;
            }
        }
    }
    else
    {
        //스프라이트일 경우
        //LTUV와 RBUV 계산을 진행한다.
        
        if (UV.x == 0.f)
        {
            if (cbTextureSymmetry == 1)
            {
                Result.x = cbRBUV.x;
            }
            else
            {
                Result.x = cbLTUV.x;
            }
        }
        else
        {
            if (cbTextureSymmetry == 1)
            {
                Result.x = cbLTUV.x;
            }
            else
            {
                Result.x = cbRBUV.x;
            }
        }
        
        if (UV.y == 0.f)
        {
            Result.y = cbLTUV.y;
        }
        else
        {
            Result.y = cbRBUV.y;
        }
    }
    
    return Result;
}

float2 UVScrolling(float2 UV)
{
    float2 ScrollingResult = UV;
    
    if (cbUVScrollEnable == 1)
    {
        ScrollingResult += cbUVScrollSpeed * cbUVScrollTime;
    }
    
    return ScrollingResult;
}

void ChromaKey(float3 rgb)
{
    //크로마키가 활성화 되어있을때
    if (cbChromakeyEnable == 1)
    {
        float dist = distance(rgb, cbChromaKeyColor);
       
        //discard
        //픽셀을 그리지않고 넘어간다.
        //clip(0미만값) discard랑 동일한 역할을 한다.
    
        if (dist < cbChromaKeyThreshold)
        {
            discard;
        }
    }
}

float4 Dissolve(float4 Color, float2 UV)
{
    float4 DissolveResult = Color;
    
    //디졸브
    //텍스처 하나를 더 이용해서 구멍이 뚤리듯이 사라지는 효과
    if (cbDissolveEnable == 1)
    {
        float noise = tbDissolveTexture.Sample(sbSampler, UV).r;
        //디졸브효과를 적용하기위해서 텍스처의 r값이 progress보다 작으면 discard
        if (noise < cbDissolveProgress)
        {
            discard;
        }
        else if (noise < cbDissolveProgress + cbDissolveEdgeWidth)
        {
            DissolveResult.rgb = cbDissolveEdgeColor.rgb;
        }
    }
    
    return DissolveResult;
}

float3 HitEffect(float3 HitColor)
{
    float3 HitEffectResult = HitColor;
    
    if (cbHitEffectEnable == 1)
    {
        HitEffectResult = lerp(HitColor.rgb, cbHitColor.rgb, cbHitIntensity);
    }
    
    return HitEffectResult;
}