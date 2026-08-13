#include "EditorPlayer.h"

#include "World/CameraComponent.h"
#include "World/TileMapActor.h"
#include "World/TileMapComponent.h"
#include "World/Input.h"
#include "World/UIManager.h"

#include "EditorWorld.h"

#include "TimeManager.h"

#include "Asset/PathManager.h"
#include "Engine.h"

CEditorPlayer::CEditorPlayer()
{}

CEditorPlayer::CEditorPlayer(const CEditorPlayer& src)
{}

CEditorPlayer::CEditorPlayer(CEditorPlayer&& src) noexcept
{}

CEditorPlayer::~CEditorPlayer()
{}

bool CEditorPlayer::Init()
{
	mCamera = CreateComponent<CCameraComponent>("EditorCamera");

	auto Input = mWorld.lock()->GetInput().lock();

	if (Input)
	{
		Input->AddBindKey("MoveForward", 'W');
		Input->SetBindFunction("MoveForward", EInputType::Hold, this, &CEditorPlayer::MoveForward);
		Input->AddBindKey("MoveBackward", 'S');
		Input->SetBindFunction("MoveBackward", EInputType::Hold, this, &CEditorPlayer::MoveBackward);
		Input->AddBindKey("MoveRight", 'D');
		Input->SetBindFunction("MoveRight", EInputType::Hold, this, &CEditorPlayer::MoveRight);
		Input->AddBindKey("MoveLeft", 'A');
		Input->SetBindFunction("MoveLeft", EInputType::Hold, this, &CEditorPlayer::MoveLeft);

		Input->AddBindKey("LeftClick", VK_LBUTTON);
		Input->SetBindFunction("LeftClick", EInputType::Hold, this, &CEditorPlayer::LeftClick);

		Input->AddBindKey("RightDrag", VK_RBUTTON);
		Input->SetBindFunction("RightDrag", EInputType::Hold, this, &CEditorPlayer::RightDrag);

		Input->AddBindKey("ChangeMode", '1');
		Input->SetBindFunction("ChangeMode", EInputType::Press, this, &CEditorPlayer::ChangeMode);
		Input->AddBindKey("ChangeTileType", '2');
		Input->SetBindFunction("ChangeTileType", EInputType::Press, this, &CEditorPlayer::ChangeTileType);
		Input->AddBindKey("ChangeFrame", '3');
		Input->SetBindFunction("ChangeFrame", EInputType::Press, this, &CEditorPlayer::ChangeFrame);

		Input->AddBindKey("Save", 'S');
		Input->SetBindFunction("Save", EInputType::Press, this, &CEditorPlayer::Save);
		Input->SetKeyCtrl("Save", true);
		Input->AddBindKey("Load", 'L');
		Input->SetBindFunction("Load", EInputType::Press, this, &CEditorPlayer::Load);
		Input->SetKeyCtrl("Load", true);

	}

	return true;
}

void CEditorPlayer::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);

	if (mTileMap.expired())
	{
		auto World = mWorld.lock();

		if (World)
		{
			mTileMap = World->FindActor<CTileMapActor>("TileMap");
		}
	}

	if (mEditorWidget.expired())
	{
		auto World = mWorld.lock();

		if (World)
		{
			auto UIMgr = World->GetUIManager().lock();

			if (UIMgr)
			{
				mEditorWidget = UIMgr->FindWidget<CEditorWidget>("EditorWidget");
			}
		}
	}

	auto Tile = GetTile().lock();

	if (Tile)
	{
		if (mHoveredTile != Tile)
		{
			if (mHoveredTile)
			{
				mHoveredTile->SetOutLineColor(mPrevColor);
			}

			mHoveredTile = Tile;
			mPrevColor = Tile->GetOutLineColor();
			Tile->SetOutLineColor(FVector4(1.f, 1.f, 0.f, 1.f));
		}
	}
	else
	{
		if (mHoveredTile)
		{
			mHoveredTile->SetOutLineColor(mPrevColor);
		}

		mHoveredTile.reset();
	}

}

void CEditorPlayer::MoveForward()
{
	AddRelativePos(0.f, CTimeManager::GetDeltaTime() * 500.f, 0.f);
}

void CEditorPlayer::MoveBackward()
{
	AddRelativePos(0.f, -CTimeManager::GetDeltaTime() * 500.f, 0.f);
}

void CEditorPlayer::MoveRight()
{
	AddRelativePos(CTimeManager::GetDeltaTime() * 500.f, 0.f, 0.f);
}

void CEditorPlayer::MoveLeft()
{
	AddRelativePos(-CTimeManager::GetDeltaTime() * 500.f, 0.f, 0.f);
}

void CEditorPlayer::RightDrag()
{
	auto World = mWorld.lock();
	if (!World) return;
	auto Input = World->GetInput().lock();
	if (!Input) return;
	FVector2 Delta = Input->GetMouseMove();
	if (Delta.x == 0.f && Delta.y == 0.f) return;
	// screen Y+ = down, world Y+ = up → negate Y
	AddRelativePos(Delta.x, -Delta.y, 0.f);
}

