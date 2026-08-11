#include "MainWorld.h"

#include "Player.h"
#include "Monster.h"
#include "BlackHole.h"
#include "OneWayWall.h"
#include "Wall.h"

#include "World/CameraManager.h"
#include "World/CameraComponent.h"

#include "Asset/AssetManager.h"
#include "Asset/AnimationManager.h"

#include "Asset/SoundManager.h"

CMainWorld::CMainWorld()
{}

CMainWorld::~CMainWorld()
{}

bool CMainWorld::Init()
{
	CWorld::Init();

	AddAnimation();

	auto Player = CreateActor<CPlayer>("Player").lock();

	if (Player)
	{
		mCameraList.push_back(Player->GetCamera());
	}

	auto Monster = CreateActor<CMonster>("Monster").lock();

	if (Monster)
	{
		Monster->SetWorldPos(200.f, 200.f);
		mCameraList.push_back(Monster->GetCamera());
	}

	Monster->SetPlayer(Player);
	Player->AddMonster(Monster);

	auto BlackHole = CreateActor<CBlackHole>("BlackHole").lock();

	if (BlackHole)
	{
		BlackHole->SetWorldPos(-600.f, 0.f, 0.f);
	}

	auto Wall = CreateActor<COneWayWall>("Wall").lock();

	if (Wall)
	{
		Wall->SetWorldPos(500.f, 0.f, 0.f);
	}

	auto BlockWall = CreateActor<CWall>("BlockWall").lock();

	if (BlockWall)
	{
		BlockWall->SetWorldPos(-200.f, 200.f, 0.f);
		BlockWall->SetWorldRotationZ(90.f);
	}

	return true;
}

void CMainWorld::ChangeCamera()
{
	auto CameraMgr = GetCameraManager().lock();

	if (CameraMgr)
	{
		auto iter = mCameraList.begin();
		auto iterEnd = mCameraList.end();

		for (; iter != iterEnd;)
		{
			if (iter->expired())
			{
				iter = mCameraList.erase(iter);
				continue;
			}

			++iter;
		}

		if (mCameraIndex >= mCameraList.size())
		{
			mCameraIndex = 0;
		}

		CameraMgr->ChangeMainCamera(mCameraList[mCameraIndex++]);
	}
}

void CMainWorld::AddAnimation()
{
	auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);

	if (AnimMgr)
	{
		AnimMgr->CreateAnimation("PlayerIdleFrame");

		auto Anim = AnimMgr->FindAnimation("PlayerIdleFrame").lock();

		Anim->SetAnimationTextureType(EAnimation2DTextureType::Frame);

		std::vector<const TCHAR*> Frames;

		Frames.push_back(TEXT("Player/PlayerFrame/frame0.png"));
		Frames.push_back(TEXT("Player/PlayerFrame/frame1.png"));
		Frames.push_back(TEXT("Player/PlayerFrame/frame2.png"));
		Frames.push_back(TEXT("Player/PlayerFrame/frame3.png"));
		Frames.push_back(TEXT("Player/PlayerFrame/frame4.png"));
		Frames.push_back(TEXT("Player/PlayerFrame/frame5.png"));

		Anim->SetTexture("PlayerIdleFrame", Frames);

		Anim->AddFrame(6, FVector2(0.f, 0.f), FVector2(1.f, 1.f));

		AnimMgr->CreateAnimation("PlayerIdle");

		Anim = AnimMgr->FindAnimation("PlayerIdle").lock();

		Anim->SetAnimationTextureType(EAnimation2DTextureType::SpriteSheet);
		Anim->SetTexture("PlayerSheet", TEXT("Player/Player.png"));

		Anim->AddFrame(FVector2(0.f, 0.f), FVector2(50.f, 200.f));
		Anim->AddFrame(FVector2(200.f, 0.f), FVector2(100.f, 200.f));
		Anim->AddFrame(FVector2(400.f, 0.f), FVector2(150.f, 150.f));
		Anim->AddFrame(FVector2(600.f, 0.f), FVector2(200.f, 100.f));
		Anim->AddFrame(FVector2(800.f, 0.f), FVector2(200.f, 50.f));

		AnimMgr->CreateAnimation("PlayerIdleRatio");

		Anim = AnimMgr->FindAnimation("PlayerIdleRatio").lock();

		Anim->SetAnimationTextureType(EAnimation2DTextureType::SpriteSheet);
		Anim->SetTexture("PlayerSheet", TEXT("Player/Player.png"));

		for (int i = 0; i < 5; ++i)
		{
			Anim->AddFrame(FVector2(200.f * i, 0.f), FVector2(200.f, 200.f));
		}

		Anim->CalculateFrameRatio();

		AnimMgr->CreateAnimation("TinyMove");

		Anim = AnimMgr->FindAnimation("TinyMove").lock();

		Anim->SetAnimationTextureType(EAnimation2DTextureType::SpriteSheet);
		Anim->SetTexture("TinySheet", TEXT("Tiny.png"));

		for (int i = 0; i < 11; ++i)
		{
			Anim->AddFrame(5.f + (i * 38.f), 2.f, 35.f, 32.f);
		}

		for (int i = 0; i < 5; ++i)
		{
			Anim->AddFrame(5.f + (i * 38.f), 37.f, 35.f, 32.f);
		}

		Anim->MakePaletteColor(FVector2(233.f, 2.f), FVector2(5.f, 177.f), FVector2(35.f, 32.f));
	}
}
