#include "ProjectileMovementComponent.h"

#include "SceneComponent.h"
#include "Actor.h"

CProjectileMovementComponent::CProjectileMovementComponent()
{}

CProjectileMovementComponent::CProjectileMovementComponent(const CProjectileMovementComponent& src)
	:CMovementComponent(src)
{
	mRange = src.mRange;
}

CProjectileMovementComponent::CProjectileMovementComponent(CProjectileMovementComponent&& src) noexcept
	:CMovementComponent(std::move(src))
{
	mRange = src.mRange;
}

CProjectileMovementComponent::~CProjectileMovementComponent()
{}

bool CProjectileMovementComponent::Init()
{
	CMovementComponent::Init();

	return true;
}

void CProjectileMovementComponent::Update(float DeltaTime)
{
	CMovementComponent::Update(DeltaTime);

	mRange -= mMoveSpeed * DeltaTime;

	if (mRange <= 0.f)
	{
		auto Owner = mOwner.lock();

		if (Owner)
		{
			Owner->Destroy();
		}
	}
	
}

void CProjectileMovementComponent::PostUpdate(float DeltaTime)
{
	CMovementComponent::PostUpdate(DeltaTime);
}

void CProjectileMovementComponent::Destroy()
{
	CMovementComponent::Destroy();

	if (mOnDestroy)
	{
		mOnDestroy();
	}
}