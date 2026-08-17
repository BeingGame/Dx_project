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

std::weak_ptr<CButton> CSpriteViewerUI::MakeBtn(const std::string& Name,
    float X, float Y, float W, float H,
    float R, float G, float B, float A, int Z)
{
    auto Btn = CreateWidget<CButton>(Name, Z).lock();
    if (Btn)
    {
        Btn->SetPos(X, Y);
        Btn->SetSize(W, H);
        float HR = min(R + .12f, 1.f), HG = min(G + .12f, 1.f), HB = min(B + .12f, 1.f);
        float CR = min(R + .22f, 1.f), CG = min(G + .22f, 1.f), CB = min(B + .22f, 1.f);
        Btn->SetTint(EWidgetState::Normal,  R,  G,  B,  A);
        Btn->SetTint(EWidgetState::Hovered, HR, HG, HB, A);
        Btn->SetTint(EWidgetState::Clicked, CR, CG, CB, A);
        Btn->SetTint(EWidgetState::Release, HR, HG, HB, A);
        Btn->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.15f, A * 0.5f);
    }
    return Btn;
}

std::weak_ptr<CTextBlock> CSpriteViewerUI::MakeLbl(const std::string& Name,
    float X, float Y, float W, float H, const wchar_t* Text,
    float FontSize, ETextAlignH AlignH, int Z)
{
    auto Lbl = CreateWidget<CTextBlock>(Name, Z).lock();
    if (Lbl)
    {
        Lbl->SetPos(X, Y);
        Lbl->SetSize(W, H);
        Lbl->SetText(Text);
        Lbl->SetFontSize(FontSize);
        Lbl->SetTextColor(FVector4(0.88f, 0.88f, 0.92f, 1.f));
        Lbl->SetAlignH(AlignH);
        Lbl->SetAlignV(ETextAlignV::Middle);
    }
    return Lbl;
}


