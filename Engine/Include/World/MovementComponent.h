#pragma once
#include "ActorComponent.h"
class CMovementComponent :
    public CActorComponent
{
public:
	CMovementComponent();
	CMovementComponent(const CMovementComponent& src);
	CMovementComponent(CMovementComponent&& src) noexcept;
	virtual ~CMovementComponent();

protected:
	std::weak_ptr<class CSceneComponent> mUpdateComp;

	FVector3 mMoveDir;
	float mMoveSpeed = 150.f;
	float mCurrentSpeed = 150.f;

public:
	float GetMoveSpeed() const
	{
		return mMoveSpeed;
	}

	float GetCurrentSpeed() const
	{
		return mCurrentSpeed;
	}

	FVector3 GetMoveDir() const
	{
		return mMoveDir;
	}

	virtual void SetMoveDir(const FVector3& Dir);

	void SetUpdateComp(const std::weak_ptr<class CSceneComponent> Comp)
	{
		mUpdateComp = Comp;
	}

	void SetSpeed(float Speed)
	{
		mMoveSpeed = Speed;
	}

	void SetCurrentSpeed(float Speed)
	{
		mCurrentSpeed = Speed;
	}

protected:
	//소유 액터의 루트 씬 컴포넌트를 이동 대상으로 물린다.
	bool BindUpdateCompFromOwner();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

public:
	virtual CMovementComponent* Clone() const
	{
		return new CMovementComponent(*this);
	}

public:
	virtual std::string GetTypeName() const override { return "CMovementComponent"; }
	virtual void Save(std::ofstream& File) const override;
	virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;
};

