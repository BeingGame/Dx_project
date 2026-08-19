#include "SpriteViewerUI.h"

#include "World/Button.h"
#include "World/TextBlock.h"
#include "World/World.h"
#include "World/Input.h"

#include <algorithm>
#include <string>

using namespace std;


CSpriteViewerUI::CSpriteViewerUI()  {}
CSpriteViewerUI::CSpriteViewerUI(const CSpriteViewerUI& src) : CWidgetContainer(src) {}
CSpriteViewerUI::~CSpriteViewerUI() {}

// ── 위젯 헬퍼 ────────────────────────────────────────────────────────────────

std::weak_ptr<CButton> CSpriteViewerUI::MakeButton(const std::string& Name,
    float X, float Y, float Width, float Height,
    float Red, float Green, float Blue, float Alpha, int ZOrder)
{
    auto Button = CreateWidget<CButton>(Name, ZOrder).lock();
    if (Button)
    {
        Button->SetPos(X, Y);
        Button->SetSize(Width, Height);
        float HoverRed = min(Red + .12f, 1.f), HoverGreen = min(Green + .12f, 1.f), HoverBlue = min(Blue + .12f, 1.f);
        float ClickRed = min(Red + .22f, 1.f), ClickGreen = min(Green + .22f, 1.f), ClickBlue = min(Blue + .22f, 1.f);
        Button->SetTint(EWidgetState::Normal,  Red,  Green,  Blue,  Alpha);
        Button->SetTint(EWidgetState::Hovered, HoverRed, HoverGreen, HoverBlue, Alpha);
        Button->SetTint(EWidgetState::Clicked, ClickRed, ClickGreen, ClickBlue, Alpha);
        Button->SetTint(EWidgetState::Release, HoverRed, HoverGreen, HoverBlue, Alpha);
        Button->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.15f, Alpha * 0.5f);
    }
    return Button;
}

std::weak_ptr<CTextBlock> CSpriteViewerUI::MakeLabel(const std::string& Name,
    float X, float Y, float Width, float Height, const wchar_t* Text,
    float FontSize, ETextAlignH AlignH, int ZOrder)
{
    auto Label = CreateWidget<CTextBlock>(Name, ZOrder).lock();
    if (Label)
    {
        Label->SetPos(X, Y);
        Label->SetSize(Width, Height);
        Label->SetText(Text);
        Label->SetFontSize(FontSize);
        Label->SetTextColor(FVector4(0.88f, 0.88f, 0.92f, 1.f));
        Label->SetAlignH(AlignH);
        Label->SetAlignV(ETextAlignV::Middle);
    }
    return Label;
}


