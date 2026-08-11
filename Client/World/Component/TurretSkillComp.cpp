#include "TurretSkillComp.h"

#include "../MainWorld.h"
#include "World/Actor.h"

#include "../Turret.h"

CTurretSkillComp::CTurretSkillComp()
{}

CTurretSkillComp::CTurretSkillComp(const CTurretSkillComp& src)
	:CSkillComponent(src)
{}

CTurretSkillComp::CTurretSkillComp(CTurretSkillComp&& src) noexcept
	:CSkillComponent(std::move(src))
{}

CTurretSkillComp::~CTurretSkillComp()
{}

bool CTurretSkillComp::Init()
{
	CSkillComponent::Init();

	return true;
}

void CTurretSkillComp::Update(float DeltaTime)
{
	CSkillComponent::Update(DeltaTime);
}

void CTurretSkillComp::PostUpdate(float DeltaTime)
{
	CSkillComponent::PostUpdate(DeltaTime);
}

void CTurretSkillComp::Destroy()
{
	CSkillComponent::Destroy();
}

bool CTurretSkillComp::SkillProgress()
{
	if (!CSkillComponent::SkillProgress())
	{
		return false;
	}

	auto World = mWorld.lock();

	if (World)
	{
		auto Owner = mOwner.lock();

		if (Owner)
		{
			for (int i = 0; i < 2; ++i)
			{
				FVector3 Axis = Owner->GetAxis(EAxis::X);

				Axis = i == 0 ? -Axis : Axis;

				FVector3 TurretPos = Owner->GetWorldPos() + Axis * 100.f;

				auto PlayerRoot = Owner->GetRootComponent().lock();

				auto Turret = World->CreateActor<CTurret>("Turret").lock();


				if (Turret)
				{
					//서로 다른 액터를 부착해준다.

					Turret->AttachActorToActor(Owner->GetThisPtr<CActor>());

					Turret->SetWorldPos(TurretPos);
					Turret->SetWorldScale(30.f, 30.f);

					/*if (i == 0)
					{
						Turret->DetachActorFromActor();
					}*/

				}


			}
		}
	}



	SkillEnd();
	return true;
}

void CTurretSkillComp::SkillEnd()
{
	CSkillComponent::SkillEnd();
}
