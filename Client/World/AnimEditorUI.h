#pragma once

#include "World/WidgetContainer.h"
#include "World/TextBlock.h"
#include <functional>
#include <vector>
#include <string>
#include <map>

// CreateAnimation() 호출 시 같이 Register()하면 에디터 "Add" 목록에 표시됨
struct CAnimRegistry
{
    inline static std::vector<std::string> sNames;
    static void Register(const std::string& Name)
    {
        for (auto& Registered : sNames) if (Registered == Name) return;
        sNames.push_back(Name);
    }
    static const std::vector<std::string>& GetAll() { return sNames; }

    // 애니메이션 이름 → 피벗 기준선 (텍셀).
    // 피벗은 CAnimation2D가 들고 있지 않고 에디터 전용 값이라, .anim2d에서 읽은 값을
    // 여기 보관했다가 SyncFrames가 시퀀스로 다시 꺼내간다.
    inline static std::map<std::string, FVector2> sPivots;

    static void SetPivot(const std::string& Name, float PivotX, float PivotY)
    {
        sPivots[Name] = FVector2(PivotX, PivotY);
    }

    static FVector2 GetPivot(const std::string& Name)
    {
        auto Found = sPivots.find(Name);
        return (Found != sPivots.end()) ? Found->second : FVector2(0.f, 0.f);
    }

    // 애니메이션 이름 → 그 이름을 등록한 .anim2d 경로.
    //
    // 애니메이션은 파일 이름이 아니라 파일 안의 AnimName으로 구분된다.
    // 그래서 백업하려고 파일만 복사해두면(예: PlayerBasicAttack3 → ...original)
    // 두 파일이 같은 이름을 주장하게 되고, 폴더를 통째로 읽을 때 뒤에 읽힌 쪽이
    // 앞의 것을 덮어써버린다. 저장은 됐는데 다시 켜면 옛날 데이터가 나오는 원인이다.
    // 어느 파일이 그 이름을 먼저 가져갔는지 기억해두고 충돌을 잡아낸다.
    inline static std::map<std::string, std::string> sSources;

    static const std::string& GetSourceFile(const std::string& Name)
    {
        static const std::string Empty;
        auto Found = sSources.find(Name);
        return (Found != sSources.end()) ? Found->second : Empty;
    }

    static void SetSourceFile(const std::string& Name, const std::string& Path)
    {
        sSources[Name] = Path;
    }

    // 애니메이션 이름 → 타입(= Asset\Anim\ 아래 하위 폴더 이름).
    // 최상위 폴더에서 읽은 애니는 타입이 ""(공용)다.
    // 액터가 고른 타입으로 "+ 애니메이션 추가" 목록을 좁힐 때 쓴다.
    inline static std::map<std::string, std::string> sTypes;

    static void SetType(const std::string& Name, const std::string& Type)
    {
        sTypes[Name] = Type;
    }

    static std::string GetType(const std::string& Name)
    {
        auto Found = sTypes.find(Name);
        return (Found != sTypes.end()) ? Found->second : std::string();
    }

    // 특정 타입에 속한 애니메이션 이름만 등록 순서대로 돌려준다.
    static std::vector<std::string> GetByType(const std::string& Type)
    {
        std::vector<std::string> Out;
        for (const auto& Name : sNames)
            if (GetType(Name) == Type)
                Out.push_back(Name);
        return Out;
    }

    // Asset\Anim\ 아래의 하위 폴더 이름을 훑어 타입 목록을 만든다. (정렬됨)
    // 파일이 아니라 폴더 구조를 직접 읽으므로, 폴더만 만들어두면 목록에 잡힌다.
    // 구현은 AnimEditorUI.cpp에 있다. (디렉터리 순회 때문에 <windows.h> 필요)
    static std::vector<std::string> ScanTypes();
};

// .anim2d 한 개를 읽었을 때 나오는 시퀀스 설정.
// 불러오기에서 컴포넌트에 붙일 때 파일에 저장된 값을 그대로 쓰기 위해 돌려받는다.
struct FLoadedAnimInfo
{
    std::string Name;
    float PlayTime = 1.f;
    float PlayRate = 1.f;
    bool  Loop     = true;
    bool  Reverse  = false;
    bool  Symmetry = false;
    float PivotX   = 0.f;
    float PivotY   = 0.f;
    int   FrameCount = 0;
};

