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
#include "World/Input.h"
#include "MainUI.h"
#include "World/Button.h"

#include "LogManager.h"

#include "EditorMenuBar.h"
#include "ContentUI.h"
#include "InspectorUI.h"
#include "MaterialEditorUI.h"
#include "AnimEditorUI.h"
#include "SpriteViewerUI.h"
#include "windows.h"

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

	// Asset\Anim\ 폴더의 모든 .anim2d 자동 로드
	CAnimEditorUI::LoadAllAnims();

	auto UIMgr = GetUIManager().lock();
	if (UIMgr)
	{
		auto MenuBarWeak = UIMgr->CreateWidgetContainer<CEditorMenuBar>("EditorMenuBar", 100);
		auto Content     = UIMgr->CreateWidgetContainer<CContentUI>("ContentUI", 50);
		auto Inspector   = UIMgr->CreateWidgetContainer<CInspectorUI>("InspectorUI", 50);
		if (auto MB = MenuBarWeak.lock()) MB->SetWorld(GetThisPtr());
		if (auto C = Content.lock())      C->SetWorld(GetThisPtr());
		if (auto I = Inspector.lock())    I->SetWorld(GetThisPtr());

		mContentPanel   = Content;
		mInspectorPanel = Inspector;

		auto MatEditorWeak = UIMgr->CreateWidgetContainer<CMaterialEditorUI>("MaterialEditorUI", 200);
		mMaterialEditorPanel = MatEditorWeak;
		if (auto ME = MatEditorWeak.lock()) ME->SetWorld(GetThisPtr());

		auto AnimEditorWeak = UIMgr->CreateWidgetContainer<CAnimEditorUI>("AnimEditorUI", 75);
		mAnimEditorPanel = AnimEditorWeak;
		if (auto AnimEd = AnimEditorWeak.lock())
		{
			AnimEd->SetWorld(GetThisPtr());
			AnimEd->SetEnable(false);
		}

		auto SpriteViewerWeak = UIMgr->CreateWidgetContainer<CSpriteViewerUI>("SpriteViewerUI", 60);
		mSpriteViewerPanel = SpriteViewerWeak;
		if (auto SV = SpriteViewerWeak.lock())
		{
			SV->SetWorld(GetThisPtr());
			SV->SetEnable(false);
		}
		if (auto AnimEd = AnimEditorWeak.lock())
			AnimEd->SetSpriteViewer(SpriteViewerWeak);

		// MenuBar가 액터 생성/컴포넌트 추가 → Inspector 갱신
		if (auto MB = MenuBarWeak.lock())
		{
			auto InspWeak = Inspector;
			MB->SetOnActorCreated([InspWeak](std::weak_ptr<CActor> Actor)
			{
				if (auto Insp = InspWeak.lock())
					Insp->SetTarget(Actor);
			});

			MB->SetOnOpenMaterialEditor([MatEditorWeak]()
			{
				if (auto MatEd = MatEditorWeak.lock())
					MatEd->SetEnable(!MatEd->IsEnable());
			});

			MB->SetOnOpenAnimEditor([AnimEditorWeak]()
			{
				if (auto AnimEd = AnimEditorWeak.lock())
					AnimEd->SetEnable(!AnimEd->IsEnable());
			});

			// Material Editor에서 저장/로드/Assign → Inspector 갱신
			{
				auto InspWeak2 = Inspector;
				if (auto MatEd = MatEditorWeak.lock())
				{
					MatEd->SetOnMaterialUpdated([InspWeak2]()
					{
						if (auto Insp = InspWeak2.lock())
							Insp->Rebuild();
					});
				}
			}
		}

		// ContentUI에서 액터 선택 → Inspector 갱신 + MenuBar 선택 동기화
		if (auto C = Content.lock())
		{
			auto InspWeak    = Inspector;
			auto MBWeak      = MenuBarWeak;
			auto MatEdWeak   = MatEditorWeak;
			auto AnimEdWeak2 = AnimEditorWeak;
			C->SetOnActorSelected([InspWeak, MBWeak, MatEdWeak, AnimEdWeak2](std::weak_ptr<CActor> Actor)
			{
				if (auto Insp = InspWeak.lock())
					Insp->SetTarget(Actor);
				if (auto MB = MBWeak.lock())
					MB->SetSelectedActor(Actor);
				if (auto MatEd = MatEdWeak.lock())
					MatEd->SetSelectedActor(Actor);
				if (auto AnimEd = AnimEdWeak2.lock())
					AnimEd->SetTarget(Actor);
			});
		}
	}

	// F12 : 에디터 패널 토글  (DIK_F12 = 0x58)
	auto Input = GetInput().lock();
	if (Input)
	{
		Input->AddBindKey("ToggleEditorPanels", 0x58);
		Input->SetBindFunction("ToggleEditorPanels", EInputType::Press,
			this, &CMainWorld::ToggleEditorPanels);
	}

	//충돌
	//실습1
	//물체를 이동시켜서 반사벡터를 이용해서 충돌 지점의 Normal 벡터를 이용해서 거울처럼 반사시킨다.

	//실습2
	//블랙홀처럼 충돌체의 중심점으로 끌어당기는 액터 만들기

	//실습3
	//일방통행할수 있는 벽 만들기
	//ex)오른쪽으로 이동할때만 통과할수 있는 벽


	//auto TileMap = CreateActor<CTileMapActor>("TestTileMap").lock();

	//if (TileMap)
	//{
	//	auto TileMapComp = TileMap->GetTileMapComp().lock();

	//	if (TileMapComp)
	//	{
	//		TileMapComp->CreateTile(ETileShape::Rect, 10, 10, FVector2(100.f, 100.f), -1, true);

	//		TileMapComp->SetTileTexture(ETileTextureType::Back, "BackTexture", TEXT("MapBackGround.png"));
	//		TileMapComp->SetTileTexture(ETileTextureType::Tile, "TileTexture", TEXT("Floors.png"));

	//		//스프라이트시트인 타일텍스처의 프레임을 지정해준다.
	//		for (int i = 0; i < 5; ++i)
	//		{
	//			TileMapComp->AddTileFrame(0.f, i * 64.f, 64.f, 64.f + i * 64.f);
	//		}

	//		for (int i = 0; i < 5; ++i)
	//		{
	//			TileMapComp->SetTileFrame(i, i);
	//		}

	//	}
	//}



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

	Mouse->AddBrushFrame((int)TextureFileName.size(), 0.f, 0.f, 32.f, 31.f);
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

void CMainWorld::ToggleEditorPanels()
{
	mEditorPanelsVisible = !mEditorPanelsVisible;

	if (auto C = mContentPanel.lock())
		C->SetEnable(mEditorPanelsVisible);
	if (auto I = mInspectorPanel.lock())
		I->SetEnable(mEditorPanelsVisible);
}

void CMainWorld::TestButtonHovered()
{
	LOG_DEBUG("Button Hovered");
}

void CMainWorld::TestButtonClicked()
{
	LOG_DEBUG("Button Clicked");
}
