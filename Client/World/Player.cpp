#include "Player.h"

#include "World/MeshComponent.h"
#include "World/CameraComponent.h"
#include "World/CameraManager.h"
#include "World/Input.h"
#include "World/CharacterMovementComponent.h"
#include "World/ColliderBox2D.h"
#include "World/ColliderSphere2D.h"
#include "World/Animation2DComponent.h"
#include "World/WidgetComponent.h"

#include "World/ProgressBar.h"

#include "TimeManager.h"

#include "MainWorld.h"
#include "Bullet.h"
#include "Monster.h"
#include "Turret.h"
#include "Chaser.h"
#include "Shield.h"
#include "SlowArea.h"
#include "ReflectBullet.h"

#include "Component/TurretSkillComp.h"

#include "LogManager.h"

CPlayer::CPlayer()
{}

CPlayer::CPlayer(const CPlayer& src)
{}

CPlayer::CPlayer(CPlayer&& src) noexcept
{}

CPlayer::~CPlayer()
{}

float CPlayer::GetSpeed()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		return MoveComp->GetMoveSpeed();
	}

	return 0.0f;
}

float CPlayer::GetCurrentSpeed()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		return MoveComp->GetCurrentSpeed();
	}

	return 0.0f;
}

void CPlayer::SetSpeed(float Speed)
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetSpeed(Speed);
	}
}

void CPlayer::SetCurrentSpeed(float Speed)
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetCurrentSpeed(Speed);
	}
}

void CPlayer::AddMonster(std::weak_ptr<class CMonster> Monster)
{
	mMonsterList.push_back(Monster);
}

void CPlayer::SetHP(float HP)
{
	SetHPMax(HP);
}

