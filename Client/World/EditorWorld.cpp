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

	auto UIManager = GetUIManager().lock();

	if (UIManager)
	{
		UIManager->CreateWidgetContainer<CEditorWidget>("EditorWidget");
		auto MenuBarWeak = UIManager->CreateWidgetContainer<CEditorMenuBar>("EditorMenuBar", 100);
		if (auto MenuBar = MenuBarWeak.lock()) MenuBar->SetWorld(GetThisPtr());

		auto ContentWeak   = UIManager->CreateWidgetContainer<CContentUI>("ContentUI", 50);
		auto InspectorWeak = UIManager->CreateWidgetContainer<CInspectorUI>("InspectorUI", 50);
		if (auto Content = ContentWeak.lock())   Content->SetWorld(GetThisPtr());
		if (auto Inspector = InspectorWeak.lock()) Inspector->SetWorld(GetThisPtr());

		mContentPanel   = ContentWeak;
		mInspectorPanel = InspectorWeak;

		auto MaterialEditorWeak = UIManager->CreateWidgetContainer<CMaterialEditorUI>("MaterialEditorUI", 200);
		mMaterialEditorPanel = MaterialEditorWeak;
		if (auto MaterialEditor = MaterialEditorWeak.lock()) MaterialEditor->SetWorld(GetThisPtr());

		auto AnimEditorWeak = UIManager->CreateWidgetContainer<CAnimEditorUI>("AnimEditorUI", 75);
		mAnimEditorPanel = AnimEditorWeak;
		if (auto AnimEditor = AnimEditorWeak.lock())
		{
			AnimEditor->SetWorld(GetThisPtr());
			AnimEditor->SetEnable(false);
		}

		auto SpriteViewerWeak = UIManager->CreateWidgetContainer<CSpriteViewerUI>("SpriteViewerUI", 60);
		mSpriteViewerPanel = SpriteViewerWeak;
		if (auto SpriteViewer = SpriteViewerWeak.lock())
		{
			SpriteViewer->SetWorld(GetThisPtr());
			SpriteViewer->SetEnable(false);
		}
		if (auto AnimEditor = AnimEditorWeak.lock())
			AnimEditor->SetSpriteViewer(SpriteViewerWeak);

		if (auto MenuBar = MenuBarWeak.lock())
		{
			auto InspectorForCreate = InspectorWeak;
			auto AnimEditorForCreate = AnimEditorWeak;
			MenuBar->SetOnActorCreated([InspectorForCreate, AnimEditorForCreate](std::weak_ptr<CActor> Actor)
			{
				if (auto Inspector = InspectorForCreate.lock())
					Inspector->SetTarget(Actor);
				if (auto AnimEditor = AnimEditorForCreate.lock())
					AnimEditor->SetTarget(Actor);
			});

			MenuBar->SetOnOpenMaterialEditor([MaterialEditorWeak]()
			{
				if (auto MaterialEditor = MaterialEditorWeak.lock())
					MaterialEditor->SetEnable(!MaterialEditor->IsEnable());
			});

			MenuBar->SetOnOpenAnimEditor([AnimEditorWeak]()
			{
				if (auto AnimEditor = AnimEditorWeak.lock())
				{
					bool WasEnabled = AnimEditor->IsEnable();
					AnimEditor->SetEnable(!WasEnabled);
					if (!WasEnabled)
						AnimEditor->RefreshTarget();
				}
			});

			// Material Editor에서 저장/로드/Assign → InspectorWeak 갱신
			{
				auto InspectorForMaterial = InspectorWeak;
				if (auto MaterialEditor = MaterialEditorWeak.lock())
				{
					MaterialEditor->SetOnMaterialUpdated([InspectorForMaterial]()
					{
						if (auto Inspector = InspectorForMaterial.lock())
							Inspector->Rebuild();
					});
				}
			}

			// Inspector에서 컴포넌트 제거 → MenuBar 프리팹 추적 목록 동기화
			{
				auto MenuBarForRemove = MenuBarWeak;
				auto AnimEditorForRemove = AnimEditorWeak;
				if (auto Inspector = InspectorWeak.lock())
				{
					Inspector->SetOnComponentRemoved([MenuBarForRemove, AnimEditorForRemove](const std::string& CompName)
					{
						if (auto MenuBar = MenuBarForRemove.lock())
							MenuBar->UntrackComponent(CompName);

						// Anim Editor가 지워진 컴포넌트를 참조하고 있을 수 있으므로 갱신
						if (auto AnimEditor = AnimEditorForRemove.lock())
							AnimEditor->RefreshTarget();
					});
				}
			}
		}

		if (auto Content = ContentWeak.lock())
		{
			auto InspectorForSelect    = InspectorWeak;
			auto MenuBarForSelect      = MenuBarWeak;
			auto MaterialEditorForSelect  = MaterialEditorWeak;
			auto AnimEditorForSelect = AnimEditorWeak;
			Content->SetOnActorSelected([InspectorForSelect, MenuBarForSelect, MaterialEditorForSelect, AnimEditorForSelect](std::weak_ptr<CActor> Actor)
			{
				if (auto Inspector = InspectorForSelect.lock())
					Inspector->SetTarget(Actor);
				if (auto MenuBar = MenuBarForSelect.lock())
					MenuBar->SetSelectedActor(Actor);
				if (auto MaterialEditor = MaterialEditorForSelect.lock())
					MaterialEditor->SetSelectedActor(Actor);
				if (auto AnimEditor = AnimEditorForSelect.lock())
					AnimEditor->SetTarget(Actor);
			});
		}
	}

	// F11 : 에디터 패널 토글
	//
	// AddBindKey는 VK 코드를 받는다. DIK 코드를 넘기면 안 된다.
	// (DIK_F12가 0x58이라 그대로 넘겼더니 VK 0x58 = 'X'로 잡혀서
	//  공격키를 누를 때마다 패널이 사라졌다 나타났다 했다)
	//
	// F12는 쓰면 안 된다. 윈도우가 디버거 브레이크용으로 잡아둔 키라
	// 디버거를 붙이고 돌리는 중에 누르면 ntdll에서 멈춰버린다.
	auto Input = GetInput().lock();
	if (Input)
	{
		Input->AddBindKey("ToggleEditorPanels", VK_F11);
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

	if (auto Content = mContentPanel.lock())
		Content->SetEnable(mEditorPanelsVisible);
	if (auto Inspector = mInspectorPanel.lock())
		Inspector->SetEnable(mEditorPanelsVisible);
}
