#pragma once

#include "World/ActorComponent.h"

class CSkillComponent :
    public CActorComponent
{
public:
	CSkillComponent();
	CSkillComponent(const CSkillComponent& src);
	CSkillComponent(CSkillComponent&& src) noexcept;
	virtual ~CSkillComponent();

protected:
	float mSkillTime = 0.f;
	float mSkillCoolTime = 5.f;

	bool mSkillEnable = false;

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

public:
	virtual bool SkillProgress() = 0;
	virtual void SkillEnd() =0;

public:
	virtual CSkillComponent* Clone() const = 0;
};

