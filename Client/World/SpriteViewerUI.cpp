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

std::weak_ptr<CButton> CSpriteViewerUI::MakeBtn(const std::string& N,
    float X, float Y, float W, float H,
    float R, float G, float B, float A, int Z)
{
    auto Btn = CreateWidget<CButton>(N, Z).lock();
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

std::weak_ptr<CTextBlock> CSpriteViewerUI::MakeLbl(const std::string& N,
    float X, float Y, float W, float H, const wchar_t* Text,
    float FontSize, ETextAlignH AlignH, int Z)
{
    auto Lbl = CreateWidget<CTextBlock>(N, Z).lock();
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
        TEXT("Sprite Viewer  (drag frame box to move, drag corners to resize)"), 11.f);

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
            TEXT("×"), 13.f, ETextAlignH::Center);
    }

    // ── 줌 컨트롤 바 ──────────────────────────────────────────────────────────
    {
        float zy = TITLE_H + 3.f;
        float zx = 6.f;

        auto ZOut = CreateWidget<CButton>("SVZOut", 10).lock();
        if (ZOut) {
            ZOut->SetPos(zx, zy);
            ZOut->SetSize(24.f, 20.f);
            ZOut->SetTint(EWidgetState::Normal,  0.22f, 0.22f, 0.28f, 1.f);
            ZOut->SetTint(EWidgetState::Hovered, 0.32f, 0.32f, 0.40f, 1.f);
            ZOut->SetTint(EWidgetState::Clicked, 0.42f, 0.42f, 0.52f, 1.f);
            ZOut->SetTint(EWidgetState::Release, 0.22f, 0.22f, 0.28f, 1.f);
            ZOut->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.18f, 1.f);
            mZoomOutBtn = ZOut;
        }
        MakeLbl("SVZOutL", zx, zy, 24.f, 20.f, TEXT("−"), 12.f, ETextAlignH::Center);
        zx += 26.f;

        auto ZLbl = CreateWidget<CTextBlock>("SVZLbl", 11).lock();
        if (ZLbl) {
            ZLbl->SetPos(zx, zy);
            ZLbl->SetSize(56.f, 20.f);
            ZLbl->SetText(TEXT("100%"));
            ZLbl->SetFontSize(11.f);
            ZLbl->SetTextColor(FVector4(0.9f, 0.9f, 0.9f, 1.f));
            ZLbl->SetAlignH(ETextAlignH::Center);
            ZLbl->SetAlignV(ETextAlignV::Middle);
            mZoomLbl = ZLbl;
        }
        zx += 58.f;

        auto ZIn = CreateWidget<CButton>("SVZIn", 10).lock();
        if (ZIn) {
            ZIn->SetPos(zx, zy);
            ZIn->SetSize(24.f, 20.f);
            ZIn->SetTint(EWidgetState::Normal,  0.22f, 0.22f, 0.28f, 1.f);
            ZIn->SetTint(EWidgetState::Hovered, 0.32f, 0.32f, 0.40f, 1.f);
            ZIn->SetTint(EWidgetState::Clicked, 0.42f, 0.42f, 0.52f, 1.f);
            ZIn->SetTint(EWidgetState::Release, 0.22f, 0.22f, 0.28f, 1.f);
            ZIn->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.18f, 1.f);
            mZoomInBtn = ZIn;
        }
        MakeLbl("SVZInL", zx, zy, 24.f, 20.f, TEXT("+"), 12.f, ETextAlignH::Center);
        zx += 26.f;

        auto ZFit = CreateWidget<CButton>("SVZFit", 10).lock();
        if (ZFit) {
            ZFit->SetPos(zx, zy);
            ZFit->SetSize(40.f, 20.f);
            ZFit->SetTint(EWidgetState::Normal,  0.15f, 0.25f, 0.42f, 1.f);
            ZFit->SetTint(EWidgetState::Hovered, 0.22f, 0.35f, 0.55f, 1.f);
            ZFit->SetTint(EWidgetState::Clicked, 0.28f, 0.42f, 0.65f, 1.f);
            ZFit->SetTint(EWidgetState::Release, 0.15f, 0.25f, 0.42f, 1.f);
            ZFit->SetTint(EWidgetState::Disable, 0.10f, 0.15f, 0.25f, 1.f);
            mZoomFitBtn = ZFit;
        }
        MakeLbl("SVZFitL", zx, zy, 40.f, 20.f, TEXT("Fit"), 10.f, ETextAlignH::Center);
        zx += 48.f;

        // ── Slice LT 토글 (방향키로 프레임 Start 이동, slice=Size 단위 / 아니면 연속) ──
        auto MvBtn = CreateWidget<CButton>("SVMvSlice", 10).lock();
        if (MvBtn)
        {
            MvBtn->SetPos(zx, zy);
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
            MvLbl->SetPos(zx, zy);
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
    float iy = PANEL_H - INFO_H;
    auto InfoBg = CreateWidget<CButton>("SVInfoBg", 10).lock();
    if (InfoBg)
    {
        InfoBg->SetPos(0.f, iy);
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
        InfoLbl->SetPos(6.f, iy + 2.f);
        InfoLbl->SetSize(PANEL_W - 116.f, INFO_H - 4.f);
        InfoLbl->SetText(TEXT("텍스처를 설정한 뒤 Open Sprite Viewer를 클릭하세요."));
        InfoLbl->SetFontSize(10.f);
        InfoLbl->SetTextColor(FVector4(0.85f, 0.85f, 0.92f, 1.f));
        InfoLbl->SetAlignV(ETextAlignV::Middle);
        mInfoLbl = InfoLbl;
    }

    // [+ Add] 버튼 (커서 위치에 32×32 프레임 추가)
    float bx = PANEL_W - 108.f;
    auto AddBtn = CreateWidget<CButton>("SVAdd", 10).lock();
    if (AddBtn)
    {
        AddBtn->SetPos(bx, iy + 14.f);
        AddBtn->SetSize(48.f, 22.f);
        AddBtn->SetTint(EWidgetState::Normal,  0.14f, 0.34f, 0.14f, 1.f);
        AddBtn->SetTint(EWidgetState::Hovered, 0.20f, 0.46f, 0.20f, 1.f);
        AddBtn->SetTint(EWidgetState::Clicked, 0.26f, 0.56f, 0.26f, 1.f);
        AddBtn->SetTint(EWidgetState::Release, 0.14f, 0.34f, 0.14f, 1.f);
        AddBtn->SetTint(EWidgetState::Disable, 0.10f, 0.20f, 0.10f, 1.f);
        mAddBtn = AddBtn;
    }
    MakeLbl("SVAddL", bx, iy + 14.f, 48.f, 22.f, TEXT("+ Add"), 10.f, ETextAlignH::Center);
    bx += 52.f;

    // [- Del] 버튼 (선택된 프레임 삭제)
    auto DelBtn = CreateWidget<CButton>("SVDel", 10).lock();
    if (DelBtn)
    {
        DelBtn->SetPos(bx, iy + 14.f);
        DelBtn->SetSize(48.f, 22.f);
        DelBtn->SetTint(EWidgetState::Normal,  0.34f, 0.14f, 0.14f, 1.f);
        DelBtn->SetTint(EWidgetState::Hovered, 0.46f, 0.20f, 0.20f, 1.f);
        DelBtn->SetTint(EWidgetState::Clicked, 0.56f, 0.26f, 0.26f, 1.f);
        DelBtn->SetTint(EWidgetState::Release, 0.34f, 0.14f, 0.14f, 1.f);
        DelBtn->SetTint(EWidgetState::Disable, 0.20f, 0.10f, 0.10f, 1.f);
        mDelBtn = DelBtn;
    }
    MakeLbl("SVDelL", bx, iy + 14.f, 48.f, 22.f, TEXT("- Del"), 10.f, ETextAlignH::Center);

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

// ── 내부 계산 ─────────────────────────────────────────────────────────────────

void CSpriteViewerUI::FitToCanvas()
{
    float scaleX = PANEL_W / mTexSize.x;
    float scaleY = CANVAS_H / mTexSize.y;
    mScale = min(scaleX, scaleY);
    mScale = max(0.05f, mScale);

    float tw = mTexSize.x * mScale;
    float th = mTexSize.y * mScale;
    mTexLocalX = (PANEL_W - tw) * 0.5f;
    mTexLocalY = CANVAS_Y + (CANVAS_H - th) * 0.5f;

    if (auto B = mTexBtn.lock())
    {
        B->SetPos(mTexLocalX, mTexLocalY);
        B->SetSize(tw, th);
    }
    UpdateZoomLabel();
}

void CSpriteViewerUI::ApplyZoom(float NewScale)
{
    mScale = max(0.05f, min(NewScale, 12.f));

    float tw = mTexSize.x * mScale;
    float th = mTexSize.y * mScale;
    // 캔버스 중앙 기준 배치 (이미지가 캔버스보다 작을 때)
    mTexLocalX = max(0.f, (PANEL_W - tw) * 0.5f);
    mTexLocalY = CANVAS_Y + max(0.f, (CANVAS_H - th) * 0.5f);

    if (auto B = mTexBtn.lock())
    {
        B->SetPos(mTexLocalX, mTexLocalY);
        B->SetSize(tw, th);
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
            L->SetText(TEXT("프레임 없음  |  [+ Add]: 커서 위치에 32×32 프레임 추가"));
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
            swprintf_s(Buf, 160, L"[%d/%d]  Start:(%.0f,%.0f)  Size:(%.0f×%.0f)  Pivot.X:%.1f  |  드래그=이동  모서리=리사이즈  청선=피벗",
                mSelectedFrame + 1, (int)mFrames.size(),
                F.Start.x, F.Start.y, F.Size.x, F.Size.y, mPivotX);
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

bool CSpriteViewerUI::HitTestFrame(int fi, FVector2 mouseScreen) const
{
    if (fi < 0 || fi >= (int)mFrames.size()) return false;
    FVector3 pp = GetPos();
    float lx = mouseScreen.x - pp.x;
    float ly = mouseScreen.y - pp.y;
    float fx = FrameDispX(fi), fy = FrameDispY(fi);
    float fw = FrameDispW(fi), fh = FrameDispH(fi);
    return lx >= fx && lx < fx + fw && ly >= fy && ly < fy + fh;
}

bool CSpriteViewerUI::HitTestHandle(int c, FVector2 mouseScreen) const
{
    if (c < 0 || c >= 4 || !mHandleHitRects[c].valid) return false;
    FVector3 pp = GetPos();
    float lx = mouseScreen.x - pp.x;
    float ly = mouseScreen.y - pp.y;
    const auto& R = mHandleHitRects[c];
    return lx >= R.x && lx < R.x + R.w && ly >= R.y && ly < R.y + R.h;
}

bool CSpriteViewerUI::IsInCanvas(FVector2 mouseScreen) const
{
    FVector3 pp = GetPos();
    float lx = mouseScreen.x - pp.x;
    float ly = mouseScreen.y - pp.y;
    return lx >= 0.f && lx < PANEL_W && ly >= CANVAS_Y && ly < CANVAS_Y + CANVAS_H;
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
    mFrameBtns.clear();
    for (auto& R : mHandleHitRects) R.valid = false;

    // 캔버스 유효 범위 (패널 로컬 좌표)
    const float CX0 = 0.f,      CX1 = PANEL_W;
    const float CY0 = CANVAS_Y, CY1 = CANVAS_Y + CANVAS_H;
    const float BT  = 2.f; // 테두리 두께(px)

    // 프레임 아웃라인 생성 (ZOrder 5 → 캔버스/텍스처 위)
    // 클리핑된 rect의 외곽선이 아닌, 실제 프레임 엣지 4개를 개별 클리핑해서 그림
    // → 프레임이 캔버스 밖으로 나가도 경계선에 가짜 바가 생기지 않음
    for (int i = 0; i < (int)mFrames.size(); i++)
    {
        float fx = FrameDispX(i), fy = FrameDispY(i);
        float fw = FrameDispW(i), fh = FrameDispH(i);
        bool  sel = (i == mSelectedFrame);

        // 캔버스와 완전히 겹치지 않으면 건너뜀
        if (fx + fw <= CX0 || fx >= CX1 || fy + fh <= CY0 || fy >= CY1)
        {
            mFrameBtns.push_back({});
            continue;
        }

        float r = sel ? 1.00f : 0.30f;
        float g = sel ? 0.90f : 0.85f;
        float b = sel ? 0.10f : 0.30f;

        auto SetAll = [&](std::shared_ptr<CButton> Btn)
        {
            Btn->SetTint(EWidgetState::Normal,  r, g, b, 1.f);
            Btn->SetTint(EWidgetState::Hovered, r, g, b, 1.f);
            Btn->SetTint(EWidgetState::Clicked, r, g, b, 1.f);
            Btn->SetTint(EWidgetState::Release, r, g, b, 1.f);
            Btn->SetTint(EWidgetState::Disable, r, g, b, 1.f);
        };

        string P = "SVFr" + to_string(i);

        // Top 바 — 실제 프레임 상단(fy)이 캔버스 안에 있을 때만 그림
        if (fy >= CY0 && fy + BT <= CY1)
        {
            float ex0 = max(fx, CX0), ex1 = min(fx + fw, CX1);
            if (ex1 > ex0)
                if (auto B = CreateWidget<CButton>(P + "T", 5).lock())
                { B->SetPos(ex0, fy); B->SetSize(ex1 - ex0, BT); SetAll(B); }
        }

        // Bottom 바 — 실제 프레임 하단(fy+fh)이 캔버스 안에 있을 때만 그림
        if (fy + fh - BT >= CY0 && fy + fh <= CY1)
        {
            float ex0 = max(fx, CX0), ex1 = min(fx + fw, CX1);
            if (ex1 > ex0)
                if (auto B = CreateWidget<CButton>(P + "B", 5).lock())
                { B->SetPos(ex0, fy + fh - BT); B->SetSize(ex1 - ex0, BT); SetAll(B); }
        }

        // Left 바 — 실제 프레임 좌측(fx)이 캔버스 안에 있을 때만, Y는 캔버스 범위 클리핑
        if (fx >= CX0 && fx + BT <= CX1)
        {
            float ey0 = max(fy + BT, CY0), ey1 = min(fy + fh - BT, CY1);
            if (ey1 > ey0)
                if (auto B = CreateWidget<CButton>(P + "L", 5).lock())
                { B->SetPos(fx, ey0); B->SetSize(BT, ey1 - ey0); SetAll(B); }
        }

        // Right 바 — 실제 프레임 우측(fx+fw)이 캔버스 안에 있을 때만, Y는 캔버스 범위 클리핑
        if (fx + fw - BT >= CX0 && fx + fw <= CX1)
        {
            float ey0 = max(fy + BT, CY0), ey1 = min(fy + fh - BT, CY1);
            if (ey1 > ey0)
                if (auto B = CreateWidget<CButton>(P + "R", 5).lock())
                { B->SetPos(fx + fw - BT, ey0); B->SetSize(BT, ey1 - ey0); SetAll(B); }
        }

        mFrameBtns.push_back({});
    }

    // 선택된 프레임의 리사이즈 핸들 (4 모서리, ZOrder 8)
    if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
    {
        float fx = FrameDispX(mSelectedFrame);
        float fy = FrameDispY(mSelectedFrame);
        float fw = FrameDispW(mSelectedFrame);
        float fh = FrameDispH(mSelectedFrame);
        constexpr float HS = 10.f;

        // TL, TR, BL, BR 순서
        float hx[4] = { fx - HS * 0.5f, fx + fw - HS * 0.5f, fx - HS * 0.5f, fx + fw - HS * 0.5f };
        float hy[4] = { fy - HS * 0.5f, fy - HS * 0.5f, fy + fh - HS * 0.5f, fy + fh - HS * 0.5f };

        for (int c = 0; c < 4; c++)
        {
            // 히트 영역은 항상 저장 (캔버스 밖이어도 드래그 가능하게)
            mHandleHitRects[c] = { hx[c], hy[c], HS, HS, true };

            // 캔버스와 전혀 안 겹치면 시각 생략
            if (hx[c] + HS <= CX0 || hx[c] >= CX1 || hy[c] + HS <= CY0 || hy[c] >= CY1)
                continue;

            // 캔버스 범위로 클리핑 후 아웃라인 그리기
            float hcx0 = max(hx[c], CX0), hcy0 = max(hy[c], CY0);
            float hcw  = min(hx[c] + HS, CX1) - hcx0;
            float hch  = min(hy[c] + HS, CY1) - hcy0;
            if (hcw > 0.f && hch > 0.f)
                MakeRectBorder("SVHa" + to_string(c), hcx0, hcy0, hcw, hch, 2.f,
                    1.f, 1.f, 0.f, 1.f, 8);
        }
    }

    // 피벗 세로선 — 선택 프레임의 Start.x 기준 mPivotX 픽셀 위치에 그림
    // 모든 프레임에서 동일한 mPivotX 값을 사용 (애니메이션 공통 pivot)
    if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
    {
        float pivX = FrameDispX(mSelectedFrame) + mPivotX * mScale;
        if (pivX >= CX0 && pivX < CX1)
        {
            if (auto B = CreateWidget<CButton>("SVPiv", 9).lock())
            {
                B->SetPos(max(pivX - 1.f, CX0), CY0);
                B->SetSize(2.f, CY1 - CY0);
                B->SetTint(EWidgetState::Normal,  0.f, 1.f, 1.f, 0.55f);
                B->SetTint(EWidgetState::Hovered, 0.f, 1.f, 1.f, 0.85f);
                B->SetTint(EWidgetState::Clicked, 1.f, 1.f, 0.f, 1.f);
                B->SetTint(EWidgetState::Release, 0.f, 1.f, 1.f, 0.55f);
                B->SetTint(EWidgetState::Disable, 0.f, 1.f, 1.f, 0.55f);
            }
        }
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

    FVector2 mouse   = Input->GetMousePos();
    bool press   = Input->GetMouseState(EMouseType::LButton, EInputType::Press);
    bool held    = Input->GetMouseState(EMouseType::LButton, EInputType::Hold);
    bool release = Input->GetMouseState(EMouseType::LButton, EInputType::Release);

    FVector3 pp = GetPos();

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
            float r = mMoveSliceLT ? 0.10f : 0.18f;
            float g = mMoveSliceLT ? 0.38f : 0.18f;
            float b = mMoveSliceLT ? 0.10f : 0.22f;
            B2->SetTint(EWidgetState::Normal,  r,  g,  b,  1.f);
            B2->SetTint(EWidgetState::Hovered, min(r+.10f,1.f), min(g+.10f,1.f), min(b+.10f,1.f), 1.f);
        }
    }

    // ── 방향키 등록 (최초 1회) ────────────────────────────────────────────────
    if (!mArrowKeysRegistered)
    {
        Input->AddBindKey("SVLeft",  DIK_LEFT);
        Input->AddBindKey("SVRight", DIK_RIGHT);
        Input->AddBindKey("SVUp",    DIK_UP);
        Input->AddBindKey("SVDown",  DIK_DOWN);
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
            if (Input->GetKey(DIK_LEFT,  EInputType::Press)) dx = -F.Size.x;
            if (Input->GetKey(DIK_RIGHT, EInputType::Press)) dx =  F.Size.x;
            if (Input->GetKey(DIK_UP,    EInputType::Press)) dy = -F.Size.y;
            if (Input->GetKey(DIK_DOWN,  EInputType::Press)) dy =  F.Size.y;
        }
        else
        {
            if (Input->GetKey(DIK_LEFT,  EInputType::Hold)) dx = -SPEED * DeltaTime;
            if (Input->GetKey(DIK_RIGHT, EInputType::Hold)) dx =  SPEED * DeltaTime;
            if (Input->GetKey(DIK_UP,    EInputType::Hold)) dy = -SPEED * DeltaTime;
            if (Input->GetKey(DIK_DOWN,  EInputType::Hold)) dy =  SPEED * DeltaTime;
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

    // ── 마우스 휠 줌 (캔버스 위에서: 위=확대, 아래=축소) ─────────────────────
    {
        float Wheel = Input->GetMouseWheelDelta();
        if (Wheel != 0.f && IsInCanvas(mouse))
        {
            if (Wheel > 0.f) ApplyZoom(mScale * 1.25f);
            else             ApplyZoom(mScale * 0.80f);
        }
    }

    // ── 프레임 추가 (커서 위치) ────────────────────────────────────────────────
    if (auto B = mAddBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        float lx = mouse.x - pp.x;
        float ly = mouse.y - pp.y;
        FVector2 tp = PanelToTex({ lx, ly });
        tp.x = max(0.f, min(tp.x - 16.f, mTexSize.x - 32.f));
        tp.y = max(0.f, min(tp.y - 16.f, mTexSize.y - 32.f));
        if (mOnFrameAdded) mOnFrameAdded(tp, { 32.f, 32.f });
    }

    // ── 프레임 삭제 (선택된 프레임) ───────────────────────────────────────────
    if (auto B = mDelBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mSelectedFrame >= 0 && mSelectedFrame < (int)mFrames.size())
        {
            int fi = mSelectedFrame;
            mSelectedFrame = -1;
            if (mOnFrameDeleted) mOnFrameDeleted(fi);
        }
    }

    // ── 패널 드래그 (타이틀 바) ────────────────────────────────────────────────
    bool inTitle = (mouse.x >= pp.x && mouse.x < pp.x + PANEL_W &&
                    mouse.y >= pp.y && mouse.y < pp.y + TITLE_H);

    if (press && inTitle && !mDragging && !mResizing)
    {
        mPanelDragging  = true;
        mDragMouseStart = mouse;
        mPanelPosStart  = { pp.x, pp.y };
    }
    if (mPanelDragging)
    {
        if (held)
        {
            SetPos(mPanelPosStart.x + (mouse.x - mDragMouseStart.x),
                   mPanelPosStart.y + (mouse.y - mDragMouseStart.y));
            RebuildFrameOverlays();
        }
        if (release) mPanelDragging = false;
        return;
    }

    // ── 캔버스 패닝 (우클릭 드래그: 좌=이미지 우, 우=이미지 좌) ──────────────
    {
        bool rpress   = Input->GetMouseState(EMouseType::RButton, EInputType::Press);
        bool rheld    = Input->GetMouseState(EMouseType::RButton, EInputType::Hold);
        bool rrelease = Input->GetMouseState(EMouseType::RButton, EInputType::Release);

        if (rpress && IsInCanvas(mouse) && !mCanvasPanning && !mResizing && !mDragging)
        {
            mCanvasPanning  = true;
            mPanMouseStart  = mouse;
            mPanOriginStart = { mTexLocalX, mTexLocalY };
        }
        if (mCanvasPanning)
        {
            if (rheld)
            {
                mTexLocalX = mPanOriginStart.x + (mouse.x - mPanMouseStart.x);
                mTexLocalY = mPanOriginStart.y + (mouse.y - mPanMouseStart.y);
                if (auto B = mTexBtn.lock())
                    B->SetPos(mTexLocalX, mTexLocalY);
                RebuildFrameOverlays();
            }
            if (rrelease) mCanvasPanning = false;
            return;
        }
    }

    // ── 피벗 세로선 드래그 ────────────────────────────────────────────────────
    if (press && !mDragging && !mResizing && !mCanvasPanning && mSelectedFrame >= 0)
    {
        float pivScreenX = pp.x + FrameDispX(mSelectedFrame) + mPivotX * mScale;
        if (std::abs(mouse.x - pivScreenX) <= 4.f && IsInCanvas(mouse))
            mPivotDragging = true;
    }
    if (mPivotDragging)
    {
        if (held || press)
        {
            if (mSelectedFrame >= 0 && mScale > 0.f)
            {
                mPivotX = (mouse.x - pp.x - FrameDispX(mSelectedFrame)) / mScale;
                RebuildFrameOverlays();
                UpdateInfoLabel();
            }
        }
        if (release) { mPivotDragging = false; RebuildFrameOverlays(); }
        return;
    }

    // ── 리사이즈 핸들 드래그 ──────────────────────────────────────────────────
    if (press && !mDragging && mSelectedFrame >= 0)
    {
        for (int c = 0; c < 4; c++)
        {
            if (HitTestHandle(c, mouse))
            {
                mResizing       = true;
                mResizeCorner   = c;
                mDragMouseStart = mouse;
                mDragDataStart  = mFrames[mSelectedFrame].Start;
                mDragSizeStart  = mFrames[mSelectedFrame].Size;
                break;
            }
        }
    }

    if (mResizing)
    {
        if (held)
        {
            float dx = (mouse.x - mDragMouseStart.x) / mScale;
            float dy = (mouse.y - mDragMouseStart.y) / mScale;
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
        if (release) mResizing = false;
        return;
    }

    // ── 프레임 박스 드래그 (선택 + 이동) ─────────────────────────────────────
    if (press && IsInCanvas(mouse))
    {
        bool hitAny = false;

        // 상위(나중에 그려진) 프레임부터 히트 테스트
        for (int i = (int)mFrames.size() - 1; i >= 0; i--)
        {
            if (HitTestFrame(i, mouse))
            {
                hitAny = true;
                if (i != mSelectedFrame)
                {
                    mSelectedFrame = i;
                    RebuildFrameOverlays();
                    UpdateInfoLabel();
                    if (mOnFrameSelected) mOnFrameSelected(i);
                }
                mDragging       = true;
                mDragMouseStart = mouse;
                mDragDataStart  = mFrames[i].Start;
                break;
            }
        }

        if (!hitAny && mSelectedFrame >= 0)
        {
            mSelectedFrame = -1;
            RebuildFrameOverlays();
            UpdateInfoLabel();
        }
    }

    if (mDragging)
    {
        if (held)
        {
            float dx = (mouse.x - mDragMouseStart.x) / mScale;
            float dy = (mouse.y - mDragMouseStart.y) / mScale;
            auto& F = mFrames[mSelectedFrame];
            F.Start.x = max(0.f, min(mDragDataStart.x + dx, mTexSize.x - F.Size.x));
            F.Start.y = max(0.f, min(mDragDataStart.y + dy, mTexSize.y - F.Size.y));

            RebuildFrameOverlays();
            UpdateInfoLabel();
            if (mOnFrameChanged) mOnFrameChanged(mSelectedFrame, F.Start, F.Size);
        }
        if (release) mDragging = false;
    }
}

// ── Clone ─────────────────────────────────────────────────────────────────────

CSpriteViewerUI* CSpriteViewerUI::Clone()
{
    return new CSpriteViewerUI(*this);
}
