#pragma once

#include "World/Actor.h"

class CShield :
    public CActor
{
public:
	CShield();
	CShield(const CShield& src);
	CShield(CShield&& src) noexcept;
	virtual ~CShield();
	virtual std::string GetTypeName() const override { return "CShield"; }

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<CActor> mOwner;

	float mAngle = 0.f;
	float mRotationSpeed = 180.f;
	float mDistance = 100.f;

public:
	void SetAngle(const std::weak_ptr<CActor> Owner,float Angle)
	{
		mOwner = Owner;
		mAngle = Angle;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();
	virtual void OnCollision(const std::weak_ptr<CActor>& HitActor);

protected:
	virtual CShield* Clone()
	{
		return new CShield(*this);
	}
};

