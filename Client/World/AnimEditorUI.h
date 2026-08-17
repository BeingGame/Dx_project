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
        for (auto& N : sNames) if (N == Name) return;
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
        auto It = sPivots.find(Name);
        return (It != sPivots.end()) ? It->second : FVector2(0.f, 0.f);
    }
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
    struct FTabBtn   { std::weak_ptr<CButton> Btn; int Idx; };
    struct FSeqBtn   { std::weak_ptr<CButton> Btn; int Idx; };
    struct FPropBtn
    {
        std::weak_ptr<CButton>    Minus, Plus;
        std::weak_ptr<CTextBlock> Lbl;
        std::weak_ptr<CButton>    ValBtn;   // 값 영역 — 더블클릭하면 직접 입력
        float Step;
        int   Decimals = 1;                // 표시 소수 자릿수 (재생 시간은 0.01초 단위라 3이 필요)
        std::function<float()>     Get;
        std::function<void(float)> Set;
    };
    struct FToggleBtn
    {
        std::weak_ptr<CButton>    Btn;
        std::weak_ptr<CTextBlock> Lbl;
        std::function<bool()>     Get;
        std::function<void(bool)> Set;
    };
    struct FRegBtn { std::weak_ptr<CButton> Btn; std::string Name; };

    std::vector<FTabBtn>    mTabBtns;
    std::vector<FSeqBtn>    mSeqBtns;
    std::vector<FPropBtn>   mPropBtns;
    std::vector<FToggleBtn> mToggleBtns;
    std::vector<FRegBtn>    mRegBtns;
    std::vector<FPropBtn>   mFramePropBtns;  // Start.X/Y, Size.W/H, Off.X/Y

    std::weak_ptr<CButton>    mAddToggleBtn;
    std::weak_ptr<CTextBlock> mFrameCountText;
    std::weak_ptr<CTextBlock> mPlayTimeText;   // 프레임 Duration 합계 표시 (읽기 전용)

    // 프레임 에디터 위젯
    std::weak_ptr<CButton>    mToggleFrameBtn;
    std::weak_ptr<CButton>    mFramePrevBtn, mFrameNextBtn;
    std::weak_ptr<CTextBlock> mFrameIdxLbl;
    std::weak_ptr<CButton>    mAddFrameBtn, mDelFrameBtn, mClearFramesBtn;

    // 텍스처 / 타입
    std::weak_ptr<CButton>    mSetTextureBtn;
    std::weak_ptr<CButton>    mTypeToggleBtn;

    // 저장 / 불러오기 / 새 애니메이션
    std::weak_ptr<CButton>    mSaveAnimBtn, mLoadAnimBtn;
    std::weak_ptr<CButton>    mNewAnimBtn;

    // SpriteViewer 연동
    std::weak_ptr<class CSpriteViewerUI> mSpriteViewer;
    std::weak_ptr<CButton>               mOpenViewerBtn;

    // ── 값 직접 입력 (더블클릭 → 타이핑 → Enter) ────────────────────────────
    static constexpr float DOUBLE_CLICK_SEC = 0.35f;  // 더블클릭 인정 간격
    static constexpr int   EDIT_BUF_MAX     = 15;     // 입력 최대 길이

    bool  mKeysRegistered = false;   // 숫자 키 바인딩 등록 여부 (최초 Update에서 1회)
    float mTimeAccum      = 0.f;     // 더블클릭 판정용 누적 시간
    void* mLastClickKey   = nullptr; // 직전에 클릭된 값 버튼
    float mLastClickTime  = -10.f;

    bool        mEditActive = false;
    int         mEditList   = -1;    // 0 = mPropBtns, 1 = mFramePropBtns
    int         mEditIdx    = -1;    // 해당 목록 내 인덱스
    std::string mEditBuffer;         // 타이핑 중인 문자열

public:
    void SetTarget(std::weak_ptr<class CActor> Actor);
    void RefreshTarget() { SetTarget(mTarget); }
    void Rebuild();
    void SetSpriteViewer(std::weak_ptr<class CSpriteViewerUI> SV) { mSpriteViewer = SV; }

    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual CAnimEditorUI* Clone();

private:
    void AddSeq(int CompIdx, const std::string& AnimName);
    void PlaySeq(int CompIdx, int SeqIdx);
    void SyncFrames(int CompIdx, int SeqIdx);
    void ApplyFrames(int CompIdx, int SeqIdx);
    void CreateNewAnim();
    void SetAnimTexture(int CompIdx, int SeqIdx);
    void SaveAnim(int CompIdx, int SeqIdx);
    void LoadAnim();
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
    void       CommitEdit();             // Enter — 버퍼를 파싱해 Set 호출
    void       CancelEdit();             // Esc / 바깥 클릭 / Rebuild
    void       RefreshEditLabel();       // 편집 중인 행에 버퍼 + 캐럿 표시
    FPropBtn*  GetEditProp();            // 편집 대상 행 (없으면 nullptr)

    // 파싱 + CAnimation2D 생성. OutInfo를 주면 읽어낸 시퀀스 설정을 채워준다.
    static bool LoadAnimFromFile(const std::string& Path, FLoadedAnimInfo* OutInfo = nullptr);

public:
    static void LoadAllAnims(); // Asset\Anim\ 폴더 전체 자동 로드

    std::weak_ptr<CButton>    MakeBtn(const std::string& Name,
        float X, float Y, float W, float H, float R, float G, float B);
    std::weak_ptr<CTextBlock> MakeLbl(const std::string& Name,
        float X, float Y, float W, float H, const wchar_t* Text,
        float FontSize = 12.f, ETextAlignH AlignH = ETextAlignH::Left);

    void AddPropRow(float& Y, const wchar_t* Label, float Step,
        std::function<float()> Get, std::function<void(float)> Set);
    void AddToggleRow(float& Y, const wchar_t* Label,
        std::function<bool()> Get, std::function<void(bool)> Set);
    void AddFramePropRow(float& Y, const wchar_t* Label, float Step,
        std::function<float()> Get, std::function<void(float)> Set, int Decimals = 1);
};
