#pragma once

#include "World/Actor.h"

class CReflectBullet :
    public CActor
{
public:
	CReflectBullet();
	CReflectBullet(const CReflectBullet& src);
	CReflectBullet(CReflectBullet&& src) noexcept;
	virtual ~CReflectBullet();

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<class CColliderBox2D> mBox;

	std::weak_ptr<class CProjectileMovementComponent> mMoveComp;
public:
	float GetSpeed();
	float GetCurrentSpeed();

	void SetMoveDir(const FVector3& Dir);
	void SetSpeed(float Speed);
	void SetCurrentSpeed(float Speed);
public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();
	virtual void OnCollision(const std::weak_ptr<CActor>& HitActor);
	virtual void OnHit(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<class CCollider> Collider);


protected:
	virtual CReflectBullet* Clone()
	{
		return new CReflectBullet(*this);
	}
};

