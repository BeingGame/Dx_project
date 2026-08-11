#pragma once
#include "Asset.h"

//모든 셰이더의 부모가될 추상클래스

class CShader :
    public CAsset
{
public:
	CShader();
	CShader(const CShader& src);
	CShader(CShader&& src) noexcept;
	virtual ~CShader() = 0;

public:
	virtual bool Init(const std::string& PathName) = 0;
	//DX에 현재 무슨 셰이더를 사용할지 결정해서 출력하는 용도로 사용된다.
	virtual void SetShader() = 0;


};