bool CSpriteViewerUI::Init()
{
    CWidgetContainer::Init();

    SetPos(PANEL_X, PANEL_Y);
    SetSize(PANEL_W, PANEL_H);

    // ── 배경 ──────────────────────────────────────────────────────────────────
    auto Background = CreateWidget<CButton>("SVBg", 0).lock();
    if (Background)
    {
        Background->SetPos(0.f, 0.f);
        Background->SetSize(PANEL_W, PANEL_H);
        Background->SetTint(EWidgetState::Normal,  0.10f, 0.10f, 0.14f, 0.97f);
        Background->SetTint(EWidgetState::Hovered, 0.10f, 0.10f, 0.14f, 0.97f);
        Background->SetTint(EWidgetState::Clicked, 0.10f, 0.10f, 0.14f, 0.97f);
        Background->SetTint(EWidgetState::Release, 0.10f, 0.10f, 0.14f, 0.97f);
        Background->SetTint(EWidgetState::Disable, 0.10f, 0.10f, 0.14f, 0.97f);
        mBgButton = Background;
    }

    // ── 타이틀 바 ──────────────────────────────────────────────────────────────
    auto TitleBg = CreateWidget<CButton>("SVTitle", 10).lock();
    if (TitleBg)
    {
        TitleBg->SetPos(0.f, 0.f);
        TitleBg->SetSize(PANEL_W, TITLE_H);
        TitleBg->SetTint(EWidgetState::Normal,  0.15f, 0.15f, 0.30f, 1.f);
        TitleBg->SetTint(EWidgetState::Hovered, 0.20f, 0.20f, 0.35f, 1.f);
        TitleBg->SetTint(EWidgetState::Clicked, 0.20f, 0.20f, 0.35f, 1.f);
        TitleBg->SetTint(EWidgetState::Release, 0.15f, 0.15f, 0.30f, 1.f);
        TitleBg->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.30f, 1.f);
    }
    MakeLabel("SVTitleLbl", 6.f, 0.f, PANEL_W - 40.f, TITLE_H,
        TEXT("Sprite Viewer  (drag: move / corners: resize / Arrow: move / Space: dup / Ctrl+D: del)"), 11.f);

    // 닫기 버튼
    {
        auto CloseButton = CreateWidget<CButton>("SVClose", 10).lock();
        if (CloseButton)
        {
            CloseButton->SetPos(PANEL_W - 28.f, 2.f);
            CloseButton->SetSize(24.f, TITLE_H - 4.f);
            CloseButton->SetTint(EWidgetState::Normal,  0.55f, 0.15f, 0.15f, 1.f);
            CloseButton->SetTint(EWidgetState::Hovered, 0.80f, 0.20f, 0.20f, 1.f);
            CloseButton->SetTint(EWidgetState::Clicked, 0.90f, 0.25f, 0.25f, 1.f);
            CloseButton->SetTint(EWidgetState::Release, 0.55f, 0.15f, 0.15f, 1.f);
            CloseButton->SetTint(EWidgetState::Disable, 0.30f, 0.10f, 0.10f, 1.f);
            mCloseButton = CloseButton;
        }
        MakeLabel("SVCloseL", PANEL_W - 28.f, 2.f, 24.f, TITLE_H - 4.f,
            TEXT("X"), 13.f, ETextAlignH::Center);
    }

    // ── 줌 컨트롤 바 ──────────────────────────────────────────────────────────
    {
        float CtrlY = TITLE_H + 3.f;   // 줌 컨트롤 바의 세로 위치
        float CtrlX = 6.f;             // 왼쪽부터 버튼을 쌓아가는 가로 커서

        auto ZOut = CreateWidget<CButton>("SVZOut", 10).lock();
        if (ZOut) {
            ZOut->SetPos(CtrlX, CtrlY);
            ZOut->SetSize(24.f, 20.f);
            ZOut->SetTint(EWidgetState::Normal,  0.22f, 0.22f, 0.28f, 1.f);
            ZOut->SetTint(EWidgetState::Hovered, 0.32f, 0.32f, 0.40f, 1.f);
            ZOut->SetTint(EWidgetState::Clicked, 0.42f, 0.42f, 0.52f, 1.f);
            ZOut->SetTint(EWidgetState::Release, 0.22f, 0.22f, 0.28f, 1.f);
            ZOut->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.18f, 1.f);
            mZoomOutButton = ZOut;
        }
        MakeLabel("SVZOutL", CtrlX, CtrlY, 24.f, 20.f, TEXT("-"), 12.f, ETextAlignH::Center);
        CtrlX += 26.f;

        auto ZLabel = CreateWidget<CTextBlock>("SVZLbl", 11).lock();
        if (ZLabel) {
            ZLabel->SetPos(CtrlX, CtrlY);
            ZLabel->SetSize(56.f, 20.f);
            ZLabel->SetText(TEXT("100%"));
            ZLabel->SetFontSize(11.f);
            ZLabel->SetTextColor(FVector4(0.9f, 0.9f, 0.9f, 1.f));
            ZLabel->SetAlignH(ETextAlignH::Center);
            ZLabel->SetAlignV(ETextAlignV::Middle);
            mZoomLabel = ZLabel;
        }
        CtrlX += 58.f;

        auto ZoomInButton = CreateWidget<CButton>("SVZIn", 10).lock();
        if (ZoomInButton) {
            ZoomInButton->SetPos(CtrlX, CtrlY);
            ZoomInButton->SetSize(24.f, 20.f);
            ZoomInButton->SetTint(EWidgetState::Normal,  0.22f, 0.22f, 0.28f, 1.f);
            ZoomInButton->SetTint(EWidgetState::Hovered, 0.32f, 0.32f, 0.40f, 1.f);
            ZoomInButton->SetTint(EWidgetState::Clicked, 0.42f, 0.42f, 0.52f, 1.f);
            ZoomInButton->SetTint(EWidgetState::Release, 0.22f, 0.22f, 0.28f, 1.f);
            ZoomInButton->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.18f, 1.f);
            mZoomInButton = ZoomInButton;
        }
        MakeLabel("SVZInL", CtrlX, CtrlY, 24.f, 20.f, TEXT("+"), 12.f, ETextAlignH::Center);
        CtrlX += 26.f;

        auto ZFit = CreateWidget<CButton>("SVZFit", 10).lock();
        if (ZFit) {
            ZFit->SetPos(CtrlX, CtrlY);
            ZFit->SetSize(40.f, 20.f);
            ZFit->SetTint(EWidgetState::Normal,  0.15f, 0.25f, 0.42f, 1.f);
            ZFit->SetTint(EWidgetState::Hovered, 0.22f, 0.35f, 0.55f, 1.f);
            ZFit->SetTint(EWidgetState::Clicked, 0.28f, 0.42f, 0.65f, 1.f);
            ZFit->SetTint(EWidgetState::Release, 0.15f, 0.25f, 0.42f, 1.f);
            ZFit->SetTint(EWidgetState::Disable, 0.10f, 0.15f, 0.25f, 1.f);
            mZoomFitButton = ZFit;
        }
        MakeLabel("SVZFitL", CtrlX, CtrlY, 40.f, 20.f, TEXT("Fit"), 10.f, ETextAlignH::Center);
        CtrlX += 48.f;

        // ── Slice LT 토글 (방향키로 프레임 Start 이동, slice=Size 단위 / 아니면 연속) ──
        auto MvButton = CreateWidget<CButton>("SVMvSlice", 10).lock();
        if (MvButton)
        {
            MvButton->SetPos(CtrlX, CtrlY);
            MvButton->SetSize(86.f, 20.f);
            MvButton->SetTint(EWidgetState::Normal,  0.18f, 0.18f, 0.22f, 1.f);
            MvButton->SetTint(EWidgetState::Hovered, 0.26f, 0.26f, 0.32f, 1.f);
            MvButton->SetTint(EWidgetState::Clicked, 0.32f, 0.32f, 0.40f, 1.f);
            MvButton->SetTint(EWidgetState::Release, 0.18f, 0.18f, 0.22f, 1.f);
            MvButton->SetTint(EWidgetState::Disable, 0.12f, 0.12f, 0.15f, 1.f);
            mMoveSliceLTButton = MvButton;
        }
        auto MvLabel = CreateWidget<CTextBlock>("SVMvSliceL", 11).lock();
        if (MvLabel)
        {
            MvLabel->SetPos(CtrlX, CtrlY);
            MvLabel->SetSize(86.f, 20.f);
            MvLabel->SetText(TEXT("[ ] Slice LT"));
            MvLabel->SetFontSize(9.f);
            MvLabel->SetTextColor(FVector4(0.75f, 0.75f, 0.80f, 1.f));
            MvLabel->SetAlignH(ETextAlignH::Center);
            MvLabel->SetAlignV(ETextAlignV::Middle);
            mMoveSliceLTLabel = MvLabel;
        }
        CtrlX += 88.f;

        // ── Focus 토글 (프레임을 선택하면 그 시작 지점으로 확대) ──
        auto FcButton = CreateWidget<CButton>("SVFocus", 10).lock();
        if (FcButton)
        {
            FcButton->SetPos(CtrlX, CtrlY);
            FcButton->SetSize(76.f, 20.f);
            FcButton->SetTint(EWidgetState::Disable, 0.12f, 0.12f, 0.15f, 1.f);
            mFocusButton = FcButton;
        }
        auto FcLabel = CreateWidget<CTextBlock>("SVFocusL", 11).lock();
        if (FcLabel)
        {
            FcLabel->SetPos(CtrlX, CtrlY);
            FcLabel->SetSize(76.f, 20.f);
            FcLabel->SetFontSize(9.f);
            FcLabel->SetAlignH(ETextAlignH::Center);
            FcLabel->SetAlignV(ETextAlignV::Middle);
            mFocusLabel = FcLabel;
        }
        UpdateFocusLabel();
    }

    // ── 캔버스 배경 ───────────────────────────────────────────────────────────
    auto CanvasBg = CreateWidget<CButton>("SVCanvas", 1).lock();
    if (CanvasBg)
    {
        CanvasBg->SetPos(0.f, CANVAS_Y);
        CanvasBg->SetSize(PANEL_W, CANVAS_H);
        CanvasBg->SetTint(EWidgetState::Normal,  0.05f, 0.05f, 0.07f, 1.f);
        CanvasBg->SetTint(EWidgetState::Hovered, 0.05f, 0.05f, 0.07f, 1.f);
        CanvasBg->SetTint(EWidgetState::Clicked, 0.05f, 0.05f, 0.07f, 1.f);
        CanvasBg->SetTint(EWidgetState::Release, 0.05f, 0.05f, 0.07f, 1.f);
        CanvasBg->SetTint(EWidgetState::Disable, 0.05f, 0.05f, 0.07f, 1.f);
    }

    // ── 텍스처 표시 버튼 (ShowTexture에서 크기/위치 업데이트) ─────────────────
    auto TexButton = CreateWidget<CButton>("SVTex", 2).lock();
    if (TexButton)
    {
        TexButton->SetPos(0.f, CANVAS_Y);
        TexButton->SetSize(0.f, 0.f);
        TexButton->SetTint(EWidgetState::Normal,  1.f, 1.f, 1.f, 1.f);
        TexButton->SetTint(EWidgetState::Hovered, 1.f, 1.f, 1.f, 1.f);
        TexButton->SetTint(EWidgetState::Clicked, 1.f, 1.f, 1.f, 1.f);
        TexButton->SetTint(EWidgetState::Release, 1.f, 1.f, 1.f, 1.f);
        TexButton->SetTint(EWidgetState::Disable, 1.f, 1.f, 1.f, 1.f);
        mTexButton = TexButton;
    }

    // ── 정보 바 ───────────────────────────────────────────────────────────────
    float InfoY = PANEL_H - INFO_H;  // 하단 정보 바의 세로 위치
    auto InfoBg = CreateWidget<CButton>("SVInfoBg", 10).lock();
    if (InfoBg)
    {
        InfoBg->SetPos(0.f, InfoY);
        InfoBg->SetSize(PANEL_W, INFO_H);
        InfoBg->SetTint(EWidgetState::Normal,  0.13f, 0.13f, 0.18f, 1.f);
        InfoBg->SetTint(EWidgetState::Hovered, 0.13f, 0.13f, 0.18f, 1.f);
        InfoBg->SetTint(EWidgetState::Clicked, 0.13f, 0.13f, 0.18f, 1.f);
        InfoBg->SetTint(EWidgetState::Release, 0.13f, 0.13f, 0.18f, 1.f);
        InfoBg->SetTint(EWidgetState::Disable, 0.13f, 0.13f, 0.18f, 1.f);
    }

    auto InfoLabel = CreateWidget<CTextBlock>("SVInfo", 11).lock();
    if (InfoLabel)
    {
        InfoLabel->SetPos(6.f, InfoY + 2.f);
        InfoLabel->SetSize(PANEL_W - 116.f, INFO_H - 4.f);
        InfoLabel->SetText(TEXT("텍스처를 설정한 뒤 Open Sprite Viewer를 클릭하세요."));
        InfoLabel->SetFontSize(10.f);
        InfoLabel->SetTextColor(FVector4(0.85f, 0.85f, 0.92f, 1.f));
        InfoLabel->SetAlignV(ETextAlignV::Middle);
        mInfoLabel = InfoLabel;
    }

    // [+ Add] 버튼 (커서 위치에 32x32 프레임 추가)
    float InfoButtonX = PANEL_W - 108.f; // 정보 바 우측 버튼들의 가로 커서
    auto AddButton = CreateWidget<CButton>("SVAdd", 10).lock();
    if (AddButton)
    {
        AddButton->SetPos(InfoButtonX, InfoY + 14.f);
        AddButton->SetSize(48.f, 22.f);
        AddButton->SetTint(EWidgetState::Normal,  0.14f, 0.34f, 0.14f, 1.f);
        AddButton->SetTint(EWidgetState::Hovered, 0.20f, 0.46f, 0.20f, 1.f);
        AddButton->SetTint(EWidgetState::Clicked, 0.26f, 0.56f, 0.26f, 1.f);
        AddButton->SetTint(EWidgetState::Release, 0.14f, 0.34f, 0.14f, 1.f);
        AddButton->SetTint(EWidgetState::Disable, 0.10f, 0.20f, 0.10f, 1.f);
        mAddButton = AddButton;
    }
    MakeLabel("SVAddL", InfoButtonX, InfoY + 14.f, 48.f, 22.f, TEXT("+ Add"), 10.f, ETextAlignH::Center);
    InfoButtonX += 52.f;

    // [- Del] 버튼 (선택된 프레임 삭제)
    auto DelButton = CreateWidget<CButton>("SVDel", 10).lock();
    if (DelButton)
    {
        DelButton->SetPos(InfoButtonX, InfoY + 14.f);
        DelButton->SetSize(48.f, 22.f);
        DelButton->SetTint(EWidgetState::Normal,  0.34f, 0.14f, 0.14f, 1.f);
        DelButton->SetTint(EWidgetState::Hovered, 0.46f, 0.20f, 0.20f, 1.f);
        DelButton->SetTint(EWidgetState::Clicked, 0.56f, 0.26f, 0.26f, 1.f);
        DelButton->SetTint(EWidgetState::Release, 0.34f, 0.14f, 0.14f, 1.f);
        DelButton->SetTint(EWidgetState::Disable, 0.20f, 0.10f, 0.10f, 1.f);
        mDelButton = DelButton;
    }
    MakeLabel("SVDelL", InfoButtonX, InfoY + 14.f, 48.f, 22.f, TEXT("- Del"), 10.f, ETextAlignH::Center);

    mStaticChildCount = (int)mChildList.size();
    return true;
}

