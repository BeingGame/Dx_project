#pragma once

#include "World/Actor.h"

class CMonster :
    public CActor
{
public:
	CMonster();
	CMonster(const CMonster& src);
	CMonster(CMonster&& src) noexcept;
	virtual ~CMonster();
	virtual std::string GetTypeName() const override { return "CMonster"; }

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<class CCameraComponent> mCamera;
	std::weak_ptr<class CColliderBox2D> mBox;
	std::weak_ptr<class CColliderSphere2D> mSphere;
	std::weak_ptr<class CSoundComponent> mSound;

	std::weak_ptr<class CStatusComponent> mStatus;
	std::weak_ptr<class CAnimation2DComponent> mAnimComp;
	std::weak_ptr<class CAIComponent> mAIComp;

	std::weak_ptr<class CPlayer> mPlayer;

	float mSpeed = 100.f;
	float mRange = 100.f;

	float mHP = 10.f;

	float mTime = 0.5f;

public:
	std::weak_ptr<class CCameraComponent> GetCamera() const
	{
		return mCamera;
	}

public:
	void SetPlayer(const std::weak_ptr<class CPlayer>& Player)
	{
		mPlayer = Player;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();
	virtual void OnCollision(const std::weak_ptr<CActor>& HitActor);

	virtual float TakeDamage(float fDamage, const std::string& Instigator, const std::weak_ptr<CActor>& CauseActor);

	void Fire();


	void DebugMessage();

protected:
	virtual CMonster* Clone()
	{
		return new CMonster(*this);
	}



};