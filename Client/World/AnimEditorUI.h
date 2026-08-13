#pragma once

#include "World/WidgetContainer.h"
#include "World/TextBlock.h"
#include <functional>
#include <vector>
#include <string>

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
    };

    struct FSeqData
    {
        std::string Name;
        float PlayTime = 1.f;
        float PlayRate = 1.f;
        bool  Loop     = true;
        bool  Reverse  = false;
        bool  Symmetry = false;
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
        float Step;
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
    void SyncFrames(int ci, int si);
    void ApplyFrames(int ci, int si);
    void CreateNewAnim();
    void SetAnimTexture(int ci, int si);
    void SaveAnim(int ci, int si);
    void LoadAnim();
    void UpdateHandles(float nw, float nh);
    void OpenSpriteViewer(int ci, int si); // SpriteViewerUI 열고 콜백 연결
    void SyncSpriteViewer(int ci, int si); // 현재 프레임 데이터를 SpriteViewer에 동기화

    static void LoadAnimFromFile(const std::string& Path); // 파싱 + CAnimation2D 생성

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
        std::function<float()> Get, std::function<void(float)> Set);
};
