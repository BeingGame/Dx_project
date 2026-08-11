#pragma once

#include "../EngineInfo.h"

//상수버퍼의 데이터를 저장할 클래스
//상수버퍼마다 필요한 구조체가 다르기 때문에
//추상클래스로 만들어서 상수버퍼의 데이터를 관리하도록 한다.

class CCBufferData
{
public:
	CCBufferData();
	CCBufferData(const CCBufferData& src);
	CCBufferData(CCBufferData&& src) noexcept;
	virtual ~CCBufferData() = 0;

protected:
	//셰이더에 데이터를 보낼때 CBuffer는 동일한 버퍼를 사용하기 위해서
	std::shared_ptr<class CCBuffer> mBuffer;

protected:
	void SetConstantBuffer(const std::string& Name);

public:
	virtual bool Init() = 0;
	virtual void UpdateBuffer() = 0;
	virtual CCBufferData* Clone() = 0;

};