// ── 공개 인터페이스 ───────────────────────────────────────────────────────────

void CSpriteViewerUI::ShowTexture(const std::string& TexName, FVector2 TexSize)
{
    mTexName = TexName;
    mTexSize.x = TexSize.x > 0.f ? TexSize.x : 1.f;
    mTexSize.y = TexSize.y > 0.f ? TexSize.y : 1.f;

    FitToCanvas();

    if (auto Button = mTexButton.lock())
    {
        Button->SetTexture(EWidgetState::Normal,  mTexName);
        Button->SetTexture(EWidgetState::Hovered, mTexName);
        Button->SetTexture(EWidgetState::Clicked, mTexName);
        Button->SetTexture(EWidgetState::Release, mTexName);
        Button->SetTexture(EWidgetState::Disable, mTexName);
    }

    RebuildFrameOverlays();
    UpdateInfoLabel();
}

void CSpriteViewerUI::SetFrames(const std::vector<FFrameRect>& Frames, int Selected)
{
    mFrames        = Frames;
    mSelectedFrame = (Selected >= 0 && Selected < (int)Frames.size()) ? Selected : -1;
    RebuildFrameOverlays();
    UpdateInfoLabel();
}

void CSpriteViewerUI::SelectFrame(int FrameIdx)
{
    mSelectedFrame = (FrameIdx >= 0 && FrameIdx < (int)mFrames.size()) ? FrameIdx : -1;

    // Focus가 켜져 있으면 새로 고른 프레임의 시작 지점으로 화면을 옮긴다.
    // (FocusFrame이 오버레이도 다시 만들어주므로 아래 RebuildFrameOverlays는 건너뛴다)
    if (mAutoFocus && mSelectedFrame >= 0)
    {
        FocusFrame(mSelectedFrame);
        UpdateInfoLabel();
        return;
    }

    RebuildFrameOverlays();
    UpdateInfoLabel();
}