bool CPlayer::Init()
{
	CActor::Init();

	SetHPMax(20.f);
	SetDeathFunction(this, &CPlayer::Destroy);

	mActorTag = "Player";

	mMeshComp = CreateComponent<CMeshComponent>("Root");
	mCamera = CreateComponent<CCameraComponent>("Camera");
	mBox = CreateComponent<CColliderBox2D>("Box");
	//mSphere = CreateComponent<CColliderSphere2D>("Sphere");
	mWidget = CreateComponent<CWidgetComponent>("Widget");

	mMoveComp = CreateComponent<CCharacterMovementComponent>("MoveComp");
	mSkillComp = CreateComponent<CTurretSkillComp>("TurretSkill");
	mAnimComp = CreateComponent<CAnimation2DComponent>("Anim");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		//Mesh->SetMesh("ColorRect");
		//Mesh->SetShader("Color2D");

		Mesh->SetMesh("TexRect");
		Mesh->SetShader("Material");
		//Mesh->AddTexture(0, "Player", TEXT("teemoChromakey.png"));
		//Mesh->AddTexture(0, "DissolveTexture", TEXT("dissolve_smoke.png"), "Texture", 1);
		Mesh->SetBlendState(0, "AlphaBlend");
		Mesh->SetRelativeScale(300.f, 300.f);

		Mesh->SetChromaKeyEnable(0, true);
		//Mesh->SetUVScrollEnable(0, true);
		//Mesh->SetUVScrollSpeed(0, FVector2(0.f, 0.2f));
	}

	auto Anim = mAnimComp.lock();

	if (Anim)
	{
		Anim->SetUpdateComponent(mMeshComp);

		Anim->AddAnimation("TinyMove", 1.f, 1.f, true);

		Anim->SetUsePalette(true);
		Anim->SetPaletteIndex(1);
	}

	auto Camera = mCamera.lock();

	if (Camera)
	{
		Camera->SetProjection(ECameraProjectionType::Ortho, 0.f, 1280.f, 720.f, 1000.f);
		Camera->SetInheritRot(false);
	}

	auto Box = mBox.lock();

	if (Box)
	{
		Box->SetInheritScale(false);
		Box->SetBoxSize(150.f, 150.f);
		Box->SetDebugDraw(true);
		Box->SetCollisionProfile("OverlapAll");
	}


	auto Sphere = mSphere.lock();

	if (Sphere)
	{
		Sphere->SetInheritScale(false);
		Sphere->SetRadius(120.f);
		Sphere->SetDebugDraw(true);
		Sphere->SetCollisionProfile("Player");
	}

	auto Widget = mWidget.lock();

	if (Widget)
	{
		Widget->SetRelativePos(0.f, 100.f, 0.f);

		auto ProgressBar = Widget->SetWidget<CProgressBar>("Player").lock();

		if (ProgressBar)
		{
			ProgressBar->SetSize(100.f, 30.f);
			ProgressBar->SetTint(EProgressBarImageType::Back, 0.4f, 0.4f, 0.4f, 1.f);
			ProgressBar->SetTint(EProgressBarImageType::Fill, FVector4::Red);
			ProgressBar->SetPercent(1.f);
		}
	}

	//실습
	//버튼을 눌러서
	//카메라를 몬스터 시점으로 변경한다. -> 카메라 맵을 순차적으로 받아와 카메라를 변경한다.
	//특정 버튼을 누르면 다시 플레이어 시점으로 변경한다.


	auto Input = mWorld.lock()->GetInput().lock();

	if (Input)
	{
		Input->AddBindKey("MoveForward", 'W');
		Input->SetBindFunction("MoveForward", EInputType::Hold, this, &CPlayer::MoveForward);
		Input->AddBindKey("MoveBackward", 'S');
		Input->SetBindFunction("MoveBackward", EInputType::Hold, this, &CPlayer::MoveBackward);
		Input->AddBindKey("MoveLeft", 'A');
		Input->SetBindFunction("MoveLeft", EInputType::Hold, this, &CPlayer::MoveLeft);
		Input->AddBindKey("MoveRight", 'D');
		Input->SetBindFunction("MoveRight", EInputType::Hold, this, &CPlayer::MoveRight);

		//회전
		Input->AddBindKey("RotateLeft", 'E');
		Input->SetBindFunction("RotateLeft", EInputType::Hold, this, &CPlayer::RotateLeft);
		Input->AddBindKey("RotateRight", 'Q');
		Input->SetBindFunction("RotateRight", EInputType::Hold, this, &CPlayer::RotateRight);


		Input->AddBindKey("Fire", VK_SPACE);
		Input->SetBindFunction("Fire", EInputType::Press, this, &CPlayer::Fire);

		Input->AddBindKey("ChangeCamera", 'F');
		Input->SetBindFunction("ChangeCamera", EInputType::Press, this, &CPlayer::ChangeCamera);
		Input->AddBindKey("ReturnCamera", 'G');
		Input->SetBindFunction("ReturnCamera", EInputType::Press, this, &CPlayer::ReturnCamera);

		Input->AddBindKey("Skill1", '1');
		Input->SetBindFunction("Skill1", EInputType::Press, this, &CPlayer::NewSkill1);
		Input->AddBindKey("Skill2", '2');
		Input->SetBindFunction("Skill2", EInputType::Press, this, &CPlayer::NewSkill2);
		Input->AddBindKey("Skill3", '3');
		Input->SetBindFunction("Skill3", EInputType::Press, this, &CPlayer::NewSkill3);

		Input->AddBindKey("Skill3", '3');
		Input->SetBindFunction("Skill3", EInputType::Press, this, &CPlayer::NewSkill3);

		Input->AddBindKey("Dissolve", 'T');
		Input->SetBindFunction("Dissolve", EInputType::Press, this, &CPlayer::Dissolve);

		Input->AddBindKey("Hit", 'Y');
		Input->SetBindFunction("Hit", EInputType::Press, this, &CPlayer::Hit);

		//Input->AddBindKey("MouseClick", VK_LBUTTON);
		//Input->SetBindFunction("MouseClick", EInputType::Press, this, &CPlayer::MouseClick);
	}


	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetUpdateComp(mRoot);
		MoveComp->SetSpeed(200.f);
		MoveComp->SetCurrentSpeed(200.f);
	}

	//스킬 실습
	//스킬1
	//각도를 이용해서 n개 이상의 총알을 생성해서 제일 가까운 몬스터에게
	//날아가는 스킬

	//스킬2
	//플레이어 양옆에 포탑을 소환해서 플레이어를 따라다니면서 총알을 본인의 정면으로 발사하는 스킬


	return true;
}

