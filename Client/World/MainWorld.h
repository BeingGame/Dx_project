#pragma once

#include "World/World.h"
#include "World/WidgetContainer.h"

struct FTestCSVData
{
	std::wstring Name;
	int Value1;
	float Value2;

	bool Init(std::vector<std::wstring> Data)
	{
		if (Data.size() != 3)
		{
			return false;
		}


		Name = Data[0];
		Value1 = std::stoi(Data[1]);
		Value2 = std::stof(Data[2]);

		return true;
	}

};

class CMainWorld :
	public CWorld
{
public:
	CMainWorld();
	virtual ~CMainWorld();

protected:
	std::vector<std::weak_ptr<class CCameraComponent>> mCameraList;
	int mCameraIndex = 0;

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

public:
	void ChangeCamera();
	void AddAnimation();

public:
	void TestButtonHovered();
	void TestButtonClicked();

};

