#pragma once

#include "EngineInfo.h"

//월드를 포함해서 매니저를 제외한 모든 클래스들의 부모가 될 추상클래스
//클래스 하나로 관리할 수 있게 설계

class CObject : public std::enable_shared_from_this<CObject>
{
public:
	CObject();
	CObject(const CObject& src);
	CObject(CObject&& src) noexcept;
	virtual ~CObject() = 0;

protected:
	//현재 활성화 되어 있는지 확인하는 불 변수
	bool mEnable = true;

	//현재 살아있는 확인하는 불 변수
	bool mAlive = true;

public:
	bool IsEnable() const
	{
		return mEnable;
	}

	bool IsAlive() const
	{
		return mAlive;
	}

	void SetEnable(bool Enable)
	{
		mEnable = Enable;
	}

	void SetAlive(bool Alive)
	{
		mAlive = Alive;
	}


public:
	template<typename T>
	std::shared_ptr<T> GetThisPtr()
	{
		return std::dynamic_pointer_cast<T>(shared_from_this());
	}

};

