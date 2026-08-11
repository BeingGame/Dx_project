#include "Monster.h"

#include "World/MeshComponent.h"
#include "World/CameraComponent.h"
#include "Component/StatusComponent.h"
#include "World/ColliderBox2D.h"
#include "World/ColliderSphere2D.h"
#include "World/Animation2DComponent.h"
#include "World/SoundComponent.h"
#include "World/AIComponent.h"

#include "MonsterAttackState.h"
#include "MonsterMoveState.h"

#include "MainWorld.h"

#include "Player.h"

#include "Bullet.h"

#include "LogManager.h"

CMonster::CMonster()
{}

CMonster::CMonster(const CMonster& src)
{}

CMonster::CMonster(CMonster&& src) noexcept
{}

CMonster::~CMonster()
{}

bool CMonster::Init()
{
	CActor::Init();

	mActorTag = "Monster";

	mMeshComp = CreateComponent<CMeshComponent>("Root");
	mCamera = CreateComponent<CCameraComponent>("Camera");
	//mBox = CreateComponent<CColliderBox2D>("Box");
	mSphere = CreateComponent<CColliderSphere2D>("Sphere");
	//mSound = CreateComponent<CSoundComponent>("Sound");

	mStatus = CreateComponent<CStatusComponent>("Status");
	mAnimComp = CreateComponent<CAnimation2DComponent>("Anim");
	mAIComp = CreateComponent<CAIComponent>("AI");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetMesh("TexRect");
		Mesh->SetShader("Animation2D");
		Mesh->AddTexture(0, "Player", TEXT("teemoChromakey.png"));
		Mesh->AddTexture(0, "DissolveTexture", TEXT("dissolve_smoke.png"), "Texture", 1);
		Mesh->SetBlendState(0, "AlphaBlend");
		Mesh->SetRelativeScale(300.f, 300.f);

	}


	auto Anim = mAnimComp.lock();

	if (Anim)
	{
		Anim->SetUpdateComponent(mMeshComp);

		Anim->AddAnimation("PlayerIdleRatio", 1.f, 1.f, true);
	}


	auto Camera = mCamera.lock();

	if (Camera)
	{
		Camera->SetProjection(ECameraProjectionType::Ortho, 0.f, 1280.f, 720.f, 1000.f);
		Camera->SetInheritRot(false);
	}

	auto Sound = mSound.lock();

	if (Sound)
	{
		Sound->SetSound("TeemoSmile", "Effect", true, "TeemoSmile.mp3");
		Sound->SetMinMaxDistance(100.f, 700.f);

		Sound->Play();
	}

	auto Box = mBox.lock();

	if (Box)
	{
		Box->SetInheritScale(false);
		Box->SetBoxSize(150.f, 150.f);
		Box->SetDebugDraw(true);
		Box->SetCollisionProfile("Monster");
	}


	auto Sphere = mSphere.lock();

	if (Sphere)
	{
		Sphere->SetInheritScale(false);
		Sphere->SetRadius(120.f);
		Sphere->SetDebugDraw(true);
		Sphere->SetCollisionProfile("BlockAll");
	}


	auto Status = mStatus.lock();

	if (Status)
	{
		Status->SetDeathFunction(this, &CMonster::Destroy);
	}

	auto AIComp = mAIComp.lock();

	if (AIComp)
	{
		AIComp->AddState<CMonsterAttackState>("Attack");
		AIComp->AddState<CMonsterMoveState>("Move");

		AIComp->ChangeState("Move");
	}

	return true;
}

void CMonster::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);

	//몬스터가 플레이어를 따라가는법
	//if (!mPlayer.expired())
	//{
	//	FVector3 PlayerPos = mPlayer.lock()->GetWorldPos();
	//	FVector3 MonsterPos = GetWorldPos();
	//	if (PlayerPos.Distance(MonsterPos) > mRange)
	//	{
	//		//방향은 몬스터가 플레이어를 향한 방향
	//		FVector3 Dir = PlayerPos - MonsterPos;

	//		Dir.Normalize();

	//		AddRelativePos(Dir * mSpeed * DeltaTime);
	//	}
	//}

	//총알을 0.5초마다 발사

	mTime -= DeltaTime;

	if (mTime <= 0.f)
	{
		mTime = 0.5f;
		//Fire();
	}


}

void CMonster::Destroy()
{
	CActor::Destroy();
}

void CMonster::OnCollision(const std::weak_ptr<CActor>& HitActor)
{
	LOG_DEBUG("Monster Hit");
}

float CMonster::TakeDamage(float fDamage, const std::string& Instigator, const std::weak_ptr<CActor>& CauseActor)
{
	float Damage = CActor::TakeDamage(fDamage, Instigator, CauseActor);

	auto Status = mStatus.lock();

	if (Status)
	{
		Status->AddHP(-Damage);


		LOG_DEBUG("Monster HP : ", Status->GetHP());
	}
	//mHP -= Damage /*-defensePoint*/;
	/*if (mHP <= 0.f)
	{
		Destroy();
	}*/

	return Damage;
}



void CMonster::Fire()
{
	auto World = mWorld.lock();

	if (World)
	{
		auto Player = mPlayer.lock();

		if (Player)
		{
			auto Bullet = World->CreateActor<CBullet>("Bullet").lock();

			if (Bullet)
			{
				//플레이어를 향해 발사

				FVector3 BulletPos = GetWorldPos();

				Bullet->SetWorldPos(BulletPos);

				FVector3 Dir = Player->GetWorldPos() - Bullet->GetWorldPos();
				Dir.Normalize();

				Bullet->SetMoveDir(Dir);
				Bullet->SetActorTag("Monster");
			}
		}
	}
}

void CMonster::DebugMessage()
{
	LOG_DEBUG("Monster");
}
