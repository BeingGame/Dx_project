#pragma once

#include "World/Actor.h"

class CTurret :
    public CActor
{
public:
	CTurret();
	CTurret(const CTurret& src);
	CTurret(CTurret&& src) noexcept;
	virtual ~CTurret();

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;

	float mFireTime = 0.5f;


public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();

	void Fire();

protected:
	virtual CTurret* Clone()
	{
		return new CTurret(*this);
	}
};

