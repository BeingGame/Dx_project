#pragma once

#include "World/Actor.h"
#include "Interface/StatusInterface.h"

class CPlayer :
    public CActor, CStatusInterface
{
public:
	CPlayer();
	CPlayer(const CPlayer& src);
	CPlayer(CPlayer&& src) noexcept;
	virtual ~CPlayer();

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<class CCameraComponent> mCamera;
	std::weak_ptr<class CColliderBox2D> mBox;
	std::weak_ptr<class CColliderSphere2D> mSphere;
	std::weak_ptr<class CWidgetComponent> mWidget;

	std::weak_ptr<class CCharacterMovementComponent> mMoveComp;
	std::weak_ptr<class CSkillComponent> mSkillComp;
	std::weak_ptr<class CAnimation2DComponent> mAnimComp;
	
	std::list<std::weak_ptr<class CMonster>> mMonsterList;

	float mMoveSpeed = 100.f;

	//스킬
	std::vector<std::weak_ptr<class CShield>> mShieldList;

	//스킬3 대쉬

	bool mDash = false;
	FVector3 mDashDir;
	float mDashDistance = 0.f;
	float mDashDistanceMax = 0.f;
	float mDashSpeed = 1200.f;

	//UV 스크롤링용 변수
	float mUVScrollTime= 0.f;

	//Dissolve 효과 변수
	bool mIsDissolve = false;
	float mDissolveProgress = 0.f;
	float mDissolveSpeed = 0.3f;

	//Hit
	bool mIsHit = false;
	float mHitTime = 0.f;
	float mHitDuration = 0.3f;

public:
	std::weak_ptr<class CCameraComponent> GetCamera() const
	{
		return mCamera;
	}

	float GetSpeed();
	float GetCurrentSpeed();

	void SetSpeed(float Speed);
	void SetCurrentSpeed(float Speed);

	void AddMonster(std::weak_ptr<class CMonster> Monster);

	virtual void SetHP(float HP);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();
	virtual float TakeDamage(float fDamage, const std::string& Instigator, const std::weak_ptr<CActor>& CauseActor);

public:
	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();

	void RotateLeft();
	void RotateRight();

	void Fire();
	void ChangeCamera();
	void ReturnCamera();

	void Skill1();
	void Skill2();
	void Skill3();

	void NewSkill1();
	void NewSkill2();
	void NewSkill3();

	void MouseClick();

	void Dissolve();

	void TestDamage();

	void Hit();

	void DebugMessage();

protected:
	virtual CPlayer* Clone()
	{
		return new CPlayer(*this);
	}
};