void CPlayer::Update(float DeltaTime)
{
	//실습
	//총알을 발사한다. 발사 방향은 본인이 선택해서
	//방향이 다르면 다른 방향으로 발사되게끔 만든다.

	//몬스터
	//특정위치까지 플레이어에게 접근한다.
	//플레이어위치 기준 100만큼의 위치가 떨어져있어야지 움직인다.

	//float Speed = 100.f * DeltaTime;

	//mForward = 1.f;
	//mRight = 0.f;

	//if (GetAsyncKeyState('W') & 0x8000)
	//{
	//	//축을 가져와서 움직임을 시도한다.
	//	FVector3 Axis = GetAxis(EAxis::Y);

	//	AddRelativePos(Axis * Speed);

	//	mForward = 1.f;
	//}

	//if (GetAsyncKeyState('A') & 0x8000)
	//{

	//	FVector3 Axis = GetAxis(EAxis::X);

	//	AddRelativePos(Axis * -Speed);

	//	mRight = -1.f;
	//}

	//if (GetAsyncKeyState('S') & 0x8000)
	//{
	//	FVector3 Axis = GetAxis(EAxis::Y);

	//	AddRelativePos(Axis * -Speed);

	//	mForward = -1.f;
	//}

	//if (GetAsyncKeyState('D') & 0x8000)
	//{

	//	FVector3 Axis = GetAxis(EAxis::X);

	//	AddRelativePos(Axis * Speed);

	//	mRight = 1.f;
	//}

	//if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	//{
	//	Fire();
	//}

	//if (mbChangeCamera)
	//{
	//	if (GetAsyncKeyState('F') & 0x8000)
	//	{
	//		ChangeCamera();
	//	}

	//	if (GetAsyncKeyState('G') & 0x8000)
	//	{
	//		ReturnCamera();
	//	}

	//	mbChangeCamera = false;
	//}

	CActor::Update(DeltaTime);

	if (mDash)
	{
		mDashDistanceMax -= mDashSpeed * DeltaTime;
		auto MoveComp = mMoveComp.lock();

		if (MoveComp)
		{
			MoveComp->SetMoveDir(mDashDir);

			if (mDashDistanceMax <= 0.f)
			{
				mDash = false;

				MoveComp->SetSpeed(MoveComp->GetCurrentSpeed());
			}
		}
	}

	mUVScrollTime += DeltaTime;

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetUVScrollTime(0, mUVScrollTime);
	}

	if (mIsDissolve)
	{
		mDissolveProgress += mDissolveSpeed * DeltaTime;

		if (mDissolveProgress >= 1.f)
		{
			mDissolveProgress = 1.f;
		}

		if (Mesh)
		{
			Mesh->SetDissolveProgress(0, mDissolveProgress);
		}
	}

	if (mIsHit)
	{
		mHitTime -= DeltaTime;

		if (mHitTime <= 0.f)
		{
			mIsHit = false;
			mHitTime = 0.f;
			if (Mesh)
			{
				Mesh->SetHitEffectEnable(0, false);
			}
		}
		else
		{
			if (Mesh)
			{
				float Intensity = mHitTime / mHitDuration;
				Mesh->SetHitIntensity(0, Intensity);
			}
		}
	}

	auto Widget = mWidget.lock();

	if (Widget)
	{
		auto Bar = Widget->GetWidget();

		if (Bar)
		{
			auto ProgressBar = std::dynamic_pointer_cast<CProgressBar>(Bar);

			if (ProgressBar)
			{
				float Percent = ProgressBar->GetPercent();
				Percent -= DeltaTime * 0.1f;
				if (Percent <= 0.f)
				{
					Percent = 1.f;
				}
				ProgressBar->SetPercent(Percent);
			}
		}
	}

	/*if (!mbChangeCamera)
	{
		mTime -= DeltaTime;

		if (mTime <= 0.f)
		{
			mTime = 1.f;
			mbChangeCamera = true;
		}
	}*/

	//LOG_DEBUG("Player Rot : ", GetWorldRot().z);

	/*auto Input = mWorld.lock()->GetInput().lock();

	if (Input)
	{
		LOG_DEBUG("Mouse Pos : ", Input->GetWorldMousePos().x, " ", Input->GetWorldMousePos().y);
	}*/
}

