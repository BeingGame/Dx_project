#pragma once

#include "World/Actor.h"
#include "TimeManager.h"

class CBullet :
    public CActor
{
public:
	CBullet();
	CBullet(const CBullet& src);
	CBullet(CBullet&& src) noexcept;
	virtual ~CBullet();

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<class CColliderBox2D> mBox;

	std::weak_ptr<class CProjectileMovementComponent> mMoveComp;

	FTimerHandle mTimerHandle;

public:
	float GetSpeed();
	float GetCurrentSpeed();

	void SetMoveDir(const FVector3& Dir);
	void SetSpeed(float Speed);
	void SetCurrentSpeed(float Speed);

	void Test();

	void OnTimerEnd();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();
	virtual void OnCollision(const std::weak_ptr<CActor>& HitActor);


protected:
	virtual CBullet* Clone()
	{
		return new CBullet(*this);
	}
};