bool CSpriteViewerUI::Init()
{
    CWidgetContainer::Init();

    SetPos(PANEL_X, PANEL_Y);
    SetSize(PANEL_W, PANEL_H);

    // ── 배경 ──────────────────────────────────────────────────────────────────
    auto Bg = CreateWidget<CButton>("SVBg", 0).lock();
    if (Bg)
    {
        Bg->SetPos(0.f, 0.f);
        Bg->SetSize(PANEL_W, PANEL_H);
        Bg->SetTint(EWidgetState::Normal,  0.10f, 0.10f, 0.14f, 0.97f);
        Bg->SetTint(EWidgetState::Hovered, 0.10f, 0.10f, 0.14f, 0.97f);
        Bg->SetTint(EWidgetState::Clicked, 0.10f, 0.10f, 0.14f, 0.97f);
        Bg->SetTint(EWidgetState::Release, 0.10f, 0.10f, 0.14f, 0.97f);
        Bg->SetTint(EWidgetState::Disable, 0.10f, 0.10f, 0.14f, 0.97f);
        mBgBtn = Bg;
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
    MakeLbl("SVTitleLbl", 6.f, 0.f, PANEL_W - 40.f, TITLE_H,
        TEXT("Sprite Viewer  (drag: move / corners: resize / Arrow: move / Space: dup / Ctrl+D: del)"), 11.f);

    // 닫기 버튼
    {
        auto Cl = CreateWidget<CButton>("SVClose", 10).lock();
        if (Cl)
        {
            Cl->SetPos(PANEL_W - 28.f, 2.f);
            Cl->SetSize(24.f, TITLE_H - 4.f);
            Cl->SetTint(EWidgetState::Normal,  0.55f, 0.15f, 0.15f, 1.f);
            Cl->SetTint(EWidgetState::Hovered, 0.80f, 0.20f, 0.20f, 1.f);
            Cl->SetTint(EWidgetState::Clicked, 0.90f, 0.25f, 0.25f, 1.f);
            Cl->SetTint(EWidgetState::Release, 0.55f, 0.15f, 0.15f, 1.f);
            Cl->SetTint(EWidgetState::Disable, 0.30f, 0.10f, 0.10f, 1.f);
            mCloseBtn = Cl;
        }
        MakeLbl("SVCloseL", PANEL_W - 28.f, 2.f, 24.f, TITLE_H - 4.f,
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
            mZoomOutBtn = ZOut;
        }
        MakeLbl("SVZOutL", CtrlX, CtrlY, 24.f, 20.f, TEXT("-"), 12.f, ETextAlignH::Center);
        CtrlX += 26.f;

        auto ZLbl = CreateWidget<CTextBlock>("SVZLbl", 11).lock();
        if (ZLbl) {
            ZLbl->SetPos(CtrlX, CtrlY);
            ZLbl->SetSize(56.f, 20.f);
            ZLbl->SetText(TEXT("100%"));
            ZLbl->SetFontSize(11.f);
            ZLbl->SetTextColor(FVector4(0.9f, 0.9f, 0.9f, 1.f));
            ZLbl->SetAlignH(ETextAlignH::Center);
            ZLbl->SetAlignV(ETextAlignV::Middle);
            mZoomLbl = ZLbl;
        }
        CtrlX += 58.f;

        auto ZIn = CreateWidget<CButton>("SVZIn", 10).lock();
        if (ZIn) {
            ZIn->SetPos(CtrlX, CtrlY);
            ZIn->SetSize(24.f, 20.f);
            ZIn->SetTint(EWidgetState::Normal,  0.22f, 0.22f, 0.28f, 1.f);
            ZIn->SetTint(EWidgetState::Hovered, 0.32f, 0.32f, 0.40f, 1.f);
            ZIn->SetTint(EWidgetState::Clicked, 0.42f, 0.42f, 0.52f, 1.f);
            ZIn->SetTint(EWidgetState::Release, 0.22f, 0.22f, 0.28f, 1.f);
            ZIn->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.18f, 1.f);
            mZoomInBtn = ZIn;
        }
        MakeLbl("SVZInL", CtrlX, CtrlY, 24.f, 20.f, TEXT("+"), 12.f, ETextAlignH::Center);
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
            mZoomFitBtn = ZFit;
        }
        MakeLbl("SVZFitL", CtrlX, CtrlY, 40.f, 20.f, TEXT("Fit"), 10.f, ETextAlignH::Center);
        CtrlX += 48.f;

        // ── Slice LT 토글 (방향키로 프레임 Start 이동, slice=Size 단위 / 아니면 연속) ──
        auto MvBtn = CreateWidget<CButton>("SVMvSlice", 10).lock();
        if (MvBtn)
        {
            MvBtn->SetPos(CtrlX, CtrlY);
            MvBtn->SetSize(86.f, 20.f);
            MvBtn->SetTint(EWidgetState::Normal,  0.18f, 0.18f, 0.22f, 1.f);
            MvBtn->SetTint(EWidgetState::Hovered, 0.26f, 0.26f, 0.32f, 1.f);
            MvBtn->SetTint(EWidgetState::Clicked, 0.32f, 0.32f, 0.40f, 1.f);
            MvBtn->SetTint(EWidgetState::Release, 0.18f, 0.18f, 0.22f, 1.f);
            MvBtn->SetTint(EWidgetState::Disable, 0.12f, 0.12f, 0.15f, 1.f);
            mMoveSliceLTBtn = MvBtn;
        }
        auto MvLbl = CreateWidget<CTextBlock>("SVMvSliceL", 11).lock();
        if (MvLbl)
        {
            MvLbl->SetPos(CtrlX, CtrlY);
            MvLbl->SetSize(86.f, 20.f);
            MvLbl->SetText(TEXT("[ ] Slice LT"));
            MvLbl->SetFontSize(9.f);
            MvLbl->SetTextColor(FVector4(0.75f, 0.75f, 0.80f, 1.f));
            MvLbl->SetAlignH(ETextAlignH::Center);
            MvLbl->SetAlignV(ETextAlignV::Middle);
            mMoveSliceLTLbl = MvLbl;
        }
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
    auto TexBtn = CreateWidget<CButton>("SVTex", 2).lock();
    if (TexBtn)
    {
        TexBtn->SetPos(0.f, CANVAS_Y);
        TexBtn->SetSize(0.f, 0.f);
        TexBtn->SetTint(EWidgetState::Normal,  1.f, 1.f, 1.f, 1.f);
        TexBtn->SetTint(EWidgetState::Hovered, 1.f, 1.f, 1.f, 1.f);
        TexBtn->SetTint(EWidgetState::Clicked, 1.f, 1.f, 1.f, 1.f);
        TexBtn->SetTint(EWidgetState::Release, 1.f, 1.f, 1.f, 1.f);
        TexBtn->SetTint(EWidgetState::Disable, 1.f, 1.f, 1.f, 1.f);
        mTexBtn = TexBtn;
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

    auto InfoLbl = CreateWidget<CTextBlock>("SVInfo", 11).lock();
    if (InfoLbl)
    {
        InfoLbl->SetPos(6.f, InfoY + 2.f);
        InfoLbl->SetSize(PANEL_W - 116.f, INFO_H - 4.f);
        InfoLbl->SetText(TEXT("텍스처를 설정한 뒤 Open Sprite Viewer를 클릭하세요."));
        InfoLbl->SetFontSize(10.f);
        InfoLbl->SetTextColor(FVector4(0.85f, 0.85f, 0.92f, 1.f));
        InfoLbl->SetAlignV(ETextAlignV::Middle);
        mInfoLbl = InfoLbl;
    }

    // [+ Add] 버튼 (커서 위치에 32x32 프레임 추가)
    float InfoBtnX = PANEL_W - 108.f; // 정보 바 우측 버튼들의 가로 커서
    auto AddBtn = CreateWidget<CButton>("SVAdd", 10).lock();
    if (AddBtn)
    {
        AddBtn->SetPos(InfoBtnX, InfoY + 14.f);
        AddBtn->SetSize(48.f, 22.f);
        AddBtn->SetTint(EWidgetState::Normal,  0.14f, 0.34f, 0.14f, 1.f);
        AddBtn->SetTint(EWidgetState::Hovered, 0.20f, 0.46f, 0.20f, 1.f);
        AddBtn->SetTint(EWidgetState::Clicked, 0.26f, 0.56f, 0.26f, 1.f);
        AddBtn->SetTint(EWidgetState::Release, 0.14f, 0.34f, 0.14f, 1.f);
        AddBtn->SetTint(EWidgetState::Disable, 0.10f, 0.20f, 0.10f, 1.f);
        mAddBtn = AddBtn;
    }
    MakeLbl("SVAddL", InfoBtnX, InfoY + 14.f, 48.f, 22.f, TEXT("+ Add"), 10.f, ETextAlignH::Center);
    InfoBtnX += 52.f;

    // [- Del] 버튼 (선택된 프레임 삭제)
    auto DelBtn = CreateWidget<CButton>("SVDel", 10).lock();
    if (DelBtn)
    {
        DelBtn->SetPos(InfoBtnX, InfoY + 14.f);
        DelBtn->SetSize(48.f, 22.f);
        DelBtn->SetTint(EWidgetState::Normal,  0.34f, 0.14f, 0.14f, 1.f);
        DelBtn->SetTint(EWidgetState::Hovered, 0.46f, 0.20f, 0.20f, 1.f);
        DelBtn->SetTint(EWidgetState::Clicked, 0.56f, 0.26f, 0.26f, 1.f);
        DelBtn->SetTint(EWidgetState::Release, 0.34f, 0.14f, 0.14f, 1.f);
        DelBtn->SetTint(EWidgetState::Disable, 0.20f, 0.10f, 0.10f, 1.f);
        mDelBtn = DelBtn;
    }
    MakeLbl("SVDelL", InfoBtnX, InfoY + 14.f, 48.f, 22.f, TEXT("- Del"), 10.f, ETextAlignH::Center);

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

    if (auto B = mTexBtn.lock())
    {
        B->SetTexture(EWidgetState::Normal,  mTexName);
        B->SetTexture(EWidgetState::Hovered, mTexName);
        B->SetTexture(EWidgetState::Clicked, mTexName);
        B->SetTexture(EWidgetState::Release, mTexName);
        B->SetTexture(EWidgetState::Disable, mTexName);
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

void CSpriteViewerUI::SelectFrame(int fi)
{
    mSelectedFrame = (fi >= 0 && fi < (int)mFrames.size()) ? fi : -1;
    RebuildFrameOverlays();
    UpdateInfoLabel();
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

    if (auto B = mTexBtn.lock())
    {
        B->SetPos(mTexLocalX, mTexLocalY);
        B->SetSize(DrawW, DrawH);
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

    if (auto B = mTexBtn.lock())
    {
        B->SetPos(mTexLocalX, mTexLocalY);
        B->SetSize(DrawW, DrawH);
    }
    UpdateZoomLabel();
    RebuildFrameOverlays();
}

void CSpriteViewerUI::UpdateZoomLabel()
{
    if (auto L = mZoomLbl.lock())
    {
        TCHAR Buf[32];
        swprintf_s(Buf, 32, L"%.0f%%", mScale * 100.f);
        L->SetText(Buf);
    }
}

void CSpriteViewerUI::UpdateInfoLabel()
{
    if (auto L = mInfoLbl.lock())
    {
        if (mFrames.empty())
        {
            L->SetText(TEXT("프레임 없음  |  [+ Add]: 커서 위치에 32x32 프레임 추가"));
        }
        else if (mSelectedFrame < 0 || mSelectedFrame >= (int)mFrames.size())
        {
            TCHAR Buf[128];
            swprintf_s(Buf, 128, L"총 %d 프레임  |  박스를 클릭해서 선택", (int)mFrames.size());
            L->SetText(Buf);
        }
        else
        {
            auto& F = mFrames[mSelectedFrame];
            TCHAR Buf[160];
            swprintf_s(Buf, 160, L"[%d/%d]  Start:(%.0f,%.0f)  Size:(%.0fx%.0f)  Pivot:(%.1f,%.1f)  |  드래그=이동  모서리=리사이즈  청선/자홍선=피벗",
                mSelectedFrame + 1, (int)mFrames.size(),
                F.Start.x, F.Start.y, F.Size.x, F.Size.y, mPivotX, mPivotY);
            L->SetText(Buf);
        }
    }
}

// ── 좌표 변환 ────────────────────────────────────────────────────────────────

FVector2 CSpriteViewerUI::PanelToTex(FVector2 PanelLocalPos) const
{
    return { (PanelLocalPos.x - mTexLocalX) / mScale,
             (PanelLocalPos.y - mTexLocalY) / mScale };
}

float CSpriteViewerUI::FrameDispX(int fi) const { return mTexLocalX + mFrames[fi].Start.x * mScale; }
float CSpriteViewerUI::FrameDispY(int fi) const { return mTexLocalY + mFrames[fi].Start.y * mScale; }
float CSpriteViewerUI::FrameDispW(int fi) const { return max(2.f, mFrames[fi].Size.x * mScale); }
float CSpriteViewerUI::FrameDispH(int fi) const { return max(2.f, mFrames[fi].Size.y * mScale); }

// ── 히트 테스트 ───────────────────────────────────────────────────────────────

// 프레임 박스 안쪽(테두리가 아니라 전체 영역)을 눌렀는지
bool CSpriteViewerUI::HitTestFrame(int fi, FVector2 mouseScreen) const
{
    if (fi < 0 || fi >= (int)mFrames.size()) return false;

    // 마우스를 패널 로컬 좌표로 옮긴다.
    FVector3 PanelPos = GetPos();
    float LocalX = mouseScreen.x - PanelPos.x;
    float LocalY = mouseScreen.y - PanelPos.y;

    float BoxX = FrameDispX(fi), BoxY = FrameDispY(fi);
    float BoxW = FrameDispW(fi), BoxH = FrameDispH(fi);

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

    const auto& Hit = mHandleHitRects[Corner];

    return LocalX >= Hit.x && LocalX < Hit.x + Hit.w &&
           LocalY >= Hit.y && LocalY < Hit.y + Hit.h;
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
    float X, float Y, float W, float H, float T,
    float R, float G, float B, float A, int Z)
{
    T = min(T, min(W, H) * 0.5f);
    if (T <= 0.f) return;

    auto SetAll = [&](std::shared_ptr<CButton> Btn)
    {
        Btn->SetTint(EWidgetState::Normal,  R, G, B, A);
        Btn->SetTint(EWidgetState::Hovered, R, G, B, A);
        Btn->SetTint(EWidgetState::Clicked, R, G, B, A);
        Btn->SetTint(EWidgetState::Release, R, G, B, A);
        Btn->SetTint(EWidgetState::Disable, R, G, B, A);
    };

    // Top
    if (auto Btn = CreateWidget<CButton>(Prefix + "T", Z).lock())
    { Btn->SetPos(X, Y);           Btn->SetSize(W, T);           SetAll(Btn); }
    // Bottom
    if (auto Btn = CreateWidget<CButton>(Prefix + "B", Z).lock())
    { Btn->SetPos(X, Y + H - T);  Btn->SetSize(W, T);           SetAll(Btn); }

    float InnerH = H - T * 2.f;
    if (InnerH > 0.f)
    {
        // Left
        if (auto Btn = CreateWidget<CButton>(Prefix + "L", Z).lock())
        { Btn->SetPos(X, Y + T);       Btn->SetSize(T, InnerH);  SetAll(Btn); }
        // Right
        if (auto Btn = CreateWidget<CButton>(Prefix + "R", Z).lock())
        { Btn->SetPos(X + W - T, Y + T); Btn->SetSize(T, InnerH); SetAll(Btn); }
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
            [](const std::shared_ptr<CWidget>& w)
            {
                const std::string& n = w->GetName();
                return n.size() >= 4 &&
                    (n.compare(0, 4, "SVFr") == 0 ||
                     n.compare(0, 4, "SVHa") == 0 ||
                     n.compare(0, 4, "SVPi") == 0);
            }),
        mChildList.end());
    for (auto& R : mHandleHitRects) R.valid = false;

    // 캔버스 유효 범위 (패널 로컬 좌표) — 이 밖으로 나가는 부분은 잘라낸다
    const float CanvasL = 0.f,      CanvasR = PANEL_W;
    const float CanvasT = CANVAS_Y, CanvasB = CANVAS_Y + CANVAS_H;

    // ── 선택된 프레임 박스 (ZOrder 5 → 캔버스/텍스처 위) ─────────────────────
    // 모든 프레임을 그리지 않고 편집 중인 하나만 그린다.
    // 테두리 바 4개 대신 반투명 사각형 하나라서, 캔버스 클리핑도
    // 사각형 교집합 한 번으로 끝난다. (아래 스프라이트가 비치도록 알파를 낮게 준다)
    if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
    {
        float BoxX = FrameDispX(mSelectedFrame), BoxY = FrameDispY(mSelectedFrame);
        float BoxW = FrameDispW(mSelectedFrame), BoxH = FrameDispH(mSelectedFrame);

        float ClipL = max(BoxX, CanvasL), ClipR = min(BoxX + BoxW, CanvasR);
        float ClipT = max(BoxY, CanvasT), ClipB = min(BoxY + BoxH, CanvasB);

        if (ClipR > ClipL && ClipB > ClipT)
        {
            if (auto B = CreateWidget<CButton>("SVFrBox", 5).lock())
            {
                B->SetPos(ClipL, ClipT);
                B->SetSize(ClipR - ClipL, ClipB - ClipT);
                B->SetTint(EWidgetState::Normal,  1.f, 0.85f, 0.10f, 0.25f);
                B->SetTint(EWidgetState::Hovered, 1.f, 0.85f, 0.10f, 0.25f);
                B->SetTint(EWidgetState::Clicked, 1.f, 0.85f, 0.10f, 0.25f);
                B->SetTint(EWidgetState::Release, 1.f, 0.85f, 0.10f, 0.25f);
                B->SetTint(EWidgetState::Disable, 1.f, 0.85f, 0.10f, 0.25f);
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
            if (HandleX[Corner] + HandleSize <= CanvasL || HandleX[Corner] >= CanvasR ||
                HandleY[Corner] + HandleSize <= CanvasT || HandleY[Corner] >= CanvasB)
                continue;

            // 캔버스 범위로 잘라낸 뒤 아웃라인 그리기
            float ClipX = max(HandleX[Corner], CanvasL);
            float ClipY = max(HandleY[Corner], CanvasT);
            float ClipW = min(HandleX[Corner] + HandleSize, CanvasR) - ClipX;
            float ClipH = min(HandleY[Corner] + HandleSize, CanvasB) - ClipY;

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
            float X, float Y, float W, float H, float R, float G, float B2)
        {
            if (auto B = CreateWidget<CButton>(Name, 9).lock())
            {
                B->SetPos(X, Y);
                B->SetSize(W, H);
                B->SetTint(EWidgetState::Normal,  R, G, B2, 0.55f);
                B->SetTint(EWidgetState::Hovered, R, G, B2, 0.85f);
                B->SetTint(EWidgetState::Clicked, 1.f, 1.f, 0.f, 1.f);
                B->SetTint(EWidgetState::Release, R, G, B2, 0.55f);
                B->SetTint(EWidgetState::Disable, R, G, B2, 0.55f);
            }
        };

        // 세로선 — 행을 가로지른다 (청록)
        float PivotLocalX = FrameDispX(mSelectedFrame) + mPivotX * mScale;
        if (PivotLocalX >= CanvasL && PivotLocalX < CanvasR)
            MakePivotLine("SVPivX", max(PivotLocalX - 1.f, CanvasL), CanvasT,
                2.f, CanvasB - CanvasT, 0.f, 1.f, 1.f);

        // 가로선 — 열을 가로지른다 (자홍) — 세로선과 구분되게 색을 다르게 준다
        float PivotLocalY = FrameDispY(mSelectedFrame) + mPivotY * mScale;
        if (PivotLocalY >= CanvasT && PivotLocalY < CanvasB)
            MakePivotLine("SVPivY", CanvasL, max(PivotLocalY - 1.f, CanvasT),
                CanvasR - CanvasL, 2.f, 1.f, 0.f, 1.f);
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
    if (auto B = mCloseBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    { SetEnable(false); return; }

    // ── 줌 컨트롤 ─────────────────────────────────────────────────────────────
    if (auto B = mZoomInBtn.lock();  B && B->GetWidgetState() == EWidgetState::Release)
    { ApplyZoom(mScale * 1.25f); }
    if (auto B = mZoomOutBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    { ApplyZoom(mScale * 0.80f); }
    if (auto B = mZoomFitBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    { FitToCanvas(); RebuildFrameOverlays(); }

    // ── Slice LT 토글 버튼 ────────────────────────────────────────────────────
    if (auto B = mMoveSliceLTBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        mMoveSliceLT = !mMoveSliceLT;
        if (auto L = mMoveSliceLTLbl.lock())
            L->SetText(mMoveSliceLT ? TEXT("[v] Slice LT") : TEXT("[ ] Slice LT"));
        if (auto B2 = mMoveSliceLTBtn.lock())
        {
            // 켜지면 초록, 꺼지면 회색
            float TintR = mMoveSliceLT ? 0.10f : 0.18f;
            float TintG = mMoveSliceLT ? 0.38f : 0.18f;
            float TintB = mMoveSliceLT ? 0.10f : 0.22f;
            B2->SetTint(EWidgetState::Normal,  TintR, TintG, TintB, 1.f);
            B2->SetTint(EWidgetState::Hovered,
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
        auto& F  = mFrames[mSelectedFrame];
        constexpr float SPEED = 100.f;
        float dx = 0.f, dy = 0.f;

        if (mMoveSliceLT)
        {
            // 슬라이스 단위 이동 — 좌/우는 Size.W, 상/하는 Size.H 만큼 한 칸씩
            if (Input->GetKey(VK_LEFT,  EInputType::Press)) dx = -F.Size.x;
            if (Input->GetKey(VK_RIGHT, EInputType::Press)) dx =  F.Size.x;
            if (Input->GetKey(VK_UP,    EInputType::Press)) dy = -F.Size.y;
            if (Input->GetKey(VK_DOWN,  EInputType::Press)) dy =  F.Size.y;
        }
        else
        {
            if (Input->GetKey(VK_LEFT,  EInputType::Hold)) dx = -SPEED * DeltaTime;
            if (Input->GetKey(VK_RIGHT, EInputType::Hold)) dx =  SPEED * DeltaTime;
            if (Input->GetKey(VK_UP,    EInputType::Hold)) dy = -SPEED * DeltaTime;
            if (Input->GetKey(VK_DOWN,  EInputType::Hold)) dy =  SPEED * DeltaTime;
        }

        if (dx != 0.f || dy != 0.f)
        {
            F.Start.x = max(0.f, min(F.Start.x + dx, mTexSize.x - F.Size.x));
            F.Start.y = max(0.f, min(F.Start.y + dy, mTexSize.y - F.Size.y));
            RebuildFrameOverlays();
            UpdateInfoLabel();
            if (mOnFrameChanged) mOnFrameChanged(mSelectedFrame, F.Start, F.Size);
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
    if (auto B = mAddBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        float LocalX = Mouse.x - PanelPos.x;
        float LocalY = Mouse.y - PanelPos.y;
        FVector2 TexPos = PanelToTex({ LocalX, LocalY });
        TexPos.x = max(0.f, min(TexPos.x - 16.f, mTexSize.x - 32.f));
        TexPos.y = max(0.f, min(TexPos.y - 16.f, mTexSize.y - 32.f));
        if (mOnFrameAdded) mOnFrameAdded(TexPos, { 32.f, 32.f });
    }

    // ── 프레임 삭제 (선택된 프레임) ───────────────────────────────────────────
    if (auto B = mDelBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
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
                if (auto B = mTexBtn.lock())
                    B->SetPos(mTexLocalX, mTexLocalY);
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
            float dx = (Mouse.x - mDragMouseStart.x) / mScale;
            float dy = (Mouse.y - mDragMouseStart.y) / mScale;
            auto& F = mFrames[mSelectedFrame];

            switch (mResizeCorner)
            {
            case 0: // TL: Start 이동, Size 반대 방향
                F.Start.x = mDragDataStart.x + dx;
                F.Start.y = mDragDataStart.y + dy;
                F.Size.x  = max(4.f, mDragSizeStart.x - dx);
                F.Size.y  = max(4.f, mDragSizeStart.y - dy);
                break;
            case 1: // TR: 위쪽 Start.y 이동, 오른쪽 Size.x 증가
                F.Start.y = mDragDataStart.y + dy;
                F.Size.x  = max(4.f, mDragSizeStart.x + dx);
                F.Size.y  = max(4.f, mDragSizeStart.y - dy);
                break;
            case 2: // BL: 왼쪽 Start.x 이동, 아래쪽 Size.y 증가
                F.Start.x = mDragDataStart.x + dx;
                F.Size.x  = max(4.f, mDragSizeStart.x - dx);
                F.Size.y  = max(4.f, mDragSizeStart.y + dy);
                break;
            case 3: // BR: Size 증가
                F.Size.x = max(4.f, mDragSizeStart.x + dx);
                F.Size.y = max(4.f, mDragSizeStart.y + dy);
                break;
            }

            RebuildFrameOverlays();
            UpdateInfoLabel();
            if (mOnFrameChanged) mOnFrameChanged(mSelectedFrame, F.Start, F.Size);
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
            float dx = (Mouse.x - mDragMouseStart.x) / mScale;
            float dy = (Mouse.y - mDragMouseStart.y) / mScale;
            auto& F = mFrames[mSelectedFrame];
            F.Start.x = max(0.f, min(mDragDataStart.x + dx, mTexSize.x - F.Size.x));
            F.Start.y = max(0.f, min(mDragDataStart.y + dy, mTexSize.y - F.Size.y));

            RebuildFrameOverlays();
            UpdateInfoLabel();
            if (mOnFrameChanged) mOnFrameChanged(mSelectedFrame, F.Start, F.Size);
        }
        if (Release) mDragging = false;
    }
}

// ── Clone ─────────────────────────────────────────────────────────────────────

CSpriteViewerUI* CSpriteViewerUI::Clone()
{
    return new CSpriteViewerUI(*this);
}