void CPlayer::Destroy()
{
	CActor::Destroy();
}

float CPlayer::TakeDamage(float fDamage, const std::string& Instigator, const std::weak_ptr<CActor>& CauseActor)
{
	float Damage = CActor::TakeDamage(fDamage, Instigator, CauseActor);

	//AddHP(-Damage);

	LOG_DEBUG("Player HP : ", GetHP());

	return Damage;
}

void CPlayer::MoveForward()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetMoveDir(GetAxis(EAxis::Y));
	}


	LOG_DEBUG("Player ForwardMove MoveDir", MoveComp->GetMoveDir().x, " ", MoveComp->GetMoveDir().y);

	/*float Speed = 100.f * CTimeManager::GetDeltaTime();

	AddRelativePos(GetAxis(EAxis::Y) * Speed);*/
}

void CPlayer::MoveBackward()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetMoveDir(-GetAxis(EAxis::Y));
	}

	/*float Speed = 100.f * CTimeManager::GetDeltaTime();

	AddRelativePos(-GetAxis(EAxis::Y) * Speed);*/
}

void CPlayer::MoveRight()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetMoveDir(GetAxis(EAxis::X));
	}

	LOG_DEBUG("Player RightMove MoveDir", MoveComp->GetMoveDir().x, " ", MoveComp->GetMoveDir().y);


	/*float Speed = 100.f * CTimeManager::GetDeltaTime();

	AddRelativePos(GetAxis(EAxis::X) * Speed);*/
}

void CPlayer::MoveLeft()
{
	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetMoveDir(-GetAxis(EAxis::X));
	}

	/*float Speed = 100.f * CTimeManager::GetDeltaTime();

	AddRelativePos(-GetAxis(EAxis::X) * Speed);*/
}

void CPlayer::RotateLeft()
{
	AddRelativeRotationZ(360.f * CTimeManager::GetDeltaTime());
}

void CPlayer::RotateRight()
{
	AddRelativeRotationZ(-360.f * CTimeManager::GetDeltaTime());
}

void CPlayer::Fire()
{
	LOG_DEBUG("Player MoveDir", mMoveComp.lock()->GetMoveDir().x, " ", mMoveComp.lock()->GetMoveDir().y);

	auto World = mWorld.lock();

	if (World)
	{
		auto Bullet = World->CreateActor<CBullet>("Bullet").lock();
		//auto Bullet = World->CreateActor<CReflectBullet>("ReflectBullet").lock();

		if (Bullet)
		{
			//정면을 향해 발사

			FVector3 Axis = GetAxis(EAxis::Y);
			Axis.Normalize();

			FVector3 BulletPos = GetWorldPos() + Axis * 100.f;

			Bullet->SetMoveDir(Axis);

			Bullet->SetWorldPos(BulletPos);
		}
	}
}

