#pragma once

#include "World/World.h"
#include "World/WidgetContainer.h"

class CEditorWorld :
    public CWorld
{
public:
	CEditorWorld();
	virtual ~CEditorWorld();

protected:
	std::weak_ptr<CWidgetContainer> mContentPanel;
	std::weak_ptr<CWidgetContainer> mInspectorPanel;
	std::weak_ptr<class CMaterialEditorUI>  mMaterialEditorPanel;
	std::weak_ptr<class CAnimEditorUI>      mAnimEditorPanel;
	std::weak_ptr<class CSpriteViewerUI>    mSpriteViewerPanel;
	bool mEditorPanelsVisible = true;

public:
	virtual bool Init();

public:
	void ToggleEditorPanels();
};

