#pragma once
#include "Component.h"
class CActorComponent :
    public CComponent
{
public:
	CActorComponent();
	CActorComponent(const CActorComponent& src);
	CActorComponent(CActorComponent&& src) noexcept;
	virtual ~CActorComponent();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void Destroy();

public:
	virtual CActorComponent* Clone() const
	{
		return new CActorComponent(*this);
	}

};

