#pragma once

#include "World/Actor.h"

class Collider;

class CSlowArea :
	public CActor
{

public:
	CSlowArea();
	CSlowArea(const CSlowArea& src);
	CSlowArea(CSlowArea&& src) noexcept;
	virtual ~CSlowArea();

protected:
	std::weak_ptr<class CColliderSphere2D> mSphere;

	//float mRadius = 150.f;
	float mSlowRatio = 2.f;

	//float mTime = 1.f;

	std::unordered_map<std::shared_ptr<CActor>, float> mSlowedActormap;

public:
	/*float GetRadius() const
	{
		return mRadius;
	}*/
	
	void SetSlowRatio(float Ratio)
	{
		mSlowRatio = Ratio;
	}

	/*void SetRadius(float Radius)
	{
		mRadius = Radius;
	}*/

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();

protected:
	virtual CSlowArea* Clone()
	{
		return new CSlowArea(*this);
	}

public:
	void OnBeginOverlap(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<class CCollider> Collider);

	void OnEndOverlap(std::weak_ptr<class CCollider> Collider);
};