class CAnimEditorUI : public CWidgetContainer
{
public:
    CAnimEditorUI();
    CAnimEditorUI(const CAnimEditorUI& src);
    virtual ~CAnimEditorUI();

private:
    static constexpr float PANEL_W   = 270.f;
    static constexpr float PANEL_H   = 580.f;
    static constexpr float PANEL_X   = 210.f;
    static constexpr float PANEL_Y   = 40.f;
    static constexpr float TITLE_H   = 28.f;
    static constexpr float ROW_H     = 22.f;
    static constexpr float HANDLE_SZ = 10.f;

    int mStaticChildCount = 0;
    int mActiveCorner     = -1;
    int mDynIdx           = 0;

    // Init 시점의 정적 위젯 목록을 강한 참조로 보존 (Rebuild 복원용)
    std::vector<std::shared_ptr<class CWidget>> mStaticWidgets;

    std::weak_ptr<class CButton>    mBackground;
    std::weak_ptr<class CTitleBar>  mTitleBarWidget;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CButton>    mHandleTL, mHandleTR, mHandleBL, mHandleBR;

    // ── 데이터 구조 ──────────────────────────────────────────────────────────
    struct FFrameData
    {
        FVector2 Start  = { 0.f, 0.f };
        FVector2 Size   = { 32.f, 32.f };
        FVector2 Offset = { 0.f, 0.f };
        // 이 프레임이 머무는 시간(초). 시퀀스 총 재생 시간은 이 값들의 합이다.
        float    Duration = 0.1f;
    };

    struct FSeqData
    {
        std::string Name;
        float PlayTime = 1.f;
        float PlayRate = 1.f;
        bool  Loop     = true;
        bool  Reverse  = false;
        bool  Symmetry = false;
        // 피벗 기준선 (텍셀, 선택 프레임의 Start 기준 상대 좌표)
        // 스프라이트 뷰어의 청록 세로선 / 자홍 가로선이 이 값이다.
        // 시퀀스마다 따로 가지며 .anim2d에 같이 저장된다.
        float PivotX = 0.f;
        float PivotY = 0.f;

        // 프레임 데이터 (CAnimation2D 미러)
        std::vector<FFrameData>   Frames;
        int                       SelectedFrame = -1;
        EAnimation2DTextureType   TextureType   = EAnimation2DTextureType::SpriteSheet;
        std::string               TextureName;
        std::string               TextureRelPath; // exe 기준 상대경로 (재로드용)
    };

    struct FCompData
    {
        std::weak_ptr<class CAnimation2DComponent> Comp;
        std::string CompName;
        std::vector<FSeqData> Seqs;
        int Selected = -1;
    };

    std::weak_ptr<class CActor> mTarget;
    std::vector<FCompData>      mComps;
    int                         mActiveComp      = 0;
    bool                        mShowRegistry    = false;
    bool                        mShowFrameEditor = false;

    // ── 동적 위젯 핸들 ───────────────────────────────────────────────────────
    struct FTabButton   { std::weak_ptr<CButton> Button; int Idx; };
    struct FSeqButton   { std::weak_ptr<CButton> Button; std::weak_ptr<CButton> DeleteButton; int Idx; };
    struct FPropButton
    {
        std::weak_ptr<CButton>    Minus, Plus;
        std::weak_ptr<CTextBlock> Label;
        std::weak_ptr<CButton>    ValButton;   // 값 영역 — 더블클릭하면 직접 입력
        float Step;
        int   Decimals = 1;                // 표시 소수 자릿수 (재생 시간은 0.01초 단위라 3이 필요)
        std::function<float()>     Getter;
        std::function<void(float)> Setter;

        // ── "All" (프레임 속성 행에만 있다) ──
        // 켜두면 선택한 프레임 하나가 아니라 시퀀스의 모든 프레임에 적용된다.
        //   +/- 버튼  → 모든 프레임을 Step만큼 밀어준다 (bDelta = true)
        //   직접 입력 → 모든 프레임을 그 값으로 맞춘다  (bDelta = false)
        std::function<void(float, bool)> SetAll;
        std::weak_ptr<CButton>    AllButton;
        std::weak_ptr<CTextBlock> AllLabel;
        int AllIdx = -1;                   // mFramePropAll 인덱스 (-1이면 All 버튼이 없는 행)
    };
    struct FToggleButton
    {
        std::weak_ptr<CButton>    Button;
        std::weak_ptr<CTextBlock> Label;
        std::function<bool()>     Getter;
        std::function<void(bool)> Setter;
    };
    struct FRegButton { std::weak_ptr<CButton> Button; std::string Name; };

