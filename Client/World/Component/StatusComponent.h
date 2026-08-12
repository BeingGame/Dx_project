#pragma once

#include "World/ActorComponent.h"

class CStatusComponent :
    public CActorComponent
{
public:
	CStatusComponent();
	CStatusComponent(const CStatusComponent& src);
	CStatusComponent(CStatusComponent&& src) noexcept;
	virtual ~CStatusComponent();

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

	void AddHP(float HP);


public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

public:
	virtual CStatusComponent* Clone() const
	{
		return new CStatusComponent(*this);
	}

	virtual std::string GetTypeName() const override { return "CStatusComponent"; }

public:
	template<typename T>
	void SetDeathFunction(T* Object, void (T::* Func)())
	{
		mDeathFunc = std::bind(Func, Object);
	}

};

