#pragma once

#include "SkillComponent.h"

class CTurretSkillComp :
    public CSkillComponent
{
public:
	CTurretSkillComp();
	CTurretSkillComp(const CTurretSkillComp& src);
	CTurretSkillComp(CTurretSkillComp&& src) noexcept;
	virtual ~CTurretSkillComp();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

public:
	virtual bool SkillProgress();
	virtual void SkillEnd();

public:
	virtual CTurretSkillComp* Clone() const
	{
		return new CTurretSkillComp(*this);
	}

	virtual std::string GetTypeName() const override { return "CTurretSkillComp"; }
};

