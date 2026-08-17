#pragma once

#include "World/WidgetContainer.h"
#include "World/TextBlock.h"
#include <functional>
#include <vector>
#include <string>
#include <array>

// 스프라이트 시트를 시각적으로 보면서 프레임 박스를 드래그/리사이즈할 수 있는 뷰어 패널
class CSpriteViewerUI : public CWidgetContainer
{
public:
    struct FFrameRect
    {
        FVector2 Start = { 0.f, 0.f };
        FVector2 Size  = { 32.f, 32.f };
    };

private:
    static constexpr float PANEL_W  = 700.f;
    static constexpr float PANEL_H  = 540.f;
    static constexpr float PANEL_X  = 490.f;
    static constexpr float PANEL_Y  = 40.f;
    static constexpr float TITLE_H  = 28.f;
    static constexpr float CTRL_H   = 26.f;
    static constexpr float INFO_H   = 52.f;
    static constexpr float CANVAS_Y = TITLE_H + CTRL_H;
    static constexpr float CANVAS_H = PANEL_H - TITLE_H - CTRL_H - INFO_H;

    // 텍스처 정보
    std::string mTexName;
    FVector2    mTexSize   = { 1.f, 1.f };
    float       mScale     = 1.f;
    float       mTexLocalX = 0.f;   // 패널 로컬 좌표 (텍스처 표시 영역 좌상단)
    float       mTexLocalY = CANVAS_Y;

    // 프레임 데이터 미러
    std::vector<FFrameRect> mFrames;
    int                     mSelectedFrame = -1;

    // 드래그/리사이즈 상태
    bool     mPanelDragging = false;
    bool     mDragging      = false;
    bool     mResizing      = false;
    int      mResizeCorner  = -1;   // 0=TL 1=TR 2=BL 3=BR

    // 피벗 기준선 (픽셀, 선택 프레임의 Start 기준 상대 좌표)
    // X는 캔버스를 세로로 가르는 선, Y는 가로로 가르는 선.
    // 값의 주인은 시퀀스(FSeqData)이고 여기 있는 건 사본이다.
    // 바뀌면 mOnPivotChanged로 애님 에디터에 돌려준다.
    float    mPivotX          = 0.f;
    float    mPivotY          = 0.f;
    bool     mPivotXDragging  = false;
    bool     mPivotYDragging  = false;

    // 캔버스 패닝 (우클릭 드래그)
    bool     mCanvasPanning  = false;
    FVector2 mPanMouseStart  = {};
    FVector2 mPanOriginStart = {};  // 패닝 시작 시점의 {mTexLocalX, mTexLocalY}

    // 방향키 LT 이동
    bool mMoveSliceLT        = false;
    bool mArrowKeysRegistered = false;
    std::weak_ptr<class CButton>    mMoveSliceLTBtn;
    std::weak_ptr<class CTextBlock> mMoveSliceLTLbl;

    FVector2 mPanelPosStart;
    FVector2 mDragMouseStart;
    FVector2 mDragDataStart;    // Start (드래그) or Start (리사이즈 기준)
    FVector2 mDragSizeStart;    // Size (리사이즈 기준)

    int mStaticChildCount = 0;

    // 정적 위젯 핸들
    std::weak_ptr<class CButton>    mBgBtn;
    std::weak_ptr<class CButton>    mTexBtn;    // 스프라이트 시트 표시 버튼
    std::weak_ptr<class CButton>    mCloseBtn;
    std::weak_ptr<class CTextBlock> mZoomLbl;
    std::weak_ptr<class CButton>    mZoomInBtn, mZoomOutBtn, mZoomFitBtn;
    std::weak_ptr<class CTextBlock> mInfoLbl;
    std::weak_ptr<class CButton>    mAddBtn, mDelBtn;

    // 핸들 히트 영역 (위젯 경계 대신 수치로 보관 → 아웃라인 표현과 분리)
    struct FHitRect { float x = 0, y = 0, w = 0, h = 0; bool valid = false; };
    std::array<FHitRect, 4> mHandleHitRects;

    // 콜백
    std::function<void(int, FVector2, FVector2)> mOnFrameChanged;
    std::function<void(int)>                     mOnFrameSelected;
    std::function<void(int)>                     mOnFrameDeleted;
    std::function<void(FVector2, FVector2)>      mOnFrameAdded;
    std::function<void(float, float)>            mOnPivotChanged;

public:
    CSpriteViewerUI();
    CSpriteViewerUI(const CSpriteViewerUI& src);
    virtual ~CSpriteViewerUI();

    // TexName: AssetManager에 등록된 텍스처 이름, TexSize: 픽셀 크기
    void ShowTexture(const std::string& TexName, FVector2 TexSize);
    void SetFrames(const std::vector<FFrameRect>& Frames, int Selected = -1);
    void SelectFrame(int fi);

    // 시퀀스가 들고 있는 피벗 값을 뷰어에 밀어넣는다. (시퀀스를 바꿀 때마다 호출)
    void SetPivot(float PivotX, float PivotY);

    float GetPivotX() const { return mPivotX; }
    float GetPivotY() const { return mPivotY; }

    void SetOnFrameChanged (std::function<void(int, FVector2, FVector2)> Fn) { mOnFrameChanged  = Fn; }
    void SetOnFrameSelected(std::function<void(int)> Fn)                     { mOnFrameSelected = Fn; }
    void SetOnFrameDeleted (std::function<void(int)> Fn)                     { mOnFrameDeleted  = Fn; }
    void SetOnFrameAdded   (std::function<void(FVector2, FVector2)> Fn)      { mOnFrameAdded    = Fn; }
    void SetOnPivotChanged (std::function<void(float, float)> Fn)            { mOnPivotChanged  = Fn; }

    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;
    virtual CSpriteViewerUI* Clone() override;

private:
    void RebuildFrameOverlays();
    void FitToCanvas();
    void ApplyZoom(float NewScale);
    void UpdateZoomLabel();
    void UpdateInfoLabel();

    // 패널 로컬 좌표 → 텍스처 픽셀 좌표 (FrameDisp*의 역변환)
    FVector2 PanelToTex(FVector2 PanelLocalPos) const;

    // 텍스처 픽셀 좌표 → 패널 로컬 좌표. fi번 프레임 박스가 화면에서 차지하는 사각형.
    float FrameDispX(int fi) const;
    float FrameDispY(int fi) const;
    float FrameDispW(int fi) const;
    float FrameDispH(int fi) const;

    // 스크린 좌표 기준 히트 테스트
    bool HitTestFrame (int fi,     FVector2 mouseScreen) const;
    bool HitTestHandle(int Corner, FVector2 mouseScreen) const; // 0=TL 1=TR 2=BL 3=BR
    bool IsInCanvas   (FVector2 mouseScreen) const;

    std::weak_ptr<class CButton>    MakeBtn(const std::string& Name,
        float X, float Y, float W, float H,
        float R, float G, float B, float A = 1.f, int Z = 10);
    std::weak_ptr<class CTextBlock> MakeLbl(const std::string& Name,
        float X, float Y, float W, float H, const wchar_t* Text,
        float FontSize = 12.f, ETextAlignH AlignH = ETextAlignH::Left, int Z = 11);

    // 아웃라인 사각형: 두께 T인 4개 얇은 바로 구성
    void MakeRectBorder(const std::string& Prefix,
        float X, float Y, float W, float H, float T,
        float R, float G, float B, float A, int Z);
};
