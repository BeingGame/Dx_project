#include "StatusComponent.h"

CStatusComponent::CStatusComponent()
{}

CStatusComponent::CStatusComponent(const CStatusComponent& src)
	:CActorComponent(std::move(src))
{}

CStatusComponent::CStatusComponent(CStatusComponent&& src) noexcept
	:CActorComponent(std::move(src))
{}

CStatusComponent::~CStatusComponent()
{}

void CStatusComponent::AddHP(float HP)
{
	mHP += HP;

	if (mHP <= 0.f)
	{
		if (mDeathFunc)
		{
			mDeathFunc();
		}
	}
	else if (mHP > mHPMax)
	{
		mHP = mHPMax;
	}
}

bool CStatusComponent::Init()
{
	return true;
}

void CStatusComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);
}

void CStatusComponent::PostUpdate(float DeltaTime)
{
	CActorComponent::PostUpdate(DeltaTime);
}

void CStatusComponent::Destroy()
{
	CActorComponent::Destroy();
}
