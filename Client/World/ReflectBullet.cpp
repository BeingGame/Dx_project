#include "ReflectBullet.h"

#include "World/MeshComponent.h"
#include "World/ProjectileMovementComponent.h"
#include "World/ColliderBox2D.h"

#include "LogManager.h"

CReflectBullet::CReflectBullet()
{}

CReflectBullet::CReflectBullet(const CReflectBullet& src)
	:CActor(src)
{}

CReflectBullet::CReflectBullet(CReflectBullet&& src) noexcept
	:CActor(std::move(src))
{}

CReflectBullet::~CReflectBullet()
{}

float CReflectBullet::GetSpeed()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		return MoveComp->GetMoveSpeed();
	}

	return 0.0f;
}

float CReflectBullet::GetCurrentSpeed()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		return MoveComp->GetCurrentSpeed();
	}

	return 0.0f;
}

void CReflectBullet::SetMoveDir(const FVector3& Dir)
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetMoveDir(Dir);
	}
}

void CReflectBullet::SetSpeed(float Speed)
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetSpeed(Speed);
	}
}

void CReflectBullet::SetCurrentSpeed(float Speed)
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetCurrentSpeed(Speed);
	}
}

bool CReflectBullet::Init()
{
	CActor::Init();

	mActorTag = "Player";

	mMeshComp = CreateComponent<CMeshComponent>("Root");
	mBox = CreateComponent<CColliderBox2D>("Box");

	mMoveComp = CreateComponent<CProjectileMovementComponent>("MoveComp");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		/*Mesh->SetMesh("ColorRect");
		Mesh->SetShader("Color2D");*/
		Mesh->SetMesh("TexRect");
		Mesh->SetShader("Material");
		Mesh->AddTexture(0, "Bullet", TEXT("Wallnut.png"));
		Mesh->SetRelativeScale(20.f, 20.f);
	}

	auto Box = mBox.lock();

	if (Box)
	{
		Box->SetInheritScale(false);
		Box->SetWorldScale(1.f, 1.f);
		Box->SetBoxSize(25.f, 25.f);
		Box->SetDebugDraw(true);
		Box->SetCollisionProfile("BlockAll");
		Box->SetHitFunc(this, &CReflectBullet::OnHit);
	}

	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetUpdateComp(Mesh);
		MoveComp->SetSpeed(350.f);
		MoveComp->SetCurrentSpeed(350.f);
	}

	return true;
}

void CReflectBullet::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);

}

void CReflectBullet::Destroy()
{
	CActor::Destroy();

}

void CReflectBullet::OnCollision(const std::weak_ptr<CActor>& HitActor)
{
	auto Actor = HitActor.lock();

	if (Actor)
	{
		Actor->TakeDamage(1.f, "Player", GetThisPtr<CActor>());
	}

	Destroy();
}

void CReflectBullet::OnHit(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<class CCollider> Collider)
{
	//반사벡터를 만들어서
	//MoveDir을 변경한다.

	auto _Collider = Collider.lock();

	if (!_Collider)
	{
		return;
	}

	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		FVector3 MoveDir = MoveComp->GetMoveDir();
		//반사 벡터 공식 : V - 2*(V*N(내적)*N
		float Dot = MoveDir.DotProduct(Normal);

		MoveDir = MoveDir - Normal * Dot * 2.f;
		MoveDir.Normalize();

		MoveComp->SetMoveDir(MoveDir);
	}

	//Bounce++;
}