void CEditorPlayer::LeftClick()
{
	auto Tile = GetTile().lock();
	auto TileMap = mTileMap.lock();

	if (Tile && TileMap)
	{
		auto TileMapComp = TileMap->GetTileMapComp().lock();

		if (TileMapComp)
		{
			switch (mEditorMode)
			{
			case EEditorMode::TileType:
				if (mTileType == ETileType::Normal)
				{
					mPrevColor = FVector4::White;
				}
				else if (mTileType == ETileType::Wall)
				{
					mPrevColor = FVector4::Red;
				}

				Tile->SetTileType(mTileType);
				Tile->SetOutLineColor(FVector4(1.f, 1.f, 0.f, 1.f));
				break;
			case EEditorMode::TileFrame:
				TileMapComp->SetTileFrame(Tile->GetIndex(), mEditorFrame);
				break;
			default:
				break;
			}
		}
	}
}

void CEditorPlayer::ChangeMode()
{
	mEditorMode = mEditorMode == EEditorMode::TileType ? EEditorMode::TileFrame : EEditorMode::TileType;

	auto Widget = mEditorWidget.lock();

	if (Widget)
	{
		Widget->SetEditorMode(mEditorMode);
	}

}

void CEditorPlayer::ChangeTileType()
{
	int Type = (int)mTileType;
	++Type;
	if (Type >= (int)ETileType::End)
	{
		Type = 0;
	}

	mTileType = (ETileType)Type;
	auto Widget = mEditorWidget.lock();

	if (Widget)
	{
		Widget->SetTileType(mTileType);
	}


}

void CEditorPlayer::ChangeFrame()
{
	++mEditorFrame;

	auto TileMap = mTileMap.lock();

	if (TileMap)
	{
		auto TileMapComp = TileMap->GetTileMapComp().lock();

		if (TileMapComp)
		{
			if (mEditorFrame >= TileMapComp->GetTileFrameCount())
			{
				mEditorFrame = 0;
			}
		}
	}

	auto Widget = mEditorWidget.lock();

	if (Widget)
	{
		Widget->SetTileFrame(mEditorFrame);
	}
}

void CEditorPlayer::Save()
{
	OPENFILENAME ofn = {};
	TCHAR FullPath[MAX_PATH] = {};

	//파일 대화상자를 열어준다.
	//대화상자를 위한 필터를 설정해준다.
	//첫번째 문자열("모든파일") 사용자에게 보여지는 텍스트
	//두번째 문자열("*.*") 실제 필터링에 사용될 확장자 패턴
	//마지막 문자열 문자열의 끝을 알려주는 것 널문자가 두개 붙어있어야한다.
	TCHAR Filter[] = TEXT("모든파일\0*.*\0Map파일\0*.tlm\0");
	TCHAR Path[MAX_PATH] = {};

	const TCHAR* AssetPath = CPathManager::FindPath("Asset");
	lstrcpy(Path, AssetPath);
	lstrcat(Path, TEXT("Map\\"));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = CEngine::GetInst()->GetWindowHandle();
	ofn.lpstrFilter = Filter;
	ofn.lpstrDefExt = TEXT("tlm");
	ofn.lpstrInitialDir = Path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = FullPath;

	ShowCursor(true);

	if (GetSaveFileName(&ofn) !=0)
	{
		auto TileMap = mTileMap.lock();

		if (TileMap)
		{
			auto TileMapComp = TileMap->GetTileMapComp().lock();

			if (TileMapComp)
			{
				TileMapComp->SaveFullPath(FullPath);
			}
		}
	}

	ShowCursor(false);
}

void CEditorPlayer::Load()
{
	OPENFILENAME ofn = {};
	TCHAR FullPath[MAX_PATH] = {};

	//파일 대화상자를 열어준다.
	//대화상자를 위한 필터를 설정해준다.
	//첫번째 문자열("모든파일") 사용자에게 보여지는 텍스트
	//두번째 문자열("*.*") 실제 필터링에 사용될 확장자 패턴
	//마지막 문자열 문자열의 끝을 알려주는 것 널문자가 두개 붙어있어야한다.
	TCHAR Filter[] = TEXT("모든파일\0*.*\0Map파일\0*.tlm\0");
	TCHAR Path[MAX_PATH] = {};

	const TCHAR* AssetPath = CPathManager::FindPath("Asset");
	lstrcpy(Path, AssetPath);
	lstrcat(Path, TEXT("Map\\"));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = CEngine::GetInst()->GetWindowHandle();
	ofn.lpstrFilter = Filter;
	ofn.lpstrDefExt = TEXT("tlm");
	ofn.lpstrInitialDir = Path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = FullPath;

	ShowCursor(true);

	if (GetOpenFileName(&ofn) != 0)
	{
		auto TileMap = mTileMap.lock();

		if (TileMap)
		{
			auto TileMapComp = TileMap->GetTileMapComp().lock();

			if (TileMapComp)
			{
				TileMapComp->LoadFullPath(FullPath);
			}
		}
	}

	ShowCursor(false);
}

std::weak_ptr<class CTile> CEditorPlayer::GetTile()
{
	auto World = mWorld.lock();

	if (World)
	{
		auto Input = World->GetInput().lock();

		if (Input)
		{
			FVector2 MousePos = Input->GetWorldMousePos();
			auto TileMap = mTileMap.lock();

			if (TileMap)
			{
				auto TileMapComp = TileMap->GetTileMapComp().lock();

				if (TileMapComp)
				{
					return TileMapComp->GetTile(MousePos);
				}
			}
		}
	}

	return std::weak_ptr<class CTile>();
}
