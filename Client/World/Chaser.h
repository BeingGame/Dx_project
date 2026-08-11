#pragma once

#include "World/Actor.h"

class CChaser :
    public CActor
{
public:
	CChaser();
	CChaser(const CChaser& src);
	CChaser(CChaser&& src) noexcept;
	virtual ~CChaser();

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<class CCharacterMovementComponent> mMoveComp;

	std::weak_ptr<CActor> mTarget;

	float mDistance = 50.f;

public:
	void SetTarget(std::weak_ptr<CActor> Target)
	{
		mTarget = Target;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();


protected:
	virtual CChaser* Clone()
	{
		return new CChaser(*this);
	}
};

