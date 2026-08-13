#pragma once

#include "World/WidgetContainer.h"
#include <functional>
#include <map>

class CInspectorUI : public CWidgetContainer
{
public:
	CInspectorUI();
	CInspectorUI(const CInspectorUI& src);
	virtual ~CInspectorUI();

private:
	static constexpr float PANEL_W  = 200.f;
	static constexpr float PANEL_H  = 680.f;
	static constexpr float PANEL_X  = 1080.f;
	static constexpr float PANEL_Y  = 40.f;
	static constexpr float TITLE_H  = 28.f;
	static constexpr float ROW_H    = 20.f;

	static constexpr float HANDLE_SZ = 10.f;

	int mStaticChildCount = 0;
	int mActiveCorner     = -1;
	int mDynIdx           = 0;      // 동적 위젯 고유 이름 카운터

	std::weak_ptr<class CButton>   mBackground;
	std::weak_ptr<class CTitleBar> mTitleBarWidget;
	std::weak_ptr<class CButton>   mHandleTL, mHandleTR, mHandleBL, mHandleBR;

	std::weak_ptr<class CActor> mTarget;

	// 실시간 트랜스폼 표시용 미리 할당된 텍스트 행
	std::weak_ptr<class CTextBlock> mTitleText;
	std::weak_ptr<class CTextBlock> mActorNameText;
	std::weak_ptr<class CTextBlock> mActorTagText;
	std::weak_ptr<class CTextBlock> mTransformHeader;
	std::weak_ptr<class CTextBlock> mRootNameText;
	std::weak_ptr<class CTextBlock> mPosText;
	std::weak_ptr<class CTextBlock> mRotText;
	std::weak_ptr<class CTextBlock> mScaleText;
	std::weak_ptr<class CTextBlock> mCompHeader;

	// ── 동적 컴포넌트 섹션 ──────────────────────────────────────────────────
	float mComponentsStartY = 0.f;
	std::vector<std::shared_ptr<class CWidget>> mDynamicRows; // 모든 동적 위젯

	// 속성 항목 (행 하나: 레이블 / [-] / 값 / [+])
	struct FInspProp
	{
		std::weak_ptr<class CTextBlock> ValueLabel;
		std::weak_ptr<class CButton>    MinusBtn;
		std::weak_ptr<class CButton>    PlusBtn;
		float Step          = 1.f;
		float MinHoldTime   = 0.f;   int MinRepeat  = 0;
		float PlusHoldTime  = 0.f;   int PlusRepeat = 0;
		std::function<float()>      Getter;
		std::function<void(float)>  Setter;
	};

	// 액션 행: 사이클 또는 탐색 버튼 (+/- 없음)
	struct FInspAction
	{
		std::weak_ptr<class CButton>    ActionBtn;
		std::weak_ptr<class CTextBlock> DisplayLbl;
		std::function<void()>           OnClick;
		std::function<std::string()>    GetDisplay;
	};

	// 콤보/드롭다운 행
	struct FInspDropdown
	{
		void*                           DropKey  = nullptr;
		std::weak_ptr<class CButton>    ToggleBtn;
		std::weak_ptr<class CTextBlock> ValueLbl;
		std::vector<std::string>        Items;
		std::vector<std::weak_ptr<class CButton>> ItemBtns;
		std::function<std::string()>             Getter;
		std::function<void(const std::string&)>  Setter;
	};

	// 컴포넌트 항목 (헤더 + 선택적 확장 속성 목록)
	struct FInspCompEntry
	{
		void*                           CompKey   = nullptr;
		std::weak_ptr<class CButton>    HeaderBtn;
		std::weak_ptr<class CTextBlock> HeaderLbl;
		bool                            bExpanded = false;
		std::vector<FInspProp>          Props;
		std::vector<FInspAction>        Actions;
		std::vector<FInspDropdown>      Dropdowns;
	};

	std::vector<FInspCompEntry> mCompEntries;
	std::map<void*, bool>       mExpandState;
	std::map<void*, bool>       mDropdownOpen;   // DropKey 기준

public:
	void SetTarget(std::weak_ptr<class CActor> Actor);
	void Rebuild();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual CInspectorUI* Clone();

private:
	void UpdateAllRowWidths(float NewWidth);
	std::weak_ptr<class CTextBlock> AddRow(float Y, const wchar_t* Text, float FontSize = 13.f);
	void AddPropRow(float& Y,
	                const wchar_t* Label,
	                float Step,
	                std::function<float()>     Getter,
	                std::function<void(float)> Setter,
	                FInspCompEntry& Entry);

	void AddActionRow(float& Y,
	                  const wchar_t* Label,
	                  const wchar_t* BtnText,
	                  std::function<void()>        OnClick,
	                  std::function<std::string()> GetDisplay,
	                  FInspCompEntry& Entry);

	void AddSectionHeader(float& Y, const wchar_t* Text);

	void AddDropdownRow(float& Y,
	                    const wchar_t* Label,
	                    void* DropKey,
	                    std::vector<std::string> Items,
	                    std::function<std::string()>            Getter,
	                    std::function<void(const std::string&)> Setter,
	                    FInspCompEntry& Entry);
};