    std::vector<FTabButton>    mTabButtons;
    std::vector<FSeqButton>    mSeqButtons;
    std::vector<FPropButton>   mPropButtons;
    std::vector<FToggleButton> mToggleButtons;
    std::vector<FRegButton>    mRegButtons;
    std::vector<FPropButton>   mFramePropButtons;  // Start.X/Y, Size.W/H, Off.X/Y, Dur

    // 프레임 속성 행마다 "All"이 켜져 있는지.
    // 위젯은 Rebuild 때마다 새로 만들어지므로 상태는 여기 남겨둔다.
    static constexpr int FRAME_PROP_MAX = 12;   // Start/Size/Offset/Dur(7) + Pivot.X/Y(2) + 여유
    bool mFramePropAll[FRAME_PROP_MAX] = {};

    std::weak_ptr<CButton>    mAddToggleButton;
    std::weak_ptr<CTextBlock> mFrameCountText;
    std::weak_ptr<CTextBlock> mPlayTimeText;   // 프레임 Duration 합계 표시 (읽기 전용)

    // 프레임 에디터 위젯
    std::weak_ptr<CButton>    mToggleFrameButton;
    std::weak_ptr<CButton>    mFramePrevButton, mFrameNextButton;
    std::weak_ptr<CTextBlock> mFrameIdxLabel;
    std::weak_ptr<CButton>    mAddFrameButton, mDelFrameButton, mClearFramesButton;
    std::weak_ptr<CButton>    mAlignFeetButton;   // 전 프레임 발밑(하단중앙) 정렬

    // 텍스처 / 타입
    std::weak_ptr<CButton>    mSetTextureButton;
    std::weak_ptr<CButton>    mTypeToggleButton;

    // 저장 / 불러오기 / 새 애니메이션
    std::weak_ptr<CButton>    mSaveAnimButton, mLoadAnimButton;
    std::weak_ptr<CButton>    mNewAnimButton;

    // SpriteViewer 연동
    std::weak_ptr<class CSpriteViewerUI> mSpriteViewer;
    std::weak_ptr<CButton>               mOpenViewerButton;

    // ── 값 직접 입력 (더블클릭 → 타이핑 → Enter) ────────────────────────────
    static constexpr float DOUBLE_CLICK_SEC = 0.35f;  // 더블클릭 인정 간격
    static constexpr int   EDIT_BUF_MAX     = 15;     // 입력 최대 길이

    bool  mKeysRegistered = false;   // 숫자 키 바인딩 등록 여부 (최초 Update에서 1회)
    float mTimeAccum      = 0.f;     // 더블클릭 판정용 누적 시간
    void* mLastClickKey   = nullptr; // 직전에 클릭된 값 버튼
    float mLastClickTime  = -10.f;

    bool        mEditActive = false;
    int         mEditList   = -1;    // 0 = mPropButtons, 1 = mFramePropButtons
    int         mEditIdx    = -1;    // 해당 목록 내 인덱스
    std::string mEditBuffer;         // 타이핑 중인 문자열

public:
    void SetTarget(std::weak_ptr<class CActor> Actor);
    void RefreshTarget() { SetTarget(mTarget); }
    void Rebuild();
    void SetSpriteViewer(std::weak_ptr<class CSpriteViewerUI> SpriteViewer) { mSpriteViewer = SpriteViewer; }

    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual CAnimEditorUI* Clone();

private:
    void AddSeq(int CompIdx, const std::string& AnimName);
    void PlaySeq(int CompIdx, int SeqIdx);
    void SyncFrames(int CompIdx, int SeqIdx);
    void ApplyFrames(int CompIdx, int SeqIdx);
    void CreateNewAnim();
    void RemoveSeq(int CompIdx, int SeqIdx);   // 시퀀스를 컴포넌트와 목록에서 제거
    void SetAnimTexture(int CompIdx, int SeqIdx);
    void SaveAnim(int CompIdx, int SeqIdx);
    void LoadAnim();

    // 소스 애니메이션(SrcName)의 내용을 대상 시퀀스(CompIdx, DstSeqIdx)에
    // 이름만 빼고 통째로 복사한다. 프레임/텍스처/타입/재생설정/피벗까지.
    // FSeqData와 대상 CAnimation2D 에셋 양쪽에 반영한다.
    void CopyAnimData(int CompIdx, int DstSeqIdx, const std::string& SrcName);
    void UpdateHandles(float NewW, float NewH);