void CPlayer::ChangeCamera()
{
	auto World = std::dynamic_pointer_cast<CMainWorld>(mWorld.lock());

	if (World)
	{
		World->ChangeCamera();
	}
}

void CPlayer::ReturnCamera()
{
	auto World = std::dynamic_pointer_cast<CMainWorld>(mWorld.lock());

	if (World)
	{
		auto CameraMgr = World->GetCameraManager().lock();

		if (CameraMgr)
		{
			CameraMgr->ChangeMainCamera(mCamera);
		}
	}
}

void CPlayer::Skill1()
{
	auto World = mWorld.lock();

	if (World)
	{
		std::list<std::weak_ptr<CMonster>> ObjectList;

		World->FindObjectList<CMonster>("Monster", ObjectList);

		std::weak_ptr<CMonster> Monster;

		auto iter = ObjectList.begin();
		auto iterEnd = ObjectList.end();

		float Distance = FLT_MAX;

		for (; iter != iterEnd; ++iter)
		{
			auto _Monster = iter->lock();

			if (_Monster)
			{
				FVector3 ObjectPos = _Monster->GetWorldPos();

				float MonsterDistance = ObjectPos.Distance(GetWorldPos());

				if (MonsterDistance < Distance)
				{
					Monster = _Monster;
					Distance = MonsterDistance;
				}

			}
		}

		for (int i = 0; i < 12; ++i)
		{
			float Angle = 30.f * i;


			//회전을 이용해서 위치를 잡는 두가지 방법

			//행렬을 이용한 방법
			//FVector3 RotVector(0.f, 0.f, Angle);
			//FMatrix SpawnMat;
			////회전 행렬을 만들어준다.
			//SpawnMat.Rotation(RotVector);

			//FVector3 WorldPos = GetWorldPos();

			////행렬에 이동을 넣어준다.
			//memcpy(&SpawnMat[3][0], &WorldPos, sizeof(FVector3));

			////TransformCoord를 이용해 최종 위치를 구해준다.
			//FVector3 Pos(0.f, 100.f, 0.f);
			//FVector3 BulletPos = Pos.TransformCoord(SpawnMat);

			float Radian = DirectX::XMConvertToRadians(Angle);
			FVector3 Dir(cosf(Radian), sinf(Radian), 0.f);

			FVector3 BulletPos = GetWorldPos() + (Dir * 100.f);

			//삼각함수 이용한 방법

			auto Bullet = World->CreateActor<CBullet>("Bullet").lock();

			auto SharedMonster = Monster.lock();

			if (Bullet)
			{
				//정면을 향해 발사


				Bullet->SetWorldPos(BulletPos);

				FVector3 Axis = SharedMonster->GetWorldPos() - Bullet->GetWorldPos();
				Axis.Normalize();

				Bullet->SetRelativeRotation(GetWorldRot());
				Bullet->SetMoveDir(Axis);
			}

		}
	}
}

void CPlayer::Skill2()
{
	auto Skill = mSkillComp.lock();

	if (Skill)
	{
		if (Skill->SkillProgress())
		{
			LOG_DEBUG("Use Skill ");
		}
		else
		{
			LOG_DEBUG("Not Use Skill ");
		}
	}
}

void CPlayer::Skill3()
{
	//실습
	//몬스터를 따라가는 액터를 스폰한다음에 몬스터와 가까워지면 사라지는 스킬 만들기 
	//액터 스폰은 마우스 월드 좌표를 이용한다.
	auto World = mWorld.lock();
	if (World)
	{
		std::list<std::weak_ptr<CMonster>> ObjectList;

		World->FindObjectList<CMonster>("Monster", ObjectList);

		std::weak_ptr<CMonster> Monster;

		auto iter = ObjectList.begin();
		auto iterEnd = ObjectList.end();

		float Distance = FLT_MAX;

		for (; iter != iterEnd; ++iter)
		{
			auto _Monster = iter->lock();

			if (_Monster)
			{
				FVector3 ObjectPos = _Monster->GetWorldPos();

				float MonsterDistance = ObjectPos.Distance(GetWorldPos());

				if (MonsterDistance < Distance)
				{
					Monster = _Monster;
					Distance = MonsterDistance;
				}

			}
		}

		auto Chaser = World->CreateActor<CChaser>("Chaser").lock();

		if (Chaser)
		{
			Chaser->SetWorldPos(GetWorldPos());
			Chaser->SetTarget(Monster);
		}
	}
}

