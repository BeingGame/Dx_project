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

#include "CSVReader.h"

#include "World/UIManager.h"
#include "MainUI.h"
#include "World/Button.h"

#include "LogManager.h"

#include "RenderManager.h"
#include "World/MouseWidget.h"

#include "World/TileMapActor.h"
#include "World/TileMapComponent.h"
#include "World/TileMapRender.h"

CMainWorld::CMainWorld()
{}

CMainWorld::~CMainWorld()
{}

bool CMainWorld::Init()
{
	CWorld::Init();

	auto SoundMgr = CAssetManager::GetInst()->GetSubManager<CSoundManager>(EAssetType::Sound);

	if (SoundMgr)
	{
		//SoundMgr->LoadSound("MainBGM", "BGM", true, "MainBgm.mp3");
		//SoundMgr->Play("MainBGM");
	}

	AddAnimation();

	auto Player = CreateActor<CPlayer>("Player").lock();

	if (Player)
	{
		//Player->SetRadius(100.f);

		mCameraList.push_back(Player->GetCamera());
	}

	auto Monster = CreateActor<CMonster>("Monster").lock();

	if (Monster)
	{
		//Monster->SetRadius(100.f);

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

	//Monster = CreateActor<CMonster>("Monster").lock();

	//if (Monster)
	//{
	//	//Monster->SetRadius(100.f);
	//	Monster->SetWorldPos(200.f, -200.f);
	//	Monster->SetPlayer(Player);
	//	mCameraList.push_back(Monster->GetCamera());
	//	Player->AddMonster(Monster);

	//}

	//Monster = CreateActor<CMonster>("Monster").lock();

	//if (Monster)
	//{
	//	//Monster->SetRadius(100.f);
	//	Monster->SetWorldPos(-200.f, 200.f);
	//	Monster->SetPlayer(Player);
	//	mCameraList.push_back(Monster->GetCamera());

	//	Player->AddMonster(Monster);
	//}


	//충돌 
	//실습1
	//물체를 이동시켜서 반사벡터를 이용해서 충돌 지점의 Normal 벡터를 이용해서 거울처럼 반사시킨다.

	//실습2
	//블랙홀처럼 충돌체의 중심점으로 끌어당기는 액터 만들기

	//실습3
	//일방통행할수 있는 벽 만들기
	//ex)오른쪽으로 이동할때만 통과할수 있는 벽



	auto TestDataList = CSVReader::ReadData<FTestCSVData>(TEXT("TestCSV.csv"));

	for (const auto& TestData : TestDataList)
	{
		std::wstring str = L"Name : " + TestData.Name + L" Value1: " + std::to_wstring(TestData.Value1) + L" Value2: " + std::to_wstring(TestData.Value2) + L"\n";

		OutputDebugString(str.c_str());
	}

	auto UIMgr = GetUIManager().lock();

	if (UIMgr)
	{
		auto MainUI = UIMgr->CreateWidgetContainer<CMainUI>("MainUI", 0).lock();

		if (MainUI)
		{
			MainUI->SetPos(200.f, 200.f);
			MainUI->SetOnFunc(Player.get(), &CPlayer::DebugMessage);
			MainUI->SetOnFunc(Monster.get(), &CMonster::DebugMessage);
		}
	}

	auto TileMap = CreateActor<CTileMapActor>("TestTileMap").lock();

	if (TileMap)
	{
		auto TileMapComp = TileMap->GetTileMapComp().lock();

		if (TileMapComp)
		{
			TileMapComp->CreateTile(ETileShape::Rect, 10, 10, FVector2(100.f, 100.f), -1, true);

			TileMapComp->SetTileTexture(ETileTextureType::Back, "BackTexture", TEXT("MapBackGround.png"));
			TileMapComp->SetTileTexture(ETileTextureType::Tile, "TileTexture", TEXT("Floors.png"));

			//스프라이트시트인 타일텍스처의 프레임을 지정해준다.
			for (int i = 0; i < 5; ++i)
			{
				TileMapComp->AddTileFrame(0.f, i * 64.f, 64.f, 64.f + i * 64.f);
			}

			for (int i = 0; i < 5; ++i)
			{
				TileMapComp->SetTileFrame(i, i);
			}

		}
	}



	auto Mouse = CRenderManager::GetInst()->SetMouseWidget<CMouseWidget>(EMouseState::Normal, "MouseNormal").lock();

	std::vector<const TCHAR*> TextureFileName;

	for (int i = 0; i <= 12; ++i)
	{
		TCHAR* FileName = new TCHAR[MAX_PATH];
		memset(FileName, 0, sizeof(TCHAR) * MAX_PATH);
		wsprintf(FileName, TEXT("Mouse/Default/%d.png"), i);
		TextureFileName.push_back(FileName);
	}

	//마우스 위젯 설정
	Mouse->SetSize(32.f, 31.f);
	Mouse->SetTexture("MouseNormal", TextureFileName);

	Mouse->AddBrushFrame(TextureFileName.size(), 0.f, 0.f, 32.f, 31.f);
	Mouse->SetBrushAnimation(true);
	Mouse->SetAnimationType(EAnimation2DTextureType::Frame);
	Mouse->SetAnimationPlayTime(1.f);

	for (int i = 0; i <= 12; ++i)
	{
		delete[] TextureFileName[i];
	}

	TextureFileName.clear();

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

		//0,1
		Anim->AddFrame(6, FVector2(0.f, 0.f), FVector2(1.f, 1.f));

		AnimMgr->CreateAnimation("PlayerIdle");

		Anim = AnimMgr->FindAnimation("PlayerIdle").lock();

		Anim->SetAnimationTextureType(EAnimation2DTextureType::SpriteSheet);
		Anim->SetTexture("PlayerSheet", TEXT("Player/Player.png"));

		/*for (int i = 0; i < 5; ++i)
		{
			Anim->AddFrame(FVector2(200.f * i, 0.f), FVector2(200.f, 200.f));
		}*/

		Anim->AddFrame(FVector2(0.f, 0.f), FVector2(50.f, 200.f));
		Anim->AddFrame(FVector2(200.f, 0.f), FVector2(100.f, 200.f));
		Anim->AddFrame(FVector2(400.f, 0.f), FVector2(150.f, 150.f));
		Anim->AddFrame(FVector2(600.f, 0.f), FVector2(200.f, 100.f));
		Anim->AddFrame(FVector2(800.f, 0.f), FVector2(200.f, 50.f));

		//Anim->CalculateFrameRatio();

		AnimMgr->CreateAnimation("PlayerIdleRatio");

		Anim = AnimMgr->FindAnimation("PlayerIdleRatio").lock();

		Anim->SetAnimationTextureType(EAnimation2DTextureType::SpriteSheet);
		Anim->SetTexture("PlayerSheet", TEXT("Player/Player.png"));

		for (int i = 0; i < 5; ++i)
		{
			Anim->AddFrame(FVector2(200.f * i, 0.f), FVector2(200.f, 200.f));
		}

		/*Anim->AddFrame(FVector2(0.f, 0.f), FVector2(50.f, 200.f));
		Anim->AddFrame(FVector2(200.f, 0.f), FVector2(100.f, 200.f));
		Anim->AddFrame(FVector2(400.f, 0.f), FVector2(150.f, 150.f));
		Anim->AddFrame(FVector2(600.f, 0.f), FVector2(200.f, 100.f));
		Anim->AddFrame(FVector2(800.f, 0.f), FVector2(200.f, 50.f));*/

		Anim->CalculateFrameRatio();

		//233,4 : originPos 307, 98 : TargetPos size 35,30

		AnimMgr->CreateAnimation("TinyMove");

		Anim = AnimMgr->FindAnimation("TinyMove").lock();


		Anim->SetAnimationTextureType(EAnimation2DTextureType::SpriteSheet);

		//Frame의 x축 38프레임씩 이동 4에서 시작, 읽는건 x기준 37 더
		//y축은 1부터 시작해서 33 총 uv는 34까지 
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

void CMainWorld::TestButtonHovered()
{
	LOG_DEBUG("Button Hovered");
}

void CMainWorld::TestButtonClicked()
{
	LOG_DEBUG("Button Clicked");
}
