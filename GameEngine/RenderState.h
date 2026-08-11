#pragma once

#include "EngineInfo.h"

//렌더 스테이트
//셰이더가 무엇을 그릴지 결정하고 그렸다면
//스테이트 그려지는 픽셀이 어떻게 처리될지 정하는 규칙
//알파값이 0일때 픽셀 그려지는게 무시되거나, 뎁스값에 따라서 앞에 이미 다른 값이 있으면 픽셀 그려지는게 무시될수 있다.

//알파 블렌드, 뎁스 스텐실(주로 3D에서 사용), 래스터라이저(개념만 공부할것)

//알파 블렌드(blend 섞는것)
//새로 그릴 픽셀과 이미 그려진 화면 픽셀을 어떻게 섞을지 결정하는것

//뎁스 스텐실
//물체의 앞/뒤 가림 판정(Depth)과 화면의 특정 영역을 마스킹(Masking)할지(Stencil) 결정하는것
//스텐실 값에 따라서 어떤 물체를 가리거나, 특정 렌더링을 시도할수 있다
//포스트 프로세스(후처리)

//래스터 라이저
//정점 데이터를 이용해서 화면의 픽셀을 그릴때 어떤 방식으로 그릴지 결정하는것
//ex) back face culling을 무시하고 그냥 픽셀을 그려버릴수도 있다.

//알파 블렌딩, 뎁스 스텐실
//OM(Output merge)단계

//래스터라이저
//RS단계


//추상클래스
class CRenderState
{
public:
	CRenderState();
	virtual ~CRenderState();

protected:
	//렌더 스테이트는 컴객체를 상속받아서 만들어준다.
	ComPtr<ID3D11DeviceChild> mState = nullptr;
	ComPtr<ID3D11DeviceChild> mTestState = nullptr;

	//스테이트를 등록하기전에 이전 스테이트를 저장할 컴객체 변수
	ComPtr<ID3D11DeviceChild> mPrevState = nullptr;
	ComPtr<ID3D11DeviceChild> mTestPrevState = nullptr;

	//스테이트 이름
	std::string mName;

public:
	const std::string& GetName() const
	{
		return mName;
	}

	void SetName(const std::string& Name)
	{
		mName = Name;
	}

public:
	virtual void SetState() = 0;
	virtual void ResetState() = 0;

};

