#pragma once

#include "EngineInfo.h"

class CStatusInterface
{

protected:
	float mHP = 10.f;
	float mHPMax = 10.f;

	std::function<void()> mDeathFunc;

public:
	float GetHP() const
	{
		return mHP;
	}

	void SetHPMax(float HPMax)
	{
		mHP = HPMax;
		mHPMax = HPMax;
	}

	virtual void SetHP(float HP) = 0;

	void AddHP(float HP);

public:
	template<typename T>
	void SetDeathFunction(T* Object, void (T::* Func)())
	{
		mDeathFunc = std::bind(Func, Object);
	}

};

