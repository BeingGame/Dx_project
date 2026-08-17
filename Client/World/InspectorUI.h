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
	static constexpr float DEL_BTN_W = 18.f;   // 컴포넌트 헤더 우측 [X] 버튼 폭

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
		std::weak_ptr<class CButton>    ValueBtn;   // 값 영역 — 더블클릭하면 직접 입력
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
		std::weak_ptr<class CComponent>	CompRef;    // 삭제 대상 컴포넌트
		std::weak_ptr<class CButton>    HeaderBtn;
		std::weak_ptr<class CTextBlock> HeaderLbl;
		std::weak_ptr<class CButton>    DeleteBtn;  // [X] 제거 버튼
		bool                            bExpanded = false;
		std::vector<FInspProp>          Props;
		std::vector<FInspAction>        Actions;
		std::vector<FInspDropdown>      Dropdowns;
	};

	std::vector<FInspCompEntry> mCompEntries;
	std::map<void*, bool>       mExpandState;
	std::map<void*, bool>       mDropdownOpen;   // DropKey 기준

	// 컴포넌트 제거 시 외부 통지 (인자: 제거된 컴포넌트 이름)
	std::function<void(const std::string&)> mOnComponentRemoved;

	// ── 값 직접 입력 (더블클릭 → 타이핑 → Enter) ────────────────────────────
	static constexpr float DOUBLE_CLICK_SEC = 0.35f;  // 더블클릭 인정 간격
	static constexpr int   EDIT_BUF_MAX     = 15;     // 입력 최대 길이

	bool  mKeysRegistered = false;   // 숫자 키 바인딩 등록 여부 (최초 Update에서 1회)
	float mTimeAccum      = 0.f;     // 더블클릭 판정용 누적 시간
	void* mLastClickKey   = nullptr; // 직전에 클릭된 값 버튼
	float mLastClickTime  = -10.f;

	bool        mEditActive  = false;
	int         mEditCompIdx = -1;   // mCompEntries 인덱스
	int         mEditPropIdx = -1;   // Entry.Props 인덱스
	std::string mEditBuffer;         // 타이핑 중인 문자열

public:
	void SetTarget(std::weak_ptr<class CActor> Actor);
	void Rebuild();

	void SetOnComponentRemoved(std::function<void(const std::string&)> Fn)
	{
		mOnComponentRemoved = std::move(Fn);
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual CInspectorUI* Clone();

private:
	void UpdateAllRowWidths(float NewWidth);

	// Rebuild 마무리 — 동적 행을 스크롤 대상으로 표시하고 콘텐츠 길이를 알려준다.
	void FinishLayout(float ContentEndY);

	// 액터 컴포넌트를 펼쳤을 때의 속성 행들 (Animation2DComponent 등)
	void AddActorCompProps(float& Y,
	                       const std::shared_ptr<class CActorComponent>& Comp,
	                       FInspCompEntry& Entry);

	// 씬 컴포넌트 중 콜라이더 전용 속성 행들
	void AddColliderProps(float& Y,
	                      const std::shared_ptr<class CSceneComponent>& Comp,
	                      FInspCompEntry& Entry);
	std::weak_ptr<class CTextBlock> AddRow(float Y, const wchar_t* Text, float FontSize = 13.f);

	// 컴포넌트 헤더 우측에 [X] 제거 버튼을 만든다.
	std::weak_ptr<class CButton> AddDeleteButton(float Y, float PanelW, int I);

	// 삭제 버튼 클릭을 처리한다. 실제로 제거했으면 true (호출부는 즉시 Rebuild 후 반환할 것)
	bool HandleDeleteButtons();

	// ── 값 직접 입력 ────────────────────────────────────────────────────────
	void RegisterEditKeys();        // 숫자/기호 키를 인풋에 등록 (최초 1회)
	void HandleValueEditInput();    // 편집 중 키 입력 처리
	void DetectValueDoubleClick();  // 값 영역 더블클릭 감지 → 편집 시작
	void BeginEdit(int CompIdx, int PropIdx);
	void CommitEdit();              // Enter — 버퍼를 파싱해 Setter 호출
	void CancelEdit();              // Esc / 바깥 클릭 / Rebuild
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
