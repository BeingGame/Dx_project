
/*
Shader의 레지스터
입력 레지스터: 정점정보를 넘겨받을때 사용한다. 이름+번호 조합으로 사용된다.
출력 레지스터: 완성된 정점정보를 넘겨줄 레지스터. 이 정보를 통해서 DX가 화면에 출력할 위치를 잡고 색상 출력 준비를 한다.
상수 레지스터: C++코드에서 일반 데이터를 Shader넘겨줄때 사용한다.
임시 레지스터: 연산 시 사용되는 레지스터
*/

#include "Share.hlsli"


struct VS_INPUT_COLOR2D
{
    //변수 뒤에 붙은 단어는 시맨틱
    //입력 레지스터의 이름과 번호를 나태낸다.
    //시맨틱을 통해서 C++코드에서 HLSL코드로 데이터를 넘겨줄수 있다.
    
    //시맨틱뒤에 번호가 안붙으면 0번 취급
    //시맨틱은 필요하다면 본인이 이름을 원하는대로 작성 가능하다.
    float3 Pos : POSITION;
    float4 Color : COLOR;
};

struct VS_OUTPUT_COLOR2D
{
    //(x,y,z,w) 값을 받을수있는 변수를 정의한다.
    //SV_: 시스템이 사용하는 레지스터
    //함부로 사용하려 하면 안된다.
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

//버텍스 셰이더
//ex) 정점 4개인 사각형의경우 4번 돌아간다.
VS_OUTPUT_COLOR2D Color2DVS(VS_INPUT_COLOR2D input)
{
    VS_OUTPUT_COLOR2D output = (VS_OUTPUT_COLOR2D) 0;
    
    //output.Pos = float4(input.Pos, 1.f);
    
    //행렬곱 mul
    //행렬을 곱해서 최종적으로 x,y,z,1.f 값이 월드의 어느위치에 있는지 계산해준다.
    output.Pos = mul(float4(input.Pos, 1.f), cbWVP);
    
    output.Color = input.Color;
    
    return output;
}

PS_OUTPUT_COLOR2D Color2DPS(VS_OUTPUT_COLOR2D input)
{
    PS_OUTPUT_COLOR2D output = (PS_OUTPUT_COLOR2D) 0;
    
    output.Color = input.Color;
   // output.Color = float4(1.f, 0.f, 0.f, 1.f);
    //SV_Target에 최종컬러를 넣어준다.
    return output;
}

float4 FrameVS(float3 pos : POSITION) : SV_POSITION
{
    float4 outputPos = float4(pos, 1.f);
    
    return outputPos;
}

float4 FramePS() : SV_TARGET
{
    return float4(1.f, 0.f, 0.f, 1.f);
}