// 선택한 프레임의 시작(좌상단)이 캔버스 왼쪽 위 근처에 오도록 확대해서 보여준다.
// Start 값을 다듬을 때 그 모서리를 크게 봐야 하므로, 프레임 전체를 꽉 채우는 대신
// 시작 지점을 기준으로 잡고 프레임이 캔버스의 절반쯤 되도록 배율을 정한다.
void CSpriteViewerUI::FocusFrame(int FrameIdx)
{
    if (FrameIdx < 0 || FrameIdx >= (int)mFrames.size())
    {
        return;
    }

    const FFrameRect& Frame = mFrames[FrameIdx];

    float FrameW = max(1.f, Frame.Size.x);
    float FrameH = max(1.f, Frame.Size.y);

    // 프레임이 캔버스의 절반 정도를 차지하는 배율. 주변도 같이 보여야 다루기 쉽다.
    float Target = min(PANEL_W * 0.5f / FrameW, CANVAS_H * 0.5f / FrameH);

    mScale = max(0.05f, min(Target, 12.f));

    // 시작 지점을 캔버스의 30% 지점에 놓는다.
    // 정중앙에 두면 프레임 몸통이 오른쪽 아래로 잘려나가기 때문이다.
    float AnchorX = PANEL_W * 0.30f;
    float AnchorY = CANVAS_Y + CANVAS_H * 0.30f;

    mTexLocalX = AnchorX - Frame.Start.x * mScale;
    mTexLocalY = AnchorY - Frame.Start.y * mScale;

    if (auto Button = mTexButton.lock())
    {
        Button->SetPos(mTexLocalX, mTexLocalY);
        Button->SetSize(mTexSize.x * mScale, mTexSize.y * mScale);
    }

    UpdateZoomLabel();
    RebuildFrameOverlays();
}

void CSpriteViewerUI::SetAutoFocus(bool bOn)
{
    mAutoFocus = bOn;
    UpdateFocusLabel();
}

void CSpriteViewerUI::UpdateFocusLabel()
{
    if (auto Label = mFocusLabel.lock())
    {
        Label->SetText(mAutoFocus ? TEXT("[v] Focus") : TEXT("[ ] Focus"));
        Label->SetTextColor(mAutoFocus ? FVector4(0.70f, 1.f, 0.75f, 1.f)
                                   : FVector4(0.75f, 0.75f, 0.80f, 1.f));
    }

    if (auto Button = mFocusButton.lock())
    {
        // 켜지면 초록, 꺼지면 회색 (Slice LT 토글과 같은 규칙)
        float Red = mAutoFocus ? 0.10f : 0.18f;
        float Green = mAutoFocus ? 0.38f : 0.18f;
        float Blue = mAutoFocus ? 0.10f : 0.22f;

        Button->SetTint(EWidgetState::Normal,  Red, Green, Blue, 1.f);
        Button->SetTint(EWidgetState::Hovered, Red + 0.10f, Green + 0.10f, Blue + 0.10f, 1.f);
        Button->SetTint(EWidgetState::Clicked, Red + 0.16f, Green + 0.16f, Blue + 0.16f, 1.f);
        Button->SetTint(EWidgetState::Release, Red + 0.10f, Green + 0.10f, Blue + 0.10f, 1.f);
    }
}

void CSpriteViewerUI::SetPivot(float PivotX, float PivotY)
{
    mPivotX = PivotX;
    mPivotY = PivotY;
    RebuildFrameOverlays();
    UpdateInfoLabel();
}

// ── 내부 계산 ─────────────────────────────────────────────────────────────────

void CSpriteViewerUI::FitToCanvas()
{
    float FitScaleX = PANEL_W / mTexSize.x;
    float FitScaleY = CANVAS_H / mTexSize.y;
    mScale = min(FitScaleX, FitScaleY);
    mScale = max(0.05f, mScale);

    // 화면에 그려질 이미지 크기 (텍스처 원본 크기 × 배율)
    float DrawW = mTexSize.x * mScale;
    float DrawH = mTexSize.y * mScale;
    mTexLocalX = (PANEL_W - DrawW) * 0.5f;
    mTexLocalY = CANVAS_Y + (CANVAS_H - DrawH) * 0.5f;

    if (auto Button = mTexButton.lock())
    {
        Button->SetPos(mTexLocalX, mTexLocalY);
        Button->SetSize(DrawW, DrawH);
    }
    UpdateZoomLabel();
}

void CSpriteViewerUI::ApplyZoom(float NewScale)
{
    mScale = max(0.05f, min(NewScale, 12.f));

    float DrawW = mTexSize.x * mScale;
    float DrawH = mTexSize.y * mScale;
    // 캔버스 중앙 기준 배치 (이미지가 캔버스보다 작을 때)
    mTexLocalX = max(0.f, (PANEL_W - DrawW) * 0.5f);
    mTexLocalY = CANVAS_Y + max(0.f, (CANVAS_H - DrawH) * 0.5f);

    if (auto Button = mTexButton.lock())
    {
        Button->SetPos(mTexLocalX, mTexLocalY);
        Button->SetSize(DrawW, DrawH);
    }
    UpdateZoomLabel();
    RebuildFrameOverlays();
}

void CSpriteViewerUI::UpdateZoomLabel()
{
    if (auto Label = mZoomLabel.lock())
    {
        TCHAR TextBuffer[32];
        swprintf_s(TextBuffer, 32, L"%.0f%%", mScale * 100.f);
        Label->SetText(TextBuffer);
    }
}

void CSpriteViewerUI::UpdateInfoLabel()
{
    if (auto Label = mInfoLabel.lock())
    {
        if (mFrames.empty())
        {
            Label->SetText(TEXT("프레임 없음  |  [+ Add]: 커서 위치에 32x32 프레임 추가"));
        }
        else if (mSelectedFrame < 0 || mSelectedFrame >= (int)mFrames.size())
        {
            TCHAR TextBuffer[128];
            swprintf_s(TextBuffer, 128, L"총 %d 프레임  |  박스를 클릭해서 선택", (int)mFrames.size());
            Label->SetText(TextBuffer);
        }
        else
        {
            auto& Frame = mFrames[mSelectedFrame];
            TCHAR TextBuffer[160];
            swprintf_s(TextBuffer, 160, L"[%d/%d]  Start:(%.0f,%.0f)  Size:(%.0fx%.0f)  Pivot:(%.1f,%.1f)  |  드래그=이동  모서리=리사이즈  청선/자홍선=피벗",
                mSelectedFrame + 1, (int)mFrames.size(),
                Frame.Start.x, Frame.Start.y, Frame.Size.x, Frame.Size.y, mPivotX, mPivotY);
            Label->SetText(TextBuffer);
        }
    }
}

// ── 좌표 변환 ────────────────────────────────────────────────────────────────

FVector2 CSpriteViewerUI::PanelToTex(FVector2 PanelLocalPos) const
{
    return { (PanelLocalPos.x - mTexLocalX) / mScale,
             (PanelLocalPos.y - mTexLocalY) / mScale };
}

float CSpriteViewerUI::FrameDispX(int FrameIdx) const { return mTexLocalX + mFrames[FrameIdx].Start.x * mScale; }
float CSpriteViewerUI::FrameDispY(int FrameIdx) const { return mTexLocalY + mFrames[FrameIdx].Start.y * mScale; }
float CSpriteViewerUI::FrameDispW(int FrameIdx) const { return max(2.f, mFrames[FrameIdx].Size.x * mScale); }
float CSpriteViewerUI::FrameDispH(int FrameIdx) const { return max(2.f, mFrames[FrameIdx].Size.y * mScale); }

