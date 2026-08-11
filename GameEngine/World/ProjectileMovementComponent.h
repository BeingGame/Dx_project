#pragma once
#include "MovementComponent.h"
class CProjectileMovementComponent :
    public CMovementComponent
{
public:
	CProjectileMovementComponent();
	CProjectileMovementComponent(const CProjectileMovementComponent& src);
	CProjectileMovementComponent(CProjectileMovementComponent&& src) noexcept;
	virtual ~CProjectileMovementComponent();

protected:
	float mRange = 1000.f;

	std::function<void()> mOnDestroy;

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

public:
	virtual CProjectileMovementComponent* Clone() const
	{
		return new CProjectileMovementComponent(*this);
	}

public:
	template<typename T>
	void SetDestroyFunction(T* Object, void (T::* Func)())
	{
		mOnDestroy = std::bind(Func, Object);
	}
};

