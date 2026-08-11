#pragma once

#include "RenderState.h"

//알파 블렌드(blend 섞는것)
//새로 그릴 픽셀과 이미 그려진 화면 픽셀을 어떻게 섞을지 결정하는것

class CBlendState :
	public CRenderState
{
public:
	CBlendState();
	~CBlendState();

private:
	//이미 그려진 화면의 픽셀을 가져오기위한 렌더타겟 desc 변수
	//렌더타겟 desc변수는 최대 8개의 렌더타겟을 지원한다.
	std::vector<D3D11_RENDER_TARGET_BLEND_DESC> mTargetDesc;

	//블렌드 팩터
	//블렌드 옵션이 D3D11_BLEND_BLEND_FACTOR 혹은 D3D11_BLEND_INV_BLEND_FACTOR로 설정되어 있을때
	//Src와 Dest의 색상에 곱해지는 값
	float mBlendFactor[4] = { 1.f,1.f,1.f,1.f };
	float mPrevBlendFactor[4] = {};

	//MSAA가 사용될때, 픽셀에 나눠져 있는 샘플의 사용 여부를 결정하는 것
	//0xffffffff 모든 샘플링이 적용된다. 보통 0xffffffff를 사용한다.
	UINT mSampleMask = 0xffffffff;
	UINT mPrevSampleMask = 0xffffffff;

public:


	/*
		블렌드 스테이트는 렌더 타겟의 색상(dest)를 그려질 리소스의 색상(src)을 보고 다음과 같이 작동한다.

		Src.rgb * srcBlend blendOp Dest.rgb * destBlend
	ex) 1.f * 1.f + 0.5f * 0.f = 1.f
	*/

	//D3D11_BLEND 블렌드를 할때 어떤식으로 사용될지 결정하는것
	//SrcBlend = D3D11_BLEND_ONE <- 1로 설정해서 현재 출력된 색과 섞이도록 하는것
	//DestBlend = D3D11_BLEND_ZERO <- 블렌딩을 적용하지 않고 현재 픽셀을 그대로 사용하는 것
	//RenderTargetWriteMask 결과물을 저장할때 색상을 어떤색상을 저장할지 결정하는 것
	//D3D11_COLOR_WRITE_ENABLE_ALL : 전체색상을 저장한다.
	//비트플래그로 되어있기 때문에 필요하다면 특정 색상만 덮어씌우게 할수도 있다.
	void AddRenderTargetBlendDesc(bool BlendEnable,
		D3D11_BLEND SrcBlend = D3D11_BLEND_ONE,
		D3D11_BLEND DestBlend = D3D11_BLEND_ZERO,
		D3D11_BLEND_OP BlendOp = D3D11_BLEND_OP_ADD,
		D3D11_BLEND SrcBlendAlpha = D3D11_BLEND_ONE,
		D3D11_BLEND DestBlendAlpha = D3D11_BLEND_ZERO,
		D3D11_BLEND_OP BlendOpAlpha = D3D11_BLEND_OP_ADD,
		UINT8 RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL);

	void SetBlendFactor(float r, float g, float b, float a);
	void SetSampleMask(UINT SampleMask);


	//블렌드 스테이트 생성 함수
	//생성시 AlphaToCoverageEnable, IndependentBlendEnable 두가지 설정을 세팅한다.
	//AlphaToCoverageEnable: MSAA사용시 알파값을 샘플 마스코 바꿔서, 투명경계를 부드럽게 변환시켜준다.
	//IndependentBlendEnable: false로 설정되면 RenderTarget[0]값이 나머지 렌더타겟에 복제되어 출력된다.
	bool CreateState(bool AlphaToCoverageEnable = false, bool IndependentBlendEnable = false);


public:
	virtual void SetState();
	virtual void ResetState();

};

