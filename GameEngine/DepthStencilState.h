#pragma once

#include "RenderState.h"

//뎁스 스텐실
//물체의 앞/뒤 가림 판정(Depth)과 화면의 특정 영역을 마스킹(Masking)할지(Stencil) 결정하는것
//스텐실 값에 따라서 어떤 물체를 가리거나, 특정 렌더링을 시도할수 있다
//포스트 프로세스(후처리)

class CDepthStencilState :
	public CRenderState
{
public:
	CDepthStencilState();
	virtual ~CDepthStencilState();

protected:
	//스텐실 버퍼와 비교할 특정값을 선언한다.
	UINT mStencilRef = 0xffffffff;
	UINT mPrevStencilRef = 0xffffffff;

public:
	//DepthEnable : 깊이를 쓸지 말지 결정한다.
	//DepthWriteMask : ZERO,ALL로 구성되어있는데, ZERO를 하면 깊이버퍼에 깊이값을 쓰지않음, ALL로 하면 깊이값을 쓴다
	//DepthFunc : 깊이 값을 비교할때 사용하는 함수
	//StencilEnable : 스텐실을 쓸지 말지 결정한다.
	//StencilReadMask : 스텐실을 테스트 할떄 읽어서 비교할 비트마스크
	//StencilWriteMask : 테스트를 통과한 후 스텐실에 값을 작성할때 비교할 비트마스크
	//front 카메라에 그려지는 폴리곤에 대한 규칙을 지정한다.
	//back 카메라에 그려지지 않는 컬링된 부분에 대한 규칙을 지정한다.

	bool CreateState(bool DepthEnable = true,
		D3D11_DEPTH_WRITE_MASK DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
		D3D11_COMPARISON_FUNC DepthFunc = D3D11_COMPARISON_LESS,
		bool StencilEnable = false, UINT8 StencilReadMask = 0xff,
		UINT8 StencilWriteMask = 0xff,
		D3D11_DEPTH_STENCILOP_DESC FrontFace = 
		{
			D3D11_STENCIL_OP_KEEP,D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
		},
		D3D11_DEPTH_STENCILOP_DESC BackFace =
		{
			D3D11_STENCIL_OP_KEEP,D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
		});


public:
	virtual void SetState();
	virtual void ResetState();
};