// ── 히트 테스트 ───────────────────────────────────────────────────────────────

// 프레임 박스 안쪽(테두리가 아니라 전체 영역)을 눌렀는지
bool CSpriteViewerUI::HitTestFrame(int FrameIdx, FVector2 mouseScreen) const
{
    if (FrameIdx < 0 || FrameIdx >= (int)mFrames.size()) return false;

    // 마우스를 패널 로컬 좌표로 옮긴다.
    FVector3 PanelPos = GetPos();
    float LocalX = mouseScreen.x - PanelPos.x;
    float LocalY = mouseScreen.y - PanelPos.y;

    float BoxX = FrameDispX(FrameIdx), BoxY = FrameDispY(FrameIdx);
    float BoxW = FrameDispW(FrameIdx), BoxH = FrameDispH(FrameIdx);

    return LocalX >= BoxX && LocalX < BoxX + BoxW &&
           LocalY >= BoxY && LocalY < BoxY + BoxH;
}

// Corner: 0=TL 1=TR 2=BL 3=BR
bool CSpriteViewerUI::HitTestHandle(int Corner, FVector2 mouseScreen) const
{
    if (Corner < 0 || Corner >= 4 || !mHandleHitRects[Corner].valid) return false;

    FVector3 PanelPos = GetPos();
    float LocalX = mouseScreen.x - PanelPos.x;
    float LocalY = mouseScreen.y - PanelPos.y;

    const auto& HitRect = mHandleHitRects[Corner];

    return LocalX >= HitRect.x && LocalX < HitRect.x + HitRect.w &&
           LocalY >= HitRect.y && LocalY < HitRect.y + HitRect.h;
}

bool CSpriteViewerUI::IsInCanvas(FVector2 mouseScreen) const
{
    FVector3 PanelPos = GetPos();
    float LocalX = mouseScreen.x - PanelPos.x;
    float LocalY = mouseScreen.y - PanelPos.y;

    return LocalX >= 0.f && LocalX < PANEL_W &&
           LocalY >= CANVAS_Y && LocalY < CANVAS_Y + CANVAS_H;
}

// ── 아웃라인 사각형 헬퍼 ─────────────────────────────────────────────────────
// 두께 T인 4개 얇은 바(Top/Bottom/Left/Right)로 빈 테두리를 그린다

void CSpriteViewerUI::MakeRectBorder(const std::string& Prefix,
    float X, float Y, float Width, float Height, float Thickness,
    float Red, float Green, float Blue, float Alpha, int ZOrder)
{
    Thickness = min(Thickness, min(Width, Height) * 0.5f);
    if (Thickness <= 0.f) return;

    auto SetAll = [&](std::shared_ptr<CButton> Button)
    {
        Button->SetTint(EWidgetState::Normal,  Red, Green, Blue, Alpha);
        Button->SetTint(EWidgetState::Hovered, Red, Green, Blue, Alpha);
        Button->SetTint(EWidgetState::Clicked, Red, Green, Blue, Alpha);
        Button->SetTint(EWidgetState::Release, Red, Green, Blue, Alpha);
        Button->SetTint(EWidgetState::Disable, Red, Green, Blue, Alpha);
    };

    // Top
    if (auto Button = CreateWidget<CButton>(Prefix + "T", ZOrder).lock())
    { Button->SetPos(X, Y);           Button->SetSize(Width, Thickness);           SetAll(Button); }
    // Bottom
    if (auto Button = CreateWidget<CButton>(Prefix + "B", ZOrder).lock())
    { Button->SetPos(X, Y + Height - Thickness);  Button->SetSize(Width, Thickness);           SetAll(Button); }

    float InnerHeight = Height - Thickness * 2.f;
    if (InnerHeight > 0.f)
    {
        // Left
        if (auto Button = CreateWidget<CButton>(Prefix + "L", ZOrder).lock())
        { Button->SetPos(X, Y + Thickness);       Button->SetSize(Thickness, InnerHeight);  SetAll(Button); }
        // Right
        if (auto Button = CreateWidget<CButton>(Prefix + "R", ZOrder).lock())
        { Button->SetPos(X + Width - Thickness, Y + Thickness); Button->SetSize(Thickness, InnerHeight); SetAll(Button); }
    }
}

// ── 프레임 오버레이 재생성 ────────────────────────────────────────────────────

