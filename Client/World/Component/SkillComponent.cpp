#include "SkillComponent.h"

CSkillComponent::CSkillComponent()
{}

CSkillComponent::CSkillComponent(const CSkillComponent & src)
	:CActorComponent(src)
{}

CSkillComponent::CSkillComponent(CSkillComponent && src) noexcept
	:CActorComponent(std::move(src))
{}

CSkillComponent::~CSkillComponent()
{}

bool CSkillComponent::Init()
{
	CActorComponent::Init();

	return true;
}

void CSkillComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);

	if (!mSkillEnable)
	{
		mSkillTime += DeltaTime;

		if (mSkillTime >= mSkillCoolTime)
		{
			mSkillTime = 0.f;
			mSkillEnable = true;
		}
	}

}

void CSkillComponent::PostUpdate(float DeltaTime)
{
	CActorComponent::PostUpdate(DeltaTime);
}

void CSkillComponent::Destroy()
{
	CActorComponent::Destroy();
}

bool CSkillComponent::SkillProgress()
{
	return mSkillEnable;
}

void CSkillComponent::SkillEnd()
{
	mSkillEnable = false;
	mSkillTime = 0.f;
}
