#pragma once

#include "MovementComponent.h"

class CCharacterMovementComponent :
    public CMovementComponent
{
public:
	CCharacterMovementComponent();
	CCharacterMovementComponent(const CCharacterMovementComponent& src);
	CCharacterMovementComponent(CCharacterMovementComponent&& src) noexcept;
	virtual ~CCharacterMovementComponent();

public:
	virtual void SetMoveDir(const FVector3& Dir) override;

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

public:
	virtual CCharacterMovementComponent* Clone() const
	{
		return new CCharacterMovementComponent(*this);
	}

	virtual std::string GetTypeName() const override { return "CCharacterMovementComponent"; }
};