    //Rebuild 마무리 — 동적 위젯을 스크롤 대상으로 표시하고 콘텐츠 길이를 알려준다.
    void FinishLayout(float ContentEndY);
    void OpenSpriteViewer(int CompIdx, int SeqIdx); // SpriteViewerUI 열고 콜백 연결
    void SyncSpriteViewer(int CompIdx, int SeqIdx); // 현재 프레임 데이터를 SpriteViewer에 동기화

    // 뷰어가 열려 있으면 지금 선택된 시퀀스로 다시 붙여준다.
    // 시퀀스가 바뀌거나 텍스처를 갈아끼운 뒤에 부른다.
    void RefreshSpriteViewer();

    // ── 값 직접 입력 ────────────────────────────────────────────────────────
    void       RegisterEditKeys();       // 숫자/기호 키를 인풋에 등록 (최초 1회)
    void       HandleValueEditInput();   // 편집 중 키 입력 처리
    void       DetectValueDoubleClick(); // 값 영역 더블클릭 감지 → 편집 시작
    void       BeginEdit(int ListIdx, int RowIdx);
    void       CommitEdit();             // Enter — 버퍼를 파싱해 Setter 호출
    void       CancelEdit();             // Esc / 바깥 클릭 / Rebuild
    void       RefreshEditLabel();       // 편집 중인 행에 버퍼 + 캐럿 표시
    FPropButton*  GetEditProp();            // 편집 대상 행 (없으면 nullptr)

    // 파싱 + CAnimation2D 생성. OutInfo를 주면 읽어낸 시퀀스 설정을 채워준다.
    //
    // bSkipDuplicateName: 그 AnimName을 이미 다른 파일이 가져갔으면 읽지 않고 물러난다.
    //   폴더를 통째로 훑는 LoadAllAnims가 켜고 부른다. 사용자가 직접 고른
    //   "Load Anim"은 덮어쓰는 게 의도이므로 끈 채로 부른다.
    //   Type: 이 파일이 속한 타입(하위 폴더 이름). 레지스트리에 그대로 기록한다.
    //         최상위 폴더에서 읽었으면 ""(공용)다.
    static bool LoadAnimFromFile(const std::string& Path, FLoadedAnimInfo* OutInfo = nullptr,
                                 bool bSkipDuplicateName = false,
                                 const std::string& Type = "");

    // AnimRoot\<Type>\*.anim2d 를 Type으로 등록한다. Type이 ""면 최상위 폴더.
    // 읽어 등록한 개수를 돌려준다. (LoadAllAnims가 타입별로 불러 모은다)
    static int LoadAnimsInFolder(const std::string& AnimRoot, const std::string& Type);

public:
    static void LoadAllAnims(); // Asset\Anim\ 폴더 전체(하위 타입 폴더 포함) 자동 로드

    std::weak_ptr<CButton>    MakeButton(const std::string& Name,
        float X, float Y, float Width, float Height, float Red, float Green, float Blue);
    std::weak_ptr<CTextBlock> MakeLabel(const std::string& Name,
        float X, float Y, float Width, float Height, const wchar_t* Text,
        float FontSize = 12.f, ETextAlignH AlignH = ETextAlignH::Left);

    void AddPropRow(float& Y, const wchar_t* Label, float Step,
        std::function<float()> Getter, std::function<void(float)> Setter);
    void AddToggleRow(float& Y, const wchar_t* Label,
        std::function<bool()> Getter, std::function<void(bool)> Setter);
    void AddFramePropRow(float& Y, const wchar_t* Label, float Step,
        std::function<float()> Getter, std::function<void(float)> Setter,
        std::function<void(float, bool)> SetAll = nullptr, int Decimals = 1);

    // 프레임의 실수 값 하나를 편집하는 행.
    // Field는 프레임에서 그 값을 참조로 꺼내주는 함수다.
    //   예) [](FFrameData& F) -> float& { return F.Size.x; }
    // MinValue 아래로는 내려가지 않는다. 제한이 필요 없으면 아주 작은 값을 넘긴다.
    void AddFrameFieldRow(float& Y, const wchar_t* Label, float Step,
        std::function<float&(FFrameData&)> Field, float MinValue, int Decimals = 1);
};
