#include "CharacterMovementComponent.h"

#include "SceneComponent.h"

CCharacterMovementComponent::CCharacterMovementComponent()
{}

CCharacterMovementComponent::CCharacterMovementComponent(const CCharacterMovementComponent& src)
	:CMovementComponent(src)
{}

CCharacterMovementComponent::CCharacterMovementComponent(CCharacterMovementComponent&& src) noexcept
	:CMovementComponent(std::move(src))
{}

CCharacterMovementComponent::~CCharacterMovementComponent()
{}

void CCharacterMovementComponent::SetMoveDir(const FVector3& Dir)
{
	mMoveDir += Dir;
}

bool CCharacterMovementComponent::Init()
{
	CMovementComponent::Init();

	return true;
}

void CCharacterMovementComponent::Update(float DeltaTime)
{
	CMovementComponent::Update(DeltaTime);

	mMoveDir = FVector3::Zero;
}

void CCharacterMovementComponent::PostUpdate(float DeltaTime)
{
	CMovementComponent::PostUpdate(DeltaTime);
}

void CCharacterMovementComponent::Destroy()
{
	CMovementComponent::Destroy();
}
