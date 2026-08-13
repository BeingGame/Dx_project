#pragma once

#include "World/Actor.h"
#include "EditorWidget.h"

class CEditorPlayer :
    public CActor
{

public:
	CEditorPlayer();
	CEditorPlayer(const CEditorPlayer& src);
	CEditorPlayer(CEditorPlayer&& src) noexcept;
	virtual ~CEditorPlayer();

protected:
	std::weak_ptr<class CCameraComponent> mCamera;

	std::weak_ptr<class CTileMapActor> mTileMap;
	std::weak_ptr<class CEditorWidget> mEditorWidget;

	EEditorMode mEditorMode = EEditorMode::TileFrame;
	int mEditorFrame = 0;
	ETileType mTileType = ETileType::Normal;
	std::shared_ptr<CTile> mHoveredTile;
	FVector4 mPrevColor = FVector4::White;

public:
	std::weak_ptr<class CCameraComponent> GetCamera() const
	{
		return mCamera;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);

public:
	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void RightDrag();

	void LeftClick();
	void ChangeMode();
	void ChangeTileType();
	void ChangeFrame();

	void Save();
	void Load();

private:
	std::weak_ptr<class CTile> GetTile();

protected:
	virtual CEditorPlayer* Clone()
	{
		return new CEditorPlayer(*this);
	}

};

