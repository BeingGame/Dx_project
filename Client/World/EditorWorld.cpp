#include "EditorWorld.h"

#include "World/TileMapActor.h"
#include "World/TileMapComponent.h"
#include "World/UIManager.h"

#include "EditorPlayer.h"
#include "EditorWidget.h"

CEditorWorld::CEditorWorld()
{}

CEditorWorld::~CEditorWorld()
{}

bool CEditorWorld::Init()
{
	CWorld::Init();

	auto Player = CreateActor<CEditorPlayer>("Player");

	auto UIMgr = GetUIManager().lock();

	if (UIMgr)
	{
		UIMgr->CreateWidgetContainer<CEditorWidget>("EditorWidget");
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
