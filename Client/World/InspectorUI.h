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
	//속성 이름이 두 줄로 접혀서 잘리지 않을 만큼은 넓어야 한다.
	//(ComboReset / JumpHeight 같은 열 글자짜리가 기준이다)
	static constexpr float PANEL_W  = 240.f;
	static constexpr float PANEL_H  = 680.f;
	//화면이 1280이므로 오른쪽 끝에 붙여둔다. 폭을 바꾸면 여기도 같이 바꾼다.
	static constexpr float PANEL_X  = 1040.f;
	static constexpr float PANEL_Y  = 40.f;
	static constexpr float TITLE_H  = 28.f;
	static constexpr float ROW_H    = 20.f;
	//컴포넌트 헤더는 이름과 타입명을 두 줄로 나눠 적는다.
	//"CCharacterMovementComponent" 같은 긴 이름이 한 줄에 안 들어가기 때문이다.
	static constexpr float HEADER_H = ROW_H * 2.f;

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

	// 이름 행 위에 깔아두는 투명 버튼. 더블클릭하면 이름을 직접 입력해 바꾼다.
	// (CTextBlock은 클릭을 안 먹으므로 아래에 버튼을 둬도 이게 더블클릭을 받는다)
	std::weak_ptr<class CButton>    mActorNameButton;

	// 애니메이션 타입 선택 콤보 (Tag 아래). 클릭하면 목록이 아래로 펼쳐진다.
	// 타입 목록은 Asset\Anim\ 하위 폴더에서 매번 스캔한다. (CAnimRegistry::ScanTypes)
	std::weak_ptr<class CTextBlock> mAnimTypeText;    // "Type: <이름> ▼" 표시
	std::weak_ptr<class CButton>    mAnimTypeButton;  // 열기/닫기 토글
	bool  mAnimTypeOpen = false;                      // 목록이 펼쳐져 있는가
	float mAnimTypeRowY = 0.f;                        // 타입 행의 Y (목록 위치 계산용)

	// 펼쳐졌을 때의 항목 버튼과 그에 대응하는 타입 문자열. (""=공용)
	// 목록은 매 Rebuild 때 다시 만들고, 폴링에서 이 쌍을 보고 선택을 처리한다.
	std::vector<std::weak_ptr<class CButton>> mAnimTypeItemButtons;
	std::vector<std::string>                  mAnimTypeOptions;

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
		std::weak_ptr<class CButton>    ValueButton;   // 값 영역 — 더블클릭하면 직접 입력
		std::weak_ptr<class CButton>    MinusButton;
		std::weak_ptr<class CButton>    PlusButton;
		float Step          = 1.f;
		float MinHoldTime   = 0.f;   int MinRepeat  = 0;
		float PlusHoldTime  = 0.f;   int PlusRepeat = 0;
		std::function<float()>      Getter;
		std::function<void(float)>  Setter;
	};

	// 액션 행: 사이클 또는 탐색 버튼 (+/- 없음)
	struct FInspAction
	{
		std::weak_ptr<class CButton>    ActionButton;
		std::weak_ptr<class CTextBlock> DisplayLabel;
		std::function<void()>           OnClick;
		std::function<std::string()>    GetDisplay;

		//같은 문자열을 매 프레임 다시 넣지 않으려고 직전 값을 기억해둔다.
		std::string                     LastDisplay;
	};

	// 콤보/드롭다운 행
	struct FInspDropdown
	{
		void*                           DropKey  = nullptr;
		std::weak_ptr<class CButton>    ToggleButton;
		std::weak_ptr<class CTextBlock> ValueLabel;
		std::vector<std::string>        Items;
		std::vector<std::weak_ptr<class CButton>> ItemButtons;
		std::function<std::string()>             Getter;
		std::function<void(const std::string&)>  Setter;
	};

	// 컴포넌트 항목 (헤더 + 선택적 확장 속성 목록)
	struct FInspCompEntry
	{
		void*                           CompKey   = nullptr;
		std::weak_ptr<class CComponent>	CompRef;    // 삭제 대상 컴포넌트
		std::weak_ptr<class CButton>    HeaderButton;
		std::weak_ptr<class CTextBlock> HeaderLabel;
		std::weak_ptr<class CButton>    DeleteButton;  // [X] 제거 버튼
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

	// 액터 이름을 직접 입력해 바꾸는 중인지. (숫자 값 편집과 별개 경로다)
	bool        mEditNameActive = false;
	static constexpr int NAME_BUF_MAX = 31;   // 이름 최대 길이

public:
	void SetTarget(std::weak_ptr<class CActor> Actor);
	void Rebuild();

	void SetOnComponentRemoved(std::function<void(const std::string&)> Function)
	{
		mOnComponentRemoved = std::move(Function);
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual CInspectorUI* Clone();

private:
	//액션 상태 컴포넌트에서 지금 편집 중인 상태. (9개를 전부 펼치면 너무 길어진다)
	int mActionEditIdx = 0;

	//그 상태의 이펙트 목록에서 지금 편집 중인 항목 인덱스. (여러 개를 다 펼치면 길어진다)
	int mFxEditIdx = 0;

	//버프(지속형) 이펙트 목록에서 지금 편집 중인 항목 인덱스.
	int mBuffEditIdx = 0;

	void UpdateAllRowWidths(float NewWidth);

	// 대상 액터의 애니메이션 타입을 라벨(mAnimTypeText)에 반영한다. (▼/▲ 포함)
	void RefreshAnimTypeLabel();

	// 타입 콤보가 열려 있을 때 항목 버튼들을 오버레이로 만든다.
	// (RebuildComponents 끝에서 호출 — 고정 위치, 비스크롤, 높은 ZOrder)
	void BuildAnimTypeMenu();

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
	std::weak_ptr<class CButton> AddDeleteButton(float Y, float PanelW, int WidgetIdx);

	// 삭제 버튼 클릭을 처리한다. 실제로 제거했으면 true (호출부는 즉시 Rebuild 후 반환할 것)
	bool HandleDeleteButtons();

	// ── 값 직접 입력 ────────────────────────────────────────────────────────
	void RegisterEditKeys();        // 숫자/기호 키를 인풋에 등록 (최초 1회)
	void HandleValueEditInput();    // 편집 중 키 입력 처리
	void DetectValueDoubleClick();  // 값 영역 더블클릭 감지 → 편집 시작
	void BeginEdit(int CompIdx, int PropIdx);
	void CommitEdit();              // Enter — 버퍼를 파싱해 Setter 호출
	void CancelEdit();              // Esc / 바깥 클릭 / Rebuild

	// ── 액터 이름 직접 입력 ─────────────────────────────────────────────────
	void DetectNameDoubleClick();   // 이름 행 더블클릭 감지 → 편집 시작
	void BeginNameEdit();           // 현재 이름을 버퍼에 담고 입력 모드 진입
	void HandleNameEditInput();     // 편집 중 키 입력 처리 + 라벨 미리보기
	void CommitNameEdit();          // Enter — 월드에 이름 변경 요청
	void CancelNameEdit();          // Esc / 바깥 클릭
	void AddPropRow(float& Y,
	                const wchar_t* Label,
	                float Step,
	                std::function<float()>     Getter,
	                std::function<void(float)> Setter,
	                FInspCompEntry& Entry);

	void AddActionRow(float& Y,
	                  const wchar_t* Label,
	                  const wchar_t* ButtonText,
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