void CSpriteViewerUI::RebuildFrameOverlays()
{
    // 이전 동적 위젯 제거
    // resize(mStaticChildCount) 대신 이름 기반으로 제거한다.
    // Render()가 매프레임 ZOrder 기준으로 mChildList를 정렬하기 때문에
    // resize는 정적 위젯을 잘라내고 동적 위젯을 남기는 버그를 일으킨다.
    mChildList.erase(
        std::remove_if(mChildList.begin(), mChildList.end(),
            [](const std::shared_ptr<CWidget>& Widget)
            {
                const std::string& WidgetName = Widget->GetName();
                return WidgetName.size() >= 4 &&
                    (WidgetName.compare(0, 4, "SVFr") == 0 ||
                     WidgetName.compare(0, 4, "SVHa") == 0 ||
                     WidgetName.compare(0, 4, "SVPi") == 0);
            }),
        mChildList.end());
    for (auto& Rect : mHandleHitRects) Rect.valid = false;

    // 캔버스 유효 범위 (패널 로컬 좌표) — 이 밖으로 나가는 부분은 잘라낸다
    const float CanvasLeft = 0.f,      CanvasRight = PANEL_W;
    const float CanvasTop = CANVAS_Y, CanvasBottom = CANVAS_Y + CANVAS_H;

    // ── 선택된 프레임 박스 (ZOrder 5 → 캔버스/텍스처 위) ─────────────────────
    // 모든 프레임을 그리지 않고 편집 중인 하나만 그린다.
    // 테두리 바 4개 대신 반투명 사각형 하나라서, 캔버스 클리핑도
    // 사각형 교집합 한 번으로 끝난다. (아래 스프라이트가 비치도록 알파를 낮게 준다)
    if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
    {
        float BoxX = FrameDispX(mSelectedFrame), BoxY = FrameDispY(mSelectedFrame);
        float BoxW = FrameDispW(mSelectedFrame), BoxH = FrameDispH(mSelectedFrame);

        float ClipLeft = max(BoxX, CanvasLeft), ClipRight = min(BoxX + BoxW, CanvasRight);
        float ClipTop = max(BoxY, CanvasTop), ClipBottom = min(BoxY + BoxH, CanvasBottom);

        if (ClipRight > ClipLeft && ClipBottom > ClipTop)
        {
            if (auto Button = CreateWidget<CButton>("SVFrBox", 5).lock())
            {
                Button->SetPos(ClipLeft, ClipTop);
                Button->SetSize(ClipRight - ClipLeft, ClipBottom - ClipTop);
                Button->SetTint(EWidgetState::Normal,  1.f, 0.85f, 0.10f, 0.25f);
                Button->SetTint(EWidgetState::Hovered, 1.f, 0.85f, 0.10f, 0.25f);
                Button->SetTint(EWidgetState::Clicked, 1.f, 0.85f, 0.10f, 0.25f);
                Button->SetTint(EWidgetState::Release, 1.f, 0.85f, 0.10f, 0.25f);
                Button->SetTint(EWidgetState::Disable, 1.f, 0.85f, 0.10f, 0.25f);
            }
        }
    }

    // 선택된 프레임의 리사이즈 핸들 (4 모서리, ZOrder 8)
    if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
    {
        float BoxX = FrameDispX(mSelectedFrame);
        float BoxY = FrameDispY(mSelectedFrame);
        float BoxW = FrameDispW(mSelectedFrame);
        float BoxH = FrameDispH(mSelectedFrame);
        constexpr float HandleSize = 10.f;

        // 모서리에 걸치도록 핸들 중심을 꼭짓점에 맞춘다. TL, TR, BL, BR 순서
        float HandleX[4] = { BoxX - HandleSize * 0.5f, BoxX + BoxW - HandleSize * 0.5f,
                             BoxX - HandleSize * 0.5f, BoxX + BoxW - HandleSize * 0.5f };
        float HandleY[4] = { BoxY - HandleSize * 0.5f, BoxY - HandleSize * 0.5f,
                             BoxY + BoxH - HandleSize * 0.5f, BoxY + BoxH - HandleSize * 0.5f };

        for (int Corner = 0; Corner < 4; Corner++)
        {
            // 히트 영역은 항상 저장 (캔버스 밖이어도 드래그 가능하게)
            mHandleHitRects[Corner] = { HandleX[Corner], HandleY[Corner], HandleSize, HandleSize, true };

            // 캔버스와 전혀 안 겹치면 시각 생략
            if (HandleX[Corner] + HandleSize <= CanvasLeft || HandleX[Corner] >= CanvasRight ||
                HandleY[Corner] + HandleSize <= CanvasTop || HandleY[Corner] >= CanvasBottom)
                continue;

            // 캔버스 범위로 잘라낸 뒤 아웃라인 그리기
            float ClipX = max(HandleX[Corner], CanvasLeft);
            float ClipY = max(HandleY[Corner], CanvasTop);
            float ClipW = min(HandleX[Corner] + HandleSize, CanvasRight) - ClipX;
            float ClipH = min(HandleY[Corner] + HandleSize, CanvasBottom) - ClipY;

            if (ClipW > 0.f && ClipH > 0.f)
                MakeRectBorder("SVHa" + to_string(Corner), ClipX, ClipY, ClipW, ClipH, 2.f,
                    1.f, 1.f, 0.f, 1.f, 8);
        }
    }

    // 피벗 기준선 (ZOrder 9) — 선택 프레임의 Start 기준 mPivotX/mPivotY 픽셀 위치
    // 세로선은 캔버스를 위아래로, 가로선은 좌우로 가로지른다.
    // 모든 프레임이 같은 값을 공유한다 (애니메이션 공통 pivot)
    if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
    {
        auto MakePivotLine = [&](const std::string& Name,
            float X, float Y, float Width, float Height, float Red, float Green, float Blue)
        {
            if (auto Button = CreateWidget<CButton>(Name, 9).lock())
            {
                Button->SetPos(X, Y);
                Button->SetSize(Width, Height);
                Button->SetTint(EWidgetState::Normal,  Red, Green, Blue, 0.55f);
                Button->SetTint(EWidgetState::Hovered, Red, Green, Blue, 0.85f);
                Button->SetTint(EWidgetState::Clicked, 1.f, 1.f, 0.f, 1.f);
                Button->SetTint(EWidgetState::Release, Red, Green, Blue, 0.55f);
                Button->SetTint(EWidgetState::Disable, Red, Green, Blue, 0.55f);
            }
        };

        // 세로선 — 행을 가로지른다 (청록)
        float PivotLocalX = FrameDispX(mSelectedFrame) + mPivotX * mScale;
        if (PivotLocalX >= CanvasLeft && PivotLocalX < CanvasRight)
            MakePivotLine("SVPivX", max(PivotLocalX - 1.f, CanvasLeft), CanvasTop,
                2.f, CanvasBottom - CanvasTop, 0.f, 1.f, 1.f);

        // 가로선 — 열을 가로지른다 (자홍) — 세로선과 구분되게 색을 다르게 준다
        float PivotLocalY = FrameDispY(mSelectedFrame) + mPivotY * mScale;
        if (PivotLocalY >= CanvasTop && PivotLocalY < CanvasBottom)
            MakePivotLine("SVPivY", CanvasLeft, max(PivotLocalY - 1.f, CanvasTop),
                CanvasRight - CanvasLeft, 2.f, 1.f, 0.f, 1.f);
    }
}

// ── Update ────────────────────────────────────────────────────────────────────

