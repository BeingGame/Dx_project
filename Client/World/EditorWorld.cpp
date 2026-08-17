#include "EditorWorld.h"

#include "World/Input.h"

#include "World/TileMapActor.h"
#include "World/TileMapComponent.h"
#include "World/UIManager.h"

#include "EditorPlayer.h"
#include "EditorWidget.h"
#include "EditorMenuBar.h"
#include "ContentUI.h"
#include "InspectorUI.h"
#include "MaterialEditorUI.h"
#include "AnimEditorUI.h"
#include "SpriteViewerUI.h"
#include "windows.h"

CEditorWorld::CEditorWorld()
{}

CEditorWorld::~CEditorWorld()
{}

bool CEditorWorld::Init()
{
	CWorld::Init();

	// Asset\Anim\ 폴더의 모든 .anim2d 자동 로드
	CAnimEditorUI::LoadAllAnims();

	auto Player = CreateActor<CEditorPlayer>("Player");

	auto UIMgr = GetUIManager().lock();

	if (UIMgr)
	{
		UIMgr->CreateWidgetContainer<CEditorWidget>("EditorWidget");
		auto MenuBarWeak = UIMgr->CreateWidgetContainer<CEditorMenuBar>("EditorMenuBar", 100);
		if (auto MB = MenuBarWeak.lock()) MB->SetWorld(GetThisPtr());

		auto Content   = UIMgr->CreateWidgetContainer<CContentUI>("ContentUI", 50);
		auto Inspector = UIMgr->CreateWidgetContainer<CInspectorUI>("InspectorUI", 50);
		if (auto C = Content.lock())   C->SetWorld(GetThisPtr());
		if (auto I = Inspector.lock()) I->SetWorld(GetThisPtr());

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

		if (auto MB = MenuBarWeak.lock())
		{
			auto InspWeak = Inspector;
			auto AnimEdWeak3 = AnimEditorWeak;
			MB->SetOnActorCreated([InspWeak, AnimEdWeak3](std::weak_ptr<CActor> Actor)
			{
				if (auto Insp = InspWeak.lock())
					Insp->SetTarget(Actor);
				if (auto AnimEd = AnimEdWeak3.lock())
					AnimEd->SetTarget(Actor);
			});

			MB->SetOnOpenMaterialEditor([MatEditorWeak]()
			{
				if (auto MatEd = MatEditorWeak.lock())
					MatEd->SetEnable(!MatEd->IsEnable());
			});

			MB->SetOnOpenAnimEditor([AnimEditorWeak]()
			{
				if (auto AnimEd = AnimEditorWeak.lock())
				{
					bool WasEnabled = AnimEd->IsEnable();
					AnimEd->SetEnable(!WasEnabled);
					if (!WasEnabled)
						AnimEd->RefreshTarget();
				}
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

			// Inspector에서 컴포넌트 제거 → MenuBar 프리팹 추적 목록 동기화
			{
				auto MBWeak2 = MenuBarWeak;
				auto AnimEdWeak4 = AnimEditorWeak;
				if (auto Insp = Inspector.lock())
				{
					Insp->SetOnComponentRemoved([MBWeak2, AnimEdWeak4](const std::string& CompName)
					{
						if (auto MB2 = MBWeak2.lock())
							MB2->UntrackComponent(CompName);

						// Anim Editor가 지워진 컴포넌트를 참조하고 있을 수 있으므로 갱신
						if (auto AnimEd = AnimEdWeak4.lock())
							AnimEd->RefreshTarget();
					});
				}
			}
		}

		if (auto C = Content.lock())
		{
			auto InspWeak    = Inspector;
			auto MBWeak      = MenuBarWeak;
			auto MatEdWeak2  = MatEditorWeak;
			auto AnimEdWeak2 = AnimEditorWeak;
			C->SetOnActorSelected([InspWeak, MBWeak, MatEdWeak2, AnimEdWeak2](std::weak_ptr<CActor> Actor)
			{
				if (auto Insp = InspWeak.lock())
					Insp->SetTarget(Actor);
				if (auto MB = MBWeak.lock())
					MB->SetSelectedActor(Actor);
				if (auto MatEd = MatEdWeak2.lock())
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
			this, &CEditorWorld::ToggleEditorPanels);
	}

	auto TileMap = CreateActor<CTileMapActor>("TileMap").lock();

	if (TileMap)
	{
		auto TileMapComp = TileMap->GetTileMapComp().lock();

		if (TileMapComp)
		{
			TileMapComp->CreateTile(ETileShape::Rect, 100, 100, FVector2(100.f, 100.f), -1, true);

			TileMapComp->SetTileTexture(ETileTextureType::Back, "BackTexture", TEXT("MapBackGround.png"));
			TileMapComp->SetTileTexture(ETileTextureType::Tile, "TileTexture", TEXT("Floors.png"));

			//스프라이트시트인 타일텍스처의 프레임을 지정해준다.
			for (int i = 0; i < 5; ++i)
			{
				TileMapComp->AddTileFrame(0.f, i * 64.f, 64.f, 64.f + i * 64.f);
			}
		}
	}

	return true;
}

void CEditorWorld::ToggleEditorPanels()
{
	mEditorPanelsVisible = !mEditorPanelsVisible;

	if (auto C = mContentPanel.lock())
		C->SetEnable(mEditorPanelsVisible);
	if (auto I = mInspectorPanel.lock())
		I->SetEnable(mEditorPanelsVisible);
}
