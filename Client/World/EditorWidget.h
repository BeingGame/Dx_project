#pragma once

#include "World/WidgetContainer.h"
#include "World/Tile.h"

enum class EEditorMode
{
    TileType,
    TileFrame
};

class CEditorWidget :
    public CWidgetContainer
{
public:
	CEditorWidget();
	CEditorWidget(const CEditorWidget& src);
	virtual ~CEditorWidget();

protected:
	std::shared_ptr<class CTextBlock> mModeText;
	std::shared_ptr<class CTextBlock> mTypeText;
	std::shared_ptr<class CTextBlock> mFrameText;

public:
	void SetEditorMode(EEditorMode Mode);
	void SetTileType(ETileType TileType);
	void SetTileFrame(int Frame);

public:
	virtual bool Init();
	virtual CEditorWidget* Clone();
};

