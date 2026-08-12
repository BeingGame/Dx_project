#pragma once

#include "World/Actor.h"

class CBlackHole :
    public CActor
{
public:
	CBlackHole();
	CBlackHole(const CBlackHole& src);
	CBlackHole(CBlackHole&& src) noexcept;
	virtual ~CBlackHole();
	virtual std::string GetTypeName() const override { return "CBlackHole"; }

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<class CColliderSphere2D> mSphere;

	std::vector<std::weak_ptr<CActor>> mActorArray;
	float mPullSpeed = 70.f;
	
public:
	void SetPullSpeed(float Speed)
	{
		mPullSpeed = Speed;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);

public:
	void OnBeginOverlap(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<class CCollider> Collider);
	void OnEndOverlap(std::weak_ptr<class CCollider> Collider);

protected:
	virtual CBlackHole* Clone()
	{
		return new CBlackHole(*this);
	}

};

