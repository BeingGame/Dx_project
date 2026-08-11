#pragma once
#include "Asset.h"
#include "CBufferContainer.h"

//상수 버퍼
//상수를 모아놓은 하나의 통로

//정점이 수만개인 메쉬의 경우엔 버텍스 셰이더 수만번 돌아간다.
//World행렬 -> View행렬 -> Proj행렬

//크기 * 회전 * 이동 <- 월드 행렬을 버텍스 셰이더에서 계산한다.
//ex) 정점이 수만개일때 수만번의 월드행렬을 만들게 된다.

//따라서 상수버퍼를 이용해서 단한번만 계산해서 불필요한 중복 계산을 줄여줄수 있다.

//상수버퍼는 16바이트를 기준으로 만들어야한다.
//이유는 GPU가 16바이트를 기준으로 읽기 때문에

class CCBuffer :
    public CAsset
{
public:
	CCBuffer();
	CCBuffer(const CCBuffer& src);
	CCBuffer(CCBuffer&& src) noexcept;
	virtual ~CCBuffer();

private:
	//레지스터에 상수버퍼를 업데이트할 버퍼
	ComPtr<ID3D11Buffer> mBuffer = nullptr;

	//상수버퍼 크기
	int mSize = 0;

	//상수버퍼의 레지스터 번호
	int mRegister = 0;

	//어떤 Shader에서 상수버퍼를 사용할지 판단하는 변수
	int mShaderBuffer = 0;
		
public:
	bool Init(int Size, int Register, int ShaderBuffer);
	void Update(void* Data);

};

