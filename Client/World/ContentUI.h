#pragma once

#include "World/WidgetContainer.h"
#include <functional>

class CContentUI : public CWidgetContainer
{
public:
	CContentUI();
	CContentUI(const CContentUI& src);
	virtual ~CContentUI();

private:
	static constexpr float PANEL_W   = 200.f;
	static constexpr float PANEL_H   = 680.f;
	static constexpr float TITLE_H   = 28.f;
	static constexpr float ENTRY_H   = 26.f;
	static constexpr float PANEL_X   = 0.f;
	static constexpr float PANEL_Y   = 40.f;

	static constexpr float HANDLE_SZ = 10.f;

	int mStaticChildCount  = 0;
	int mLastActorCount    = -1;
	int mActiveCorner      = -1; // -1=없음 0=좌상 1=우상 2=좌하 3=우하

	std::weak_ptr<class CButton>    mBackground;
	std::weak_ptr<class CTitleBar>  mTitleBarWidget;
	std::weak_ptr<class CTextBlock> mTitleText;
	std::weak_ptr<class CButton>    mHandleTL, mHandleTR, mHandleBL, mHandleBR;

	std::weak_ptr<class CActor> mSelectedActor;

	std::function<void(std::weak_ptr<class CActor>)> mOnActorSelected;

	struct FEntry
	{
		std::weak_ptr<class CActor>    Actor;
		std::weak_ptr<class CButton>   Button;
		int                            Index = 0;
	};
	std::vector<FEntry> mEntries;

public:
	void SetOnActorSelected(std::function<void(std::weak_ptr<class CActor>)> Fn)
	{
		mOnActorSelected = std::move(Fn);
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual CContentUI* Clone();

private:
	void Rebuild();
	void OnEntryClicked(int Index);
	void RefreshSelectionTints();
};