void CPlayer::NewSkill1()
{
	//투사체를 막는 실드 생성

	size_t Size = mShieldList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Shield = mShieldList[i].lock();

		if (Shield)
		{
			Shield->Destroy();
		}
	}
	mShieldList.clear();

	auto World = mWorld.lock();

	if (World)
	{
		for (int i = 0; i < 3; ++i)
		{
			auto Shield = World->CreateActor<CShield>("Shield").lock();

			if (Shield)
			{
				Shield->SetAngle(GetThisPtr<CActor>(), 120.f * i);
				mShieldList.push_back(Shield);
			}
		}
	}


}

void CPlayer::NewSkill2()
{
	auto World = mWorld.lock();

	if (World)
	{
		auto Input = World->GetInput().lock();

		if (Input)
		{
			auto SlowArea = World->CreateActor<CSlowArea>("SlowArea").lock();

			if (SlowArea)
			{
				SlowArea->SetWorldPos(Input->GetWorldMousePos());
				//SlowArea->SetRadius(750.f);
			}

		}


	}
}

void CPlayer::NewSkill3()
{
	auto World = mWorld.lock();

	if (World)
	{
		auto Input = World->GetInput().lock();

		if (Input)
		{
			FVector3 MouseWorldPos(Input->GetWorldMousePos().x, Input->GetWorldMousePos().y, 0.f);

			FVector3 StartPos = GetWorldPos();

			mDashDir = MouseWorldPos - StartPos;
			mDashDir.Normalize();

			mDash = true;

			mDashDistanceMax = StartPos.Distance(MouseWorldPos);

			auto MoveComp = mMoveComp.lock();

			if (MoveComp)
			{
				MoveComp->SetSpeed(mDashSpeed);
			}

		}
	}
}

void CPlayer::MouseClick()
{
	auto World = mWorld.lock();

	if (World)
	{
		auto Input = World->GetInput().lock();

		auto Turret = World->CreateActor<CTurret>("Turret").lock();

		if (Turret)
		{
			//서로 다른 액터를 부착해준다.

			FVector3 TurretPos(Input->GetWorldMousePos().x, Input->GetWorldMousePos().y, 0.f);

			Turret->SetWorldPos(TurretPos);
			Turret->SetWorldScale(30.f, 30.f);
		}
	}
}

void CPlayer::Dissolve()
{
	mIsDissolve = true;
	mDissolveProgress = 0.f;

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetDissolveEnable(0, true);
	}

}

void CPlayer::TestDamage()
{
	TakeDamage(1.f, "Player", GetThisPtr<CActor>());
}

void CPlayer::Hit()
{
	mIsHit = true;
	mHitTime = mHitDuration;

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetHitEffectEnable(0, true);
	}
}

void CPlayer::DebugMessage()
{
	LOG_DEBUG("Player");
}

//실습1
//투사체를 막는 실드 생성
//플레이어의 주변을 회전하도록 만든다.
//실드가 아직 있을때 스킬을 재사용하면 이전의 실드는 전부 제거하고 새로운 실드를 생성한다.

//실습2
//슬로우 영역 만들기
//특정 지점에 들어온 모든 액테의 속도를 n퍼 이상 늦춘다.
//지점에 빠져나오면 다시 원래속도로 돌려준다.

//실습3
//마우스 위치로 빠른속도로 이동하는 대쉬 기능 만들기