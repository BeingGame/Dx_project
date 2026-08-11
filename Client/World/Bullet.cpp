#include "Bullet.h"

#include "World/MeshComponent.h"
#include "World/ProjectileMovementComponent.h"
#include "World/ColliderBox2D.h"
#include "World/World.h"

#include "LogManager.h"

CBullet::CBullet()
{}

CBullet::CBullet(const CBullet& src)
	:CActor(src)
{}

CBullet::CBullet(CBullet&& src) noexcept
	:CActor(std::move(src))
{}

CBullet::~CBullet()
{}

float CBullet::GetSpeed()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		return MoveComp->GetMoveSpeed();
	}

	return 0.0f;
}

float CBullet::GetCurrentSpeed()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		return MoveComp->GetCurrentSpeed();
	}

	return 0.0f;
}

void CBullet::SetMoveDir(const FVector3& Dir)
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetMoveDir(Dir);
	}
}

void CBullet::SetSpeed(float Speed)
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetSpeed(Speed);
	}
}

void CBullet::SetCurrentSpeed(float Speed)
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetCurrentSpeed(Speed);
	}
}

void CBullet::Test()
{
	//LOG_DEBUG("Test Bullet");
}

void CBullet::OnTimerEnd()
{
	LOG_DEBUG("Timer End");

	Destroy();
}

bool CBullet::Init()
{
	CActor::Init();

	mActorTag = "Player";

	mMeshComp = CreateComponent<CMeshComponent>("Root");
	mBox = CreateComponent<CColliderBox2D>("Box");

	//mMoveComp = CreateComponent<CProjectileMovementComponent>("MoveComp");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		//Mesh->SetMesh("ColorRect");
		//Mesh->SetShader("Color2D");
		Mesh->SetMesh("TexRect");
		Mesh->SetShader("Material");
		Mesh->AddTexture(0, "Bullet", TEXT("Wallnut.png"));
		Mesh->SetRelativeScale(20.f, 20.f);
	}

	auto Box = mBox.lock();

	if (Box)
	{
		Box->SetInheritScale(false);
		Box->SetBoxSize(150.f, 150.f);
		Box->SetDebugDraw(true);
		Box->SetCollisionProfile("Monster");
	}

	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetUpdateComp(Mesh);
		MoveComp->SetSpeed(350.f);
		MoveComp->SetCurrentSpeed(350.f);
		MoveComp->SetDestroyFunction<CBullet>(this, &CBullet::Test);
	}

	//실습
	//캐릭터 무브먼트 컴포넌트를 참고해서 projectilemovementcomponent 제작
	//차이점은 캐릭터는 연속적인 입력을 받지만, 발사체는 단 한번만 입력을 받는다.
	//발사체는 일정 거리를 이동하면 월드에서 사라진다.

	//스킬 만들었던거
	//컴포넌트로 만들어서 플레이어에 부착하기



	auto World = mWorld.lock();

	if (World)
	{
		mTimerHandle = World->SetTimer(2.f, false, this, &CBullet::OnTimerEnd);
	}

	return true;
}

void CBullet::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);

	AddRelativePos(GetAxis(EAxis::Y) * 300.f * DeltaTime);

	/*mrange -= mbulletspeed * deltatime;

	if (mrange < 0.f)
	{
		destroy();
	}*/

}

void CBullet::Destroy()
{
	CActor::Destroy();

	//CTimeManager::ClearTImer(mTimerHandle);

}

void CBullet::OnCollision(const std::weak_ptr<CActor>& HitActor)
{
	auto Actor = HitActor.lock();

	if (Actor)
	{
		Actor->TakeDamage(1.f, "Player", GetThisPtr<CActor>());
	}

	Destroy();
}