void CSpriteViewerUI::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    auto World = mWorld.lock();
    if (!World) return;

    auto Input = World->GetInput().lock();
    if (!Input) return;

    FVector2 Mouse   = Input->GetMousePos();
    bool Press   = Input->GetMouseState(EMouseType::LButton, EInputType::Press);
    bool Held    = Input->GetMouseState(EMouseType::LButton, EInputType::Hold);
    bool Release = Input->GetMouseState(EMouseType::LButton, EInputType::Release);

    FVector3 PanelPos = GetPos();

    // ── 닫기 ──────────────────────────────────────────────────────────────────
    if (auto Button = mCloseButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    { SetEnable(false); return; }

    // ── 줌 컨트롤 ─────────────────────────────────────────────────────────────
    if (auto Button = mZoomInButton.lock();  Button && Button->GetWidgetState() == EWidgetState::Release)
    { ApplyZoom(mScale * 1.25f); }
    if (auto Button = mZoomOutButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    { ApplyZoom(mScale * 0.80f); }
    if (auto Button = mZoomFitButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    { FitToCanvas(); RebuildFrameOverlays(); }

    // ── Focus 토글 버튼 ───────────────────────────────────────────────────────
    // 켤 때는 지금 고른 프레임으로 바로 한 번 이동해준다.
    if (auto Button = mFocusButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        SetAutoFocus(!mAutoFocus);

        if (mAutoFocus && mSelectedFrame >= 0)
            FocusFrame(mSelectedFrame);
    }

    // ── Slice LT 토글 버튼 ────────────────────────────────────────────────────
    if (auto Button = mMoveSliceLTButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        mMoveSliceLT = !mMoveSliceLT;
        if (auto Label = mMoveSliceLTLabel.lock())
            Label->SetText(mMoveSliceLT ? TEXT("[v] Slice LT") : TEXT("[ ] Slice LT"));
        if (auto SliceButton = mMoveSliceLTButton.lock())
        {
            // 켜지면 초록, 꺼지면 회색
            float TintR = mMoveSliceLT ? 0.10f : 0.18f;
            float TintG = mMoveSliceLT ? 0.38f : 0.18f;
            float TintB = mMoveSliceLT ? 0.10f : 0.22f;
            SliceButton->SetTint(EWidgetState::Normal,  TintR, TintG, TintB, 1.f);
            SliceButton->SetTint(EWidgetState::Hovered,
                min(TintR + .10f, 1.f), min(TintG + .10f, 1.f), min(TintB + .10f, 1.f), 1.f);
        }
    }

    // ── 단축키 등록 (최초 1회) ────────────────────────────────────────────────
    // 인풋은 VK 코드로 등록/조회해야 한다.
    // AddBindKey에 DIK 코드를 넘기면 ConvertKey가 매칭에 실패해 0xff를 돌려주고,
    // 그러면 mKeyState[0xff]를 보게 되어 그 키는 영원히 눌리지 않은 상태가 된다.
    if (!mArrowKeysRegistered)
    {
        Input->AddBindKey("SVLeft",  VK_LEFT);
        Input->AddBindKey("SVRight", VK_RIGHT);
        Input->AddBindKey("SVUp",    VK_UP);
        Input->AddBindKey("SVDown",  VK_DOWN);
        Input->AddBindKey("SVSpace", VK_SPACE);   // 프레임 복사 추가
        Input->AddBindKey("SVKeyD",  'D');        // Ctrl+D — 프레임 삭제
        mArrowKeysRegistered = true;
    }

    // ── 방향키: 선택된 프레임 Start(LT) 이동 ────────────────────────────────
    if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
    {
        auto& Frame  = mFrames[mSelectedFrame];
        constexpr float SPEED = 100.f;
        float DeltaX = 0.f, DeltaY = 0.f;

        if (mMoveSliceLT)
        {
            // 슬라이스 단위 이동 — 좌/우는 Size.W, 상/하는 Size.H 만큼 한 칸씩
            if (Input->GetKey(VK_LEFT,  EInputType::Press)) DeltaX = -Frame.Size.x;
            if (Input->GetKey(VK_RIGHT, EInputType::Press)) DeltaX =  Frame.Size.x;
            if (Input->GetKey(VK_UP,    EInputType::Press)) DeltaY = -Frame.Size.y;
            if (Input->GetKey(VK_DOWN,  EInputType::Press)) DeltaY =  Frame.Size.y;
        }
        else
        {
            if (Input->GetKey(VK_LEFT,  EInputType::Hold)) DeltaX = -SPEED * DeltaTime;
            if (Input->GetKey(VK_RIGHT, EInputType::Hold)) DeltaX =  SPEED * DeltaTime;
            if (Input->GetKey(VK_UP,    EInputType::Hold)) DeltaY = -SPEED * DeltaTime;
            if (Input->GetKey(VK_DOWN,  EInputType::Hold)) DeltaY =  SPEED * DeltaTime;
        }

        if (DeltaX != 0.f || DeltaY != 0.f)
        {
            Frame.Start.x = max(0.f, min(Frame.Start.x + DeltaX, mTexSize.x - Frame.Size.x));
            Frame.Start.y = max(0.f, min(Frame.Start.y + DeltaY, mTexSize.y - Frame.Size.y));
            RebuildFrameOverlays();
            UpdateInfoLabel();
            if (mOnFrameChanged) mOnFrameChanged(mSelectedFrame, Frame.Start, Frame.Size);
        }
    }

    // ── Ctrl+D: 선택된 프레임 삭제 ────────────────────────────────────────────
    // Space보다 먼저 검사한다. (콜백이 mFrames를 다시 채우므로 처리 후 즉시 빠져나간다)
    if (Input->GetCtrl(EInputType::Hold) && Input->GetKey('D', EInputType::Press))
    {
        if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
        {
            int DeleteIdx = mSelectedFrame;
            mSelectedFrame = -1;
            if (mOnFrameDeleted) mOnFrameDeleted(DeleteIdx);
            return;
        }
    }

    // ── Space: 현재 프레임을 복사해서 추가 (추가된 프레임이 선택된다) ────────
    if (Input->GetKey(VK_SPACE, EInputType::Press))
    {
        // 선택된 프레임이 없으면 기본 크기로 새로 만든다.
        FVector2 NewStart = { 0.f, 0.f };
        FVector2 NewSize  = { 32.f, 32.f };

        if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
        {
            NewStart = mFrames[mSelectedFrame].Start;
            NewSize  = mFrames[mSelectedFrame].Size;
        }

        // 콜백에서 프레임을 추가하고 마지막 프레임을 선택한 뒤 SetFrames로 되돌려준다.
        if (mOnFrameAdded)
        {
            mOnFrameAdded(NewStart, NewSize);
            return;
        }
    }

    // ── 마우스 휠 줌 (캔버스 위에서: 위=확대, 아래=축소) ─────────────────────
    {
        float Wheel = Input->GetMouseWheelDelta();
        if (Wheel != 0.f && IsInCanvas(Mouse))
        {
            if (Wheel > 0.f) ApplyZoom(mScale * 1.25f);
            else             ApplyZoom(mScale * 0.80f);
        }
    }

    // ── 프레임 추가 (커서 위치) ────────────────────────────────────────────────
    if (auto Button = mAddButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        float LocalX = Mouse.x - PanelPos.x;
        float LocalY = Mouse.y - PanelPos.y;
        FVector2 TexPos = PanelToTex({ LocalX, LocalY });
        TexPos.x = max(0.f, min(TexPos.x - 16.f, mTexSize.x - 32.f));
        TexPos.y = max(0.f, min(TexPos.y - 16.f, mTexSize.y - 32.f));
        if (mOnFrameAdded) mOnFrameAdded(TexPos, { 32.f, 32.f });
    }

    // ── 프레임 삭제 (선택된 프레임) ───────────────────────────────────────────
    if (auto Button = mDelButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
        {
            int DeleteIdx = mSelectedFrame;
            mSelectedFrame = -1;
            if (mOnFrameDeleted) mOnFrameDeleted(DeleteIdx);
        }
    }

    // ── 패널 드래그 (타이틀 바) ────────────────────────────────────────────────
    bool bInTitle = (Mouse.x >= PanelPos.x && Mouse.x < PanelPos.x + PANEL_W &&
                    Mouse.y >= PanelPos.y && Mouse.y < PanelPos.y + TITLE_H);

    if (Press && bInTitle && !mDragging && !mResizing)
    {
        mPanelDragging  = true;
        mDragMouseStart = Mouse;
        mPanelPosStart  = { PanelPos.x, PanelPos.y };
    }
    if (mPanelDragging)
    {
        if (Held)
        {
            SetPos(mPanelPosStart.x + (Mouse.x - mDragMouseStart.x),
                   mPanelPosStart.y + (Mouse.y - mDragMouseStart.y));
            RebuildFrameOverlays();
        }
        if (Release) mPanelDragging = false;
        return;
    }

    // ── 캔버스 패닝 (우클릭 드래그: 좌=이미지 우, 우=이미지 좌) ──────────────
    {
        bool RPress   = Input->GetMouseState(EMouseType::RButton, EInputType::Press);
        bool RHeld    = Input->GetMouseState(EMouseType::RButton, EInputType::Hold);
        bool RRelease = Input->GetMouseState(EMouseType::RButton, EInputType::Release);

        if (RPress && IsInCanvas(Mouse) && !mCanvasPanning && !mResizing && !mDragging)
        {
            mCanvasPanning  = true;
            mPanMouseStart  = Mouse;
            mPanOriginStart = { mTexLocalX, mTexLocalY };
        }
        if (mCanvasPanning)
        {
            if (RHeld)
            {
                mTexLocalX = mPanOriginStart.x + (Mouse.x - mPanMouseStart.x);
                mTexLocalY = mPanOriginStart.y + (Mouse.y - mPanMouseStart.y);
                if (auto Button = mTexButton.lock())
                    Button->SetPos(mTexLocalX, mTexLocalY);
                RebuildFrameOverlays();
            }
            if (RRelease) mCanvasPanning = false;
            return;
        }
    }

    // ── 피벗 기준선 드래그 ────────────────────────────────────────────────────
    // 선이 2px라 정확히 찍기 어려우므로 ±4px 여유를 두고 좌표로 판정한다.
    // 두 선이 만나는 지점에서는 세로선이 우선한다.
    if (Press && !mDragging && !mResizing && !mCanvasPanning && mSelectedFrame >= 0)
    {
        float PivotScreenX = PanelPos.x + FrameDispX(mSelectedFrame) + mPivotX * mScale;
        float PivotScreenY = PanelPos.y + FrameDispY(mSelectedFrame) + mPivotY * mScale;

        if (IsInCanvas(Mouse))
        {
            if (std::abs(Mouse.x - PivotScreenX) <= 4.f)
                mPivotXDragging = true;
            else if (std::abs(Mouse.y - PivotScreenY) <= 4.f)
                mPivotYDragging = true;
        }
    }
    if (mPivotXDragging || mPivotYDragging)
    {
        if (Held || Press)
        {
            if (mSelectedFrame >= 0 && mScale > 0.f)
            {
                if (mPivotXDragging)
                    mPivotX = (Mouse.x - PanelPos.x - FrameDispX(mSelectedFrame)) / mScale;
                else
                    mPivotY = (Mouse.y - PanelPos.y - FrameDispY(mSelectedFrame)) / mScale;

                RebuildFrameOverlays();
                UpdateInfoLabel();

                // 값의 주인은 시퀀스이므로 애님 에디터에 돌려준다.
                if (mOnPivotChanged) mOnPivotChanged(mPivotX, mPivotY);
            }
        }
        if (Release)
        {
            mPivotXDragging = false;
            mPivotYDragging = false;
            RebuildFrameOverlays();
        }
        return;
    }

    // ── 리사이즈 핸들 드래그 ──────────────────────────────────────────────────
    if (Press && !mDragging && mSelectedFrame >= 0)
    {
        for (int Corner = 0; Corner < 4; Corner++)
        {
            if (HitTestHandle(Corner, Mouse))
            {
                mResizing       = true;
                mResizeCorner   = Corner;
                mDragMouseStart = Mouse;
                mDragDataStart  = mFrames[mSelectedFrame].Start;
                mDragSizeStart  = mFrames[mSelectedFrame].Size;
                break;
            }
        }
    }

    if (mResizing)
    {
        if (Held)
        {
            float DeltaX = (Mouse.x - mDragMouseStart.x) / mScale;
            float DeltaY = (Mouse.y - mDragMouseStart.y) / mScale;
            auto& Frame = mFrames[mSelectedFrame];

            switch (mResizeCorner)
            {
            case 0: // TL: Start 이동, Size 반대 방향
                Frame.Start.x = mDragDataStart.x + DeltaX;
                Frame.Start.y = mDragDataStart.y + DeltaY;
                Frame.Size.x  = max(4.f, mDragSizeStart.x - DeltaX);
                Frame.Size.y  = max(4.f, mDragSizeStart.y - DeltaY);
                break;
            case 1: // TR: 위쪽 Start.y 이동, 오른쪽 Size.x 증가
                Frame.Start.y = mDragDataStart.y + DeltaY;
                Frame.Size.x  = max(4.f, mDragSizeStart.x + DeltaX);
                Frame.Size.y  = max(4.f, mDragSizeStart.y - DeltaY);
                break;
            case 2: // BL: 왼쪽 Start.x 이동, 아래쪽 Size.y 증가
                Frame.Start.x = mDragDataStart.x + DeltaX;
                Frame.Size.x  = max(4.f, mDragSizeStart.x - DeltaX);
                Frame.Size.y  = max(4.f, mDragSizeStart.y + DeltaY);
                break;
            case 3: // BR: Size 증가
                Frame.Size.x = max(4.f, mDragSizeStart.x + DeltaX);
                Frame.Size.y = max(4.f, mDragSizeStart.y + DeltaY);
                break;
            }

            RebuildFrameOverlays();
            UpdateInfoLabel();
            if (mOnFrameChanged) mOnFrameChanged(mSelectedFrame, Frame.Start, Frame.Size);
        }
        if (Release) mResizing = false;
        return;
    }

    // ── 프레임 박스 드래그 (선택 + 이동) ─────────────────────────────────────
    if (Press && IsInCanvas(Mouse))
    {
        bool bHitAny = false;

        // 상위(나중에 그려진) 프레임부터 히트 테스트
        for (int i = (int)mFrames.size() - 1; i >= 0; i--)
        {
            if (HitTestFrame(i, Mouse))
            {
                bHitAny = true;
                if (i != mSelectedFrame)
                {
                    mSelectedFrame = i;
                    RebuildFrameOverlays();
                    UpdateInfoLabel();
                    if (mOnFrameSelected) mOnFrameSelected(i);
                }
                mDragging       = true;
                mDragMouseStart = Mouse;
                mDragDataStart  = mFrames[i].Start;
                break;
            }
        }

        if (!bHitAny && mSelectedFrame >= 0)
        {
            mSelectedFrame = -1;
            RebuildFrameOverlays();
            UpdateInfoLabel();
        }
    }

    if (mDragging)
    {
        if (Held)
        {
            float DeltaX = (Mouse.x - mDragMouseStart.x) / mScale;
            float DeltaY = (Mouse.y - mDragMouseStart.y) / mScale;
            auto& Frame = mFrames[mSelectedFrame];
            Frame.Start.x = max(0.f, min(mDragDataStart.x + DeltaX, mTexSize.x - Frame.Size.x));
            Frame.Start.y = max(0.f, min(mDragDataStart.y + DeltaY, mTexSize.y - Frame.Size.y));

            RebuildFrameOverlays();
            UpdateInfoLabel();
            if (mOnFrameChanged) mOnFrameChanged(mSelectedFrame, Frame.Start, Frame.Size);
        }
        if (Release) mDragging = false;
    }
}

// ── Clone ─────────────────────────────────────────────────────────────────────

CSpriteViewerUI* CSpriteViewerUI::Clone()
{
    return new CSpriteViewerUI(*this);
}
