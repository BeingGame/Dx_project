#include "Turret.h"

#include "World/MeshComponent.h"

#include "MainWorld.h"
#include "Bullet.h"

#include "LogManager.h"

CTurret::CTurret()
{}

CTurret::CTurret(const CTurret & src)
{}

CTurret::CTurret(CTurret && src) noexcept
{}

CTurret::~CTurret()
{}

bool CTurret::Init()
{
	mMeshComp = CreateComponent<CMeshComponent>("Root");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetMesh("ColorRect");
		Mesh->SetShader("Color2D");
		Mesh->SetRelativeScale(30.f, 30.f);
	}


	return true;
}

void CTurret::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);

	mFireTime -= DeltaTime;

	if (mFireTime <= 0.f)
	{
		mFireTime = 0.5f;
		Fire();
	}
}

void CTurret::Destroy()
{
	CActor::Destroy();
}

void CTurret::Fire()
{
	auto World = mWorld.lock();

	if (World)
	{
		auto Bullet = World->CreateActor<CBullet>("Bullet").lock();

		if (Bullet)
		{
			//정면을 향해 발사

			
			Bullet->SetRelativeRotation(GetWorldRot());


			FVector3 Axis = Bullet->GetAxis(EAxis::Y);
			Axis.Normalize();
			FVector3 BulletPos = GetWorldPos() + Axis * 100.f;

			Bullet->SetWorldPos(BulletPos);
			Bullet->SetMoveDir(Axis);

		}
	}
}
