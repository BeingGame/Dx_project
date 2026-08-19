#include "AnimEditorUI.h"
#include "SpriteViewerUI.h"

#include "World/Button.h"
#include "World/TextBlock.h"
#include "World/TitleBar.h"
#include "World/Actor.h"
#include "World/Animation2DComponent.h"
#include "World/Input.h"
#include "World/World.h"
#include "Asset/AssetManager.h"
#include "Asset/AnimationManager.h"
#include "Asset/Texture.h"
#include "LogManager.h"
#include "DialogUtil.h"

#include <fstream>
#include <sstream>
#include <cstdio>   // sprintf_s (값 직접 입력 버퍼 초기화)

namespace
{
    // CTextureManager는 텍스처를 "Texture_" 접두사를 붙인 키로 보관하고,
    // CTexture::GetName()도 그 접두사가 붙은 내부 이름을 돌려준다.
    // 그런데 FindTexture()는 받은 이름에 접두사를 "다시" 붙여서 찾는다.
    //
    //   LoadTexture("프리스트_낫")  → 맵 키 "Texture_프리스트_낫", GetName() = "Texture_프리스트_낫"
    //   FindTexture("프리스트_낫")        → "Texture_프리스트_낫"        ✓
    //   FindTexture(Texture->GetName())       → "Texture_Texture_프리스트_낫" ✗
    //
    // 그래서 GetName() 결과를 조회용 이름으로 쓰려면 접두사를 떼야 한다.
    // .anim2d 파일이 어떤 AnimName을 담고 있는지만 훑어본다.
    // 파일이 없거나 형식이 아니면 빈 문자열.
    std::string PeekAnimName(const std::string& Path)
    {
        std::ifstream File(Path);

        if (!File)
        {
            return {};
        }

        std::string Line;

        while (std::getline(File, Line))
        {
            std::istringstream LineStream(Line);
            std::string Key;
            LineStream >> Key;

            if (Key == "AnimName")
            {
                std::string Name;
                LineStream >> Name;
                return Name;
            }
        }

        return {};
    }

    // 같은 파일을 가리키는 경로인지. (윈도우라 대소문자는 무시한다)
    bool IsSamePath(const std::string& PathA, const std::string& PathB)
    {
        return !PathA.empty() && !PathB.empty() && _stricmp(PathA.c_str(), PathB.c_str()) == 0;
    }

    std::string StripTexturePrefix(const std::string& Name)
    {
        static const std::string Prefix = "Texture_";

        if (Name.size() > Prefix.size() && Name.compare(0, Prefix.size(), Prefix) == 0)
            return Name.substr(Prefix.size());

        return Name;
    }
}

CAnimEditorUI::CAnimEditorUI()
{}

CAnimEditorUI::CAnimEditorUI(const CAnimEditorUI& src)
    : CWidgetContainer(src)
{}

CAnimEditorUI::~CAnimEditorUI()
{}

// ── 위젯 헬퍼 ────────────────────────────────────────────────────────────────
//
// 이 패널의 위젯 이름 규칙: "역할 + 종류 + _번호"  (예: FramePropValueLabel_37)
//   · 종류는 Button / Label 둘 중 하나로 끝난다.

// CButton은 사각형만, CTextBlock은 글자만 그린다.
// 그래서 "글자 있는 버튼"은 같은 자리에 버튼(ZOrder 3) + 라벨(ZOrder 4)을 겹쳐 만든다.
// CTextBlock::CollisionMouse가 항상 false라서 위에 덮인 라벨은 클릭을 가로채지 않는다.

// 버튼 하나를 만든다.
// 넘긴 색이 기본색이고, 호버/클릭 색은 여기서 +0.15 / +0.25로 자동 계산한다.
std::weak_ptr<CButton> CAnimEditorUI::MakeButton(const std::string& Name,
    float X, float Y, float Width, float Height, float Red, float Green, float Blue)
{
    auto Button = CreateWidget<CButton>(Name, 3).lock();
    if (Button)
    {
        Button->SetPos(X, Y);
        Button->SetSize(Width, Height);
        float HoverRed = min(Red + .15f, 1.f), HoverGreen = min(Green + .15f, 1.f), HoverBlue = min(Blue + .15f, 1.f);
        float ClickRed = min(Red + .25f, 1.f), ClickGreen = min(Green + .25f, 1.f), ClickBlue = min(Blue + .25f, 1.f);
        Button->SetTint(EWidgetState::Normal,  Red,  Green,  Blue,  1.f);
        Button->SetTint(EWidgetState::Hovered, HoverRed, HoverGreen, HoverBlue, 1.f);
        Button->SetTint(EWidgetState::Clicked, ClickRed, ClickGreen, ClickBlue, 1.f);
        Button->SetTint(EWidgetState::Release, HoverRed, HoverGreen, HoverBlue, 1.f);
        Button->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.15f, 0.5f);
    }
    return Button;
}

// 글자 라벨 하나를 만든다.
// 글자색은 회백색으로 고정이므로, 다른 색이 필요한 값 라벨은
// 이 함수를 쓰지 않고 CreateWidget<CTextBlock>을 직접 부른다.
std::weak_ptr<CTextBlock> CAnimEditorUI::MakeLabel(const std::string& Name,
    float X, float Y, float Width, float Height, const wchar_t* Text,
    float FontSize, ETextAlignH AlignH)
{
    auto Label = CreateWidget<CTextBlock>(Name, 4).lock();
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

// ── Prop Row (시퀀스 속성) ────────────────────────────────────────────────────
//
// 숫자 한 개를 편집하는 행을 만든다. 위젯 7개로 구성된다.
//
//   6         80  104                        W-34
//   | PlayTime |−|          1.00              | + |
//                ↑  ↑       ↑                   ↑
//        MinusButton  ValueButton(판)  PlusButton
//        +MinusLabel  +ValueLabel(숫자) +PlusLabel
//
//   Label       — 속성 이름
//   MinusButton — Step만큼 빼는 버튼 / MinusLabel은 그 위의 "−" 글자
//   ValueButton — 값 영역 배경. 더블클릭 감지를 받으려고 깔아둔 판이다.
//   ValueLabel  — 현재 숫자. 편집 중에는 입력 버퍼 + 캐럿을 보여준다.
//   PlusButton  — Step만큼 더하는 버튼 / PlusLabel은 그 위의 "+" 글자
//
// Y     : 참조로 받아서 행 높이만큼 밀어준다. 호출부는 Y를 계속 재사용하면 된다.
// Step  : +/- 버튼 한 번에 움직일 양
// Getter   : 표시할 값을 읽어오는 함수
// Setter   : 바뀐 값을 써넣는 함수 (범위 제한도 여기서 한다)
//
// Getter/Setter를 콜백으로 받는 이유는, 이 행이 어떤 데이터를 편집하는지 몰라도 되게
// 하기 위해서다. Update()는 mPropButtons를 돌면서 Getter/Setter만 호출한다.
void CAnimEditorUI::AddPropRow(float& Y, const wchar_t* Label, float Step,
    std::function<float()> Getter, std::function<void(float)> Setter)
{
    int WidgetIdx = mDynIdx++;
    float PanelW = GetSize().x;

    MakeLabel("AnimPropLabel_" + std::to_string(WidgetIdx), 6.f, Y, 72.f, ROW_H, Label, 11.f);

    auto MinButton = MakeButton("AnimPropMinusButton_" + std::to_string(WidgetIdx), 80.f, Y + 1.f, 22.f, ROW_H - 2.f, 0.22f, 0.22f, 0.28f);
    MakeLabel("AnimPropMinusLabel_" + std::to_string(WidgetIdx), 80.f, Y + 1.f, 22.f, ROW_H - 2.f, TEXT("-"), 12.f, ETextAlignH::Center);

    // 값 표시 배경 — 더블클릭하면 직접 입력 모드로 들어간다.
    // (CTextBlock은 마우스를 먹지 않으므로 위에 겹쳐 있어도 이 버튼이 클릭을 받는다)
    auto ValButton = MakeButton("AnimPropValueButton_" + std::to_string(WidgetIdx), 104.f, Y + 1.f, PanelW - 140.f, ROW_H - 2.f,
        0.15f, 0.15f, 0.19f);

    TCHAR ValueBuffer[32];
    swprintf_s(ValueBuffer, 32, L"%.2f", Getter ? Getter() : 0.f);
    auto ValLabel = CreateWidget<CTextBlock>("AnimPropValueLabel_" + std::to_string(WidgetIdx), 4).lock();
    if (ValLabel)
    {
        ValLabel->SetPos(104.f, Y);
        ValLabel->SetSize(PanelW - 140.f, ROW_H);
        ValLabel->SetText(ValueBuffer);
        ValLabel->SetFontSize(11.f);
        ValLabel->SetTextColor(FVector4(0.9f, 0.95f, 1.f, 1.f));
        ValLabel->SetAlignH(ETextAlignH::Center);
        ValLabel->SetAlignV(ETextAlignV::Middle);
    }

    auto PlusButton = MakeButton("AnimPropPlusButton_" + std::to_string(WidgetIdx), PanelW - 34.f, Y + 1.f, 22.f, ROW_H - 2.f, 0.22f, 0.22f, 0.28f);
    MakeLabel("AnimPropPlusLabel_" + std::to_string(WidgetIdx), PanelW - 34.f, Y + 1.f, 22.f, ROW_H - 2.f, TEXT("+"), 12.f, ETextAlignH::Center);

    mPropButtons.push_back({ MinButton, PlusButton, ValLabel, ValButton, Step, 2, Getter, Setter });
    Y += ROW_H + 2.f;
}

// ── Frame Prop Row (프레임 속성, mFramePropButtons) ───────────────────────────────
//
// 구조는 AddPropRow와 완전히 같고 다른 점은 세 가지뿐이다.
//   1. mPropButtons가 아니라 mFramePropButtons에 담긴다.
//      값이 바뀌면 Update()가 ApplyFrames()로 CAnimation2D에 즉시 반영하고
//      SpriteViewer에도 동기화한다. 시퀀스 속성에는 그 과정이 없다.
//   2. 초록 계열 색을 쓴다. (Start.X / Size.PanelW / Offset.Y …)
//   3. 소수점 한 자리로 표시한다. (시퀀스 속성은 두 자리)
void CAnimEditorUI::AddFramePropRow(float& Y, const wchar_t* Label, float Step,
    std::function<float()> Getter, std::function<void(float)> Setter,
    std::function<void(float, bool)> SetAll, int Decimals)
{
    int WidgetIdx = mDynIdx++;
    float PanelW = GetSize().x;

    // SetAll을 준 행에는 오른쪽 끝에 "All" 체크 버튼이 붙는다. 그만큼 자리를 비워둔다.
    const float AllW   = SetAll ? 30.f : 0.f;
    const float PlusX  = PanelW - 34.f - AllW;
    const float ValX   = 100.f;
    const float ValW   = PlusX - ValX - 2.f;

    MakeLabel("FramePropLabel_" + std::to_string(WidgetIdx), 6.f, Y, 70.f, ROW_H, Label, 11.f);

    auto MinButton = MakeButton("FramePropMinusButton_" + std::to_string(WidgetIdx), 76.f, Y + 1.f, 22.f, ROW_H - 2.f, 0.22f, 0.25f, 0.22f);
    MakeLabel("FramePropMinusLabel_" + std::to_string(WidgetIdx), 76.f, Y + 1.f, 22.f, ROW_H - 2.f, TEXT("-"), 12.f, ETextAlignH::Center);

    // 값 표시 배경 — 더블클릭하면 직접 입력 모드로 들어간다.
    auto ValButton = MakeButton("FramePropValueButton_" + std::to_string(WidgetIdx), ValX, Y + 1.f, ValW, ROW_H - 2.f,
        0.15f, 0.18f, 0.15f);

    TCHAR ValueBuffer[32];
    TCHAR ValueFormat[8];
    swprintf_s(ValueFormat, 8, L"%%.%df", Decimals);
    swprintf_s(ValueBuffer, 32, ValueFormat, Getter ? Getter() : 0.f);
    auto ValLabel = CreateWidget<CTextBlock>("FramePropValueLabel_" + std::to_string(WidgetIdx), 4).lock();
    if (ValLabel)
    {
        ValLabel->SetPos(ValX, Y);
        ValLabel->SetSize(ValW, ROW_H);
        ValLabel->SetText(ValueBuffer);
        ValLabel->SetFontSize(11.f);
        ValLabel->SetTextColor(FVector4(0.85f, 1.f, 0.85f, 1.f));
        ValLabel->SetAlignH(ETextAlignH::Center);
        ValLabel->SetAlignV(ETextAlignV::Middle);
    }

    auto PlusButton = MakeButton("FramePropPlusButton_" + std::to_string(WidgetIdx), PlusX, Y + 1.f, 22.f, ROW_H - 2.f, 0.22f, 0.25f, 0.22f);
    MakeLabel("FramePropPlusLabel_" + std::to_string(WidgetIdx), PlusX, Y + 1.f, 22.f, ROW_H - 2.f, TEXT("+"), 12.f, ETextAlignH::Center);

    FPropButton Row{ MinButton, PlusButton, ValLabel, ValButton, Step, Decimals, Getter, Setter };

    // ── All 체크 버튼 ──
    // 켜져 있으면 이 행의 값 변경이 시퀀스의 모든 프레임에 들어간다.
    if (SetAll)
    {
        int AllIdx = (int)mFramePropButtons.size();
        bool bOn   = (AllIdx < FRAME_PROP_MAX) && mFramePropAll[AllIdx];

        float AllX = PanelW - AllW - 4.f;

        auto AllButton = MakeButton("FramePropAllButton_" + std::to_string(WidgetIdx), AllX, Y + 1.f, AllW, ROW_H - 2.f,
            bOn ? 0.16f : 0.20f, bOn ? 0.40f : 0.20f, bOn ? 0.16f : 0.24f);

        auto AllLabel = MakeLabel("FramePropAllLabel_" + std::to_string(WidgetIdx), AllX, Y + 1.f, AllW, ROW_H - 2.f,
            TEXT("All"), 9.f, ETextAlignH::Center);

        if (auto Label = AllLabel.lock())
            Label->SetTextColor(bOn ? FVector4(0.6f, 1.f, 0.6f, 1.f) : FVector4(0.55f, 0.55f, 0.6f, 1.f));

        Row.SetAll = SetAll;
        Row.AllButton = AllButton;
        Row.AllLabel = AllLabel;
        Row.AllIdx = AllIdx;
    }

    mFramePropButtons.push_back(std::move(Row));
    Y += ROW_H + 2.f;
}

// 프레임의 실수 값 하나를 편집하는 행.
// Getter/Setter/SetAll 세 벌을 매번 손으로 쓰지 않도록, "그 값을 참조로 꺼내주는 함수"
// 하나만 받아서 셋 다 여기서 만들어준다.
void CAnimEditorUI::AddFrameFieldRow(float& Y, const wchar_t* Label, float Step,
    std::function<float&(FFrameData&)> Field, float MinValue, int Decimals)
{
    const int CompIdx  = mActiveComp;
    const int SeqIdx   = (CompIdx >= 0 && CompIdx < (int)mComps.size()) ? mComps[CompIdx].Selected : -1;
    const int FrameIdx = (SeqIdx  >= 0 && SeqIdx  < (int)mComps[CompIdx].Seqs.size())
                       ? mComps[CompIdx].Seqs[SeqIdx].SelectedFrame : -1;

    // 프레임 목록은 매번 다시 찾는다. (프레임 추가/삭제로 벡터가 바뀔 수 있다)
    auto GetFrames = [this, CompIdx, SeqIdx]() -> std::vector<FFrameData>*
    {
        if (CompIdx < 0 || CompIdx >= (int)mComps.size())                 return nullptr;
        if (SeqIdx  < 0 || SeqIdx  >= (int)mComps[CompIdx].Seqs.size())   return nullptr;
        return &mComps[CompIdx].Seqs[SeqIdx].Frames;
    };

    auto Clamp = [MinValue](float Value) { return Value < MinValue ? MinValue : Value; };

    AddFramePropRow(Y, Label, Step,
        // 표시 — 선택된 프레임의 값
        [GetFrames, Field, FrameIdx]() -> float
        {
            auto* Frames = GetFrames();
            if (!Frames || FrameIdx < 0 || FrameIdx >= (int)Frames->size()) return 0.f;
            return Field((*Frames)[FrameIdx]);
        },
        // 쓰기 — 선택된 프레임만
        [GetFrames, Field, Clamp, FrameIdx](float Value)
        {
            auto* Frames = GetFrames();
            if (!Frames || FrameIdx < 0 || FrameIdx >= (int)Frames->size()) return;
            Field((*Frames)[FrameIdx]) = Clamp(Value);
        },
        // 쓰기 — 모든 프레임. bDelta면 각 프레임에 그만큼 더한다.
        [GetFrames, Field, Clamp](float Value, bool bDelta)
        {
            auto* Frames = GetFrames();
            if (!Frames) return;

            for (auto& Frame : *Frames)
            {
                float& FieldRef = Field(Frame);
                FieldRef = Clamp(bDelta ? FieldRef + Value : Value);
            }
        },
        Decimals);
}

// ── 토글 행 ──────────────────────────────────────────────────────────────────
//
// ON/OFF 하나를 뒤집는 행. 위젯 3개다. (Loop / Reverse / Symmetry)
//
//   ToggleRowLabel       — 속성 이름
//   ToggleRowButton      — 누르면 뒤집히는 버튼. 켜지면 초록, 꺼지면 회색
//   ToggleRowButtonLabel — 그 위의 "ON" / "OFF" 글자
//
// +/- 가 없으므로 Step도 없고, Getter/Setter가 float이 아니라 bool을 다룬다.
void CAnimEditorUI::AddToggleRow(float& Y, const wchar_t* Label,
    std::function<bool()> Getter, std::function<void(bool)> Setter)
{
    int WidgetIdx = mDynIdx++;
    MakeLabel("ToggleRowLabel_" + std::to_string(WidgetIdx), 6.f, Y, 72.f, ROW_H, Label, 11.f);

    bool bOn = Getter ? Getter() : false;
    auto ToggleButton = MakeButton("ToggleRowButton_" + std::to_string(WidgetIdx), 80.f, Y + 1.f, 60.f, ROW_H - 2.f,
        bOn ? 0.12f : 0.22f, bOn ? 0.38f : 0.22f, bOn ? 0.12f : 0.28f);

    auto ButtonLabel = CreateWidget<CTextBlock>("ToggleRowButtonLabel_" + std::to_string(WidgetIdx), 5).lock();
    if (ButtonLabel)
    {
        ButtonLabel->SetPos(80.f, Y + 1.f);
        ButtonLabel->SetSize(60.f, ROW_H - 2.f);
        ButtonLabel->SetText(bOn ? TEXT("ON") : TEXT("OFF"));
        ButtonLabel->SetFontSize(11.f);
        ButtonLabel->SetTextColor(FVector4::White);
        ButtonLabel->SetAlignH(ETextAlignH::Center);
        ButtonLabel->SetAlignV(ETextAlignV::Middle);
    }

    mToggleButtons.push_back({ ToggleButton, ButtonLabel, Getter, Setter });
    Y += ROW_H + 2.f;
}

// ── 초기화 ───────────────────────────────────────────────────────────────────

bool CAnimEditorUI::Init()
{
    CWidgetContainer::Init();

    SetPos(PANEL_X, PANEL_Y);
    SetSize(PANEL_W, PANEL_H);

    // 내용이 패널보다 길어지면 휠로 스크롤할 수 있게 한다.
    // (타이틀바 아래부터 패널 끝까지가 스크롤 영역)
    EnableScroll(true);
    SetScrollArea(TITLE_H, PANEL_H);
    SetScrollStep(ROW_H + 2.f);

    auto Background = CreateWidget<CButton>("PanelBackground", 0).lock();
    if (Background)
    {
        Background->SetPos(0.f, 0.f);
        Background->SetSize(PANEL_W, PANEL_H);
        Background->SetTint(EWidgetState::Normal,  0.11f, 0.11f, 0.15f, 0.95f);
        Background->SetTint(EWidgetState::Hovered, 0.11f, 0.11f, 0.15f, 0.95f);
        Background->SetTint(EWidgetState::Clicked, 0.11f, 0.11f, 0.15f, 0.95f);
        Background->SetTint(EWidgetState::Release, 0.11f, 0.11f, 0.15f, 0.95f);
        Background->SetTint(EWidgetState::Disable, 0.11f, 0.11f, 0.15f, 0.95f);
        mBackground = Background;
    }

    auto TitleBar = CreateWidget<CTitleBar>("PanelTitleBar", 1).lock();
    if (TitleBar)
    {
        TitleBar->SetPos(0.f, 0.f);
        TitleBar->SetSize(PANEL_W, TITLE_H);
        TitleBar->SetTint(0.18f, 0.18f, 0.28f, 1.f);
        TitleBar->SetUpdateWidget(GetThisPtr<CWidget>());
        mTitleBarWidget = TitleBar;
    }

    auto TitleText = CreateWidget<CTextBlock>("PanelTitleLabel", 2).lock();
    if (TitleText)
    {
        TitleText->SetPos(0.f, 0.f);
        TitleText->SetSize(PANEL_W, TITLE_H);
        TitleText->SetText(TEXT("Animation Editor"));
        TitleText->SetFontSize(14.f);
        TitleText->SetTextColor(FVector4::White);
        TitleText->SetAlignH(ETextAlignH::Center);
        TitleText->SetAlignV(ETextAlignV::Middle);
        mTitleText = TitleText;
    }

    auto MakeHandle = [&](const std::string& Name, float X, float PosY) -> std::weak_ptr<CButton>
    {
        auto Handle = CreateWidget<CButton>(Name, 10).lock();
        if (Handle)
        {
            Handle->SetPos(X, PosY);
            Handle->SetSize(HANDLE_SZ, HANDLE_SZ);
            Handle->SetTint(EWidgetState::Normal,  0.40f, 0.40f, 0.50f, 0.85f);
            Handle->SetTint(EWidgetState::Hovered, 0.70f, 0.90f, 1.00f, 1.f);
            Handle->SetTint(EWidgetState::Clicked, 1.00f, 1.00f, 1.00f, 1.f);
            Handle->SetTint(EWidgetState::Release, 1.00f, 1.00f, 1.00f, 1.f);
            Handle->SetTint(EWidgetState::Disable, 0.25f, 0.25f, 0.25f, 0.5f);
        }
        return Handle;
    };
    mHandleTL = MakeHandle("ResizeHandleTopLeft", 0.f,               0.f);
    mHandleTR = MakeHandle("ResizeHandleTopRight", PANEL_W - HANDLE_SZ, 0.f);
    mHandleBL = MakeHandle("ResizeHandleBottomLeft", 0.f,               PANEL_H - HANDLE_SZ);
    mHandleBR = MakeHandle("ResizeHandleBottomRight", PANEL_W - HANDLE_SZ, PANEL_H - HANDLE_SZ);

    mStaticChildCount = (int)mChildList.size();
    mStaticWidgets    = mChildList;   // 정적 위젯 목록을 강한 참조로 보존
    return true;
}

// ── 핵심 로직 ─────────────────────────────────────────────────────────────────

void CAnimEditorUI::SyncFrames(int CompIdx, int SeqIdx)
{
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    if (SeqIdx < 0 || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;

    auto& Seq = mComps[CompIdx].Seqs[SeqIdx];
    Seq.Frames.clear();

    auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimManager) return;

    auto Anim = AnimManager->FindAnimation(Seq.Name).lock();
    if (!Anim) return;

    Seq.TextureType = Anim->GetType();

    // 피벗은 CAnimation2D에 없으므로 레지스트리에서 꺼내온다.
    FVector2 Pivot = CAnimRegistry::GetPivot(Seq.Name);
    Seq.PivotX = Pivot.x;
    Seq.PivotY = Pivot.y;

    // 재생 설정도 .anim2d에 저장된 값이 원본이다.
    // 시퀀스에서 읽어오면 안 된다. 액션 상태 컴포넌트가 상태를 바꿀 때마다
    // Loop와 Symmetry를 덮어쓰기 때문이다. (Symmetry는 바라보는 방향마다 뒤집힌다)
    // 그 값을 그대로 저장하면 런타임 상태가 파일에 박혀버린다.
    if (Anim->HasPlaySettings())
    {
        Seq.PlayRate = Anim->GetPlayRate();
        Seq.Loop     = Anim->GetLoop();
        Seq.Reverse  = Anim->GetReverse();
        Seq.Symmetry = Anim->GetSymmetry();
    }

    // 총 재생 시간은 프레임별 시간의 합이다. (저장된 값이 아니라 매번 더해서 낸다)
    Seq.PlayTime = Anim->GetTotalDuration();

    if (auto Texture = Anim->GetTexture().lock())
    {
        // 조회용 이름으로 쓰이므로 접두사를 뗀 형태로 보관한다.
        Seq.TextureName = StripTexturePrefix(Texture->GetName());

        // 텍스처 경로도 같이 복구한다.
        // 이걸 빼먹으면 [불러오기 → 수정 → 저장] 시 TextureRelPath가 빈 줄로 기록되고,
        // 다음 실행에서 SetTexture(이름) 폴백으로 넘어간다.
        // 그 함수는 이미 로드된 텍스처를 찾기만 할 뿐 파일을 읽지 않으므로
        // 아무도 그 텍스처를 안 올렸으면 영영 비어 있게 된다.
        if (const FTextureInfo* Info = Texture->GetTexture(0))
        {
            if (!Info->FullPath.empty())
                Seq.TextureRelPath = DialogUtil::ToRelativePath(DialogUtil::ToNarrow(Info->FullPath));
        }
    }

    int Count = Anim->GetFrameCount();
    for (int i = 0; i < Count; ++i)
    {
        const FTextureFrame& SourceFrame = Anim->GetFrame(i);
        FFrameData FrameData;
        FrameData.Start    = SourceFrame.Start;
        FrameData.Size     = SourceFrame.Size;
        FrameData.Offset   = SourceFrame.Offset;
        FrameData.Duration = SourceFrame.Duration;
        Seq.Frames.push_back(FrameData);
    }
    Seq.SelectedFrame = Seq.Frames.empty() ? -1 : 0;
}

void CAnimEditorUI::ApplyFrames(int CompIdx, int SeqIdx)
{
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    if (SeqIdx < 0 || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;

    auto& Seq = mComps[CompIdx].Seqs[SeqIdx];

    auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimManager) return;

    auto Anim = AnimManager->FindAnimation(Seq.Name).lock();
    if (!Anim) return;

    Anim->ClearFrame();
    for (auto& FrameData : Seq.Frames)
        Anim->AddFrame(FrameData.Start, FrameData.Size, FrameData.Offset, FrameData.Duration);
}

void CAnimEditorUI::SetTarget(std::weak_ptr<CActor> Actor)
{
    mTarget = Actor;
    mComps.clear();
    mActiveComp      = 0;
    mShowRegistry    = false;
    mShowFrameEditor = false;

    auto TargetActor = Actor.lock();
    if (TargetActor)
    {
        for (auto& Comp : TargetActor->GetActorCompList())
        {
            auto AnimComp = std::dynamic_pointer_cast<CAnimation2DComponent>(Comp);
            if (!AnimComp) continue;

            FCompData NewCompData;
            NewCompData.Comp     = AnimComp;
            NewCompData.CompName = Comp->GetName();

            // 컴포넌트에 이미 붙어 있는 시퀀스를 그대로 읽어온다.
            // 이게 없으면 월드를 불러오거나 액터를 다시 선택했을 때
            // 실제로는 애니메이션이 있는데도 Sequences 목록이 비어 보인다.
            const std::string CurrentName = AnimComp->GetCurrentAnimationName();

            for (const auto& Pair : AnimComp->GetAnimationMap())
            {
                const auto& Sequence = Pair.second;

                FSeqData Seq;
                Seq.Name     = Pair.first;
                Seq.PlayTime = Sequence->GetPlayTime();
                Seq.PlayRate = Sequence->GetPlayRate();
                Seq.Loop     = Sequence->GetLoop();
                Seq.Reverse  = Sequence->GetReverse();
                Seq.Symmetry = Sequence->GetSymmetry();

                if (Seq.Name == CurrentName)
                    NewCompData.Selected = (int)NewCompData.Seqs.size();

                NewCompData.Seqs.push_back(Seq);
            }

            // 현재 시퀀스가 따로 없으면 첫 번째를 고른다.
            if (NewCompData.Selected < 0 && !NewCompData.Seqs.empty())
                NewCompData.Selected = 0;

            mComps.push_back(NewCompData);
        }

        // 프레임/텍스처/피벗은 SyncFrames가 채운다. (mComps가 다 만들어진 뒤에 호출)
        for (int CompIdx = 0; CompIdx < (int)mComps.size(); ++CompIdx)
        {
            for (int SeqIdx = 0; SeqIdx < (int)mComps[CompIdx].Seqs.size(); ++SeqIdx)
                SyncFrames(CompIdx, SeqIdx);
        }
    }
    Rebuild();
}

void CAnimEditorUI::AddSeq(int CompIdx, const std::string& AnimName)
{
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    auto& CompData = mComps[CompIdx];

    for (auto& ExistSeq : CompData.Seqs) if (ExistSeq.Name == AnimName) return;

    FSeqData Seq;
    Seq.Name = AnimName;

    // .anim2d에 저장된 설정 그대로 붙인다.
    // 기본값(PlayTime 1초)으로 붙이면 AddAnimation -> SetPlayTime -> ScaleTotalDuration이
    // 돌면서 에셋의 프레임별 재생 시간이 전부 1초에 맞춰 늘어나거나 줄어든다.
    if (auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D))
    {
        if (auto Anim = AnimManager->FindAnimation(AnimName).lock())
        {
            if (Anim->GetFrameCount() > 0)
                Seq.PlayTime = Anim->GetTotalDuration();

            if (Anim->HasPlaySettings())
            {
                Seq.PlayRate = Anim->GetPlayRate();
                Seq.Loop     = Anim->GetLoop();
                Seq.Reverse  = Anim->GetReverse();
                Seq.Symmetry = Anim->GetSymmetry();
            }
        }
    }

    if (auto AnimComp = CompData.Comp.lock())
        AnimComp->AddAnimation(AnimName, Seq.PlayTime, Seq.PlayRate, Seq.Loop, Seq.Reverse, Seq.Symmetry);

    CompData.Seqs.push_back(Seq);
    CompData.Selected = (int)CompData.Seqs.size() - 1;

    // 기존 프레임 데이터 동기화
    SyncFrames(CompIdx, CompData.Selected);

    // 방금 추가한 시퀀스로 뷰어를 옮겨준다.
    RefreshSpriteViewer();

    mShowRegistry = false;
    Rebuild();
}

void CAnimEditorUI::PlaySeq(int CompIdx, int SeqIdx)
{
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    auto& CompData = mComps[CompIdx];
    if (SeqIdx < 0 || SeqIdx >= (int)CompData.Seqs.size()) return;

    if (auto AnimComp = CompData.Comp.lock())
        AnimComp->ChangeAnimation(CompData.Seqs[SeqIdx].Name);

    CompData.Selected = SeqIdx;

    RefreshSpriteViewer();

    Rebuild();
}

// 뷰어의 콜백은 열릴 당시의 (CompIdx, SeqIdx)를 캡처해둔다.
// 그래서 SyncSpriteViewer로 프레임만 갈아끼우면 편집 결과가 이전 시퀀스로 흘러들어가고,
// 텍스처도 이전 것이 그대로 남는다. 통째로 다시 열어야 한다.
void CAnimEditorUI::RefreshSpriteViewer()
{
    auto Viewer = mSpriteViewer.lock();

    if (!Viewer || !Viewer->IsEnable())
        return;

    if (mActiveComp < 0 || mActiveComp >= (int)mComps.size())
        return;

    auto& CompData = mComps[mActiveComp];

    if (CompData.Selected < 0 || CompData.Selected >= (int)CompData.Seqs.size())
        return;

    OpenSpriteViewer(mActiveComp, CompData.Selected);
}

void CAnimEditorUI::CreateNewAnim()
{
    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Anim\\";
    CreateDirectoryA((DialogUtil::GetExeDir() + "Asset\\").c_str(), nullptr);
    CreateDirectoryA(Dir.c_str(), nullptr);

    // SaveFile 다이얼로그: 파일명 입력 = 애니메이션 이름
    std::string Path = DialogUtil::SaveFile(
        "Anim Files\0*.anim2d\0All Files\0*.*\0", Dir.c_str(), "anim2d");
    if (Path.empty()) return;

    std::string AnimName = DialogUtil::ExtractBaseName(Path);
    if (AnimName.empty()) return;

    auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimManager) return;

    if (!AnimManager->FindAnimation(AnimName).lock())
        AnimManager->CreateAnimation(AnimName);

    CAnimRegistry::Register(AnimName);

    // AnimComp가 있으면 시퀀스도 추가, 없으면 에셋만 등록
    if (!mComps.empty())
    {
        auto& CompData = mComps[mActiveComp];
        bool AlreadyHave = false;
        for (auto& ExistSeq : CompData.Seqs) if (ExistSeq.Name == AnimName) { AlreadyHave = true; break; }

        if (!AlreadyHave)
        {
            if (auto AnimComp = CompData.Comp.lock())
                AnimComp->AddAnimation(AnimName, 1.f, 1.f, true, false, false);

            FSeqData Seq;
            Seq.Name = AnimName;
            CompData.Seqs.push_back(Seq);
            CompData.Selected = (int)CompData.Seqs.size() - 1;
        }

        // 새로 만든 시퀀스로 뷰어를 옮겨준다.
        // (아직 텍스처가 없으므로 뷰어는 비워진 상태가 된다)
        RefreshSpriteViewer();
    }

    LOG_DEBUG("[AnimEditor] Created: %s", AnimName.c_str());
    Rebuild();
}

void CAnimEditorUI::SetAnimTexture(int CompIdx, int SeqIdx)
{
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    if (SeqIdx < 0 || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;

    auto& Seq = mComps[CompIdx].Seqs[SeqIdx];

    std::string Path = DialogUtil::OpenFile(
        "Image Files\0*.png;*.jpg;*.bmp;*.tga\0All Files\0*.*\0", nullptr);
    if (Path.empty()) return;

    std::string TexName = DialogUtil::ExtractBaseName(Path);
    Seq.TextureName = TexName;

    // exe 기준 상대경로 저장 (Save/Load 재사용 위해. 폴더 밖이면 절대경로 그대로)
    Seq.TextureRelPath = DialogUtil::ToRelativePath(Path);

    auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimManager) return;

    auto Anim = AnimManager->FindAnimation(Seq.Name).lock();
    if (!Anim) return;

    std::wstring WPath = DialogUtil::ToWide(Path);
    Anim->SetTextureFullPath(TexName, WPath.c_str());

    // 뷰어가 열려 있으면 새 텍스처로 다시 그려준다.
    // (안 하면 이전 시퀀스의 시트가 그대로 남아 있다)
    RefreshSpriteViewer();

    LOG_DEBUG("[AnimEditor] Texture set: %s -> %s", Seq.Name.c_str(), TexName.c_str());
    Rebuild();
}

void CAnimEditorUI::SaveAnim(int CompIdx, int SeqIdx)
{
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    if (SeqIdx < 0 || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;

    auto& Seq = mComps[CompIdx].Seqs[SeqIdx];

    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Anim\\";
    CreateDirectoryA((DialogUtil::GetExeDir() + "Asset\\").c_str(), nullptr);
    CreateDirectoryA(Dir.c_str(), nullptr);

    // 이 애니메이션이 실제로 들어 있던 파일을 기본값으로 열어준다.
    // 그래야 "덮어쓰기"가 자연스럽게 원래 파일을 향한다.
    std::string Suggested = CAnimRegistry::GetSourceFile(Seq.Name);

    if (Suggested.empty())
    {
        Suggested = Dir + Seq.Name + ".anim2d";
    }

    std::string Path = DialogUtil::SaveFile(
        "Anim Files\0*.anim2d\0All Files\0*.*\0", Dir.c_str(), "anim2d", Suggested.c_str());
    if (Path.empty()) return;

    // ── 이름 충돌 검사 ────────────────────────────────────────────────────────
    // 애니메이션은 파일 이름이 아니라 파일 안의 AnimName으로 구분된다.
    // 그래서 파일 이름만 바꿔 저장해도 AnimName은 그대로 따라가고,
    // 같은 이름이 두 파일에 생기면 다음 실행에서 폴더를 읽을 때
    // 어느 쪽이 살아남을지 알 수 없게 된다. (저장했는데 옛날 데이터가 나오는 원인)
    {
        const std::string ExistingName = PeekAnimName(Path);

        // (1) 다른 애니메이션이 들어 있는 파일에 덮어쓰려는 경우
        if (!ExistingName.empty() && ExistingName != Seq.Name)
        {
            std::wstring Message = L"이 파일에는 '" + DialogUtil::ToWide(ExistingName) + L"' 이(가) 들어 있습니다.\n"
                               L"여기에 '" + DialogUtil::ToWide(Seq.Name) + L"' 을(를) 저장하면 같은 이름이 두 파일에 생겨서,\n"
                               L"다음 실행에서 어느 쪽이 쓰일지 알 수 없게 됩니다.\n\n"
                               L"원래 파일에 덮어쓰거나 새 이름의 파일을 골라주세요.";

            DialogUtil::Alert(Message.c_str(), L"저장 취소");

            LOG_ERROR("[AnimEditor] Save canceled: '%s' already holds anim '%s' (saving '%s')",
                Path.c_str(), ExistingName.c_str(), Seq.Name.c_str());
            return;
        }

        // (2) 이 애니메이션이 이미 다른 파일에 저장돼 있는 경우
        const std::string& Prev = CAnimRegistry::GetSourceFile(Seq.Name);

        if (!Prev.empty() && !IsSamePath(Prev, Path))
        {
            std::wstring Message = L"'" + DialogUtil::ToWide(Seq.Name) + L"' 은(는) 이미 아래 파일에 저장돼 있습니다.\n\n"
                               + DialogUtil::ToWide(Prev) + L"\n\n"
                               L"다른 파일로 또 저장하면 같은 이름이 두 개가 되어\n"
                               L"다음 실행에서 방금 저장한 쪽이 무시될 수 있습니다.\n"
                               L"위 파일에 덮어써 주세요. (백업은 Asset 폴더 바깥에 두세요)";

            DialogUtil::Alert(Message.c_str(), L"저장 취소");

            LOG_ERROR("[AnimEditor] Save canceled: anim '%s' already lives in '%s' (tried '%s')",
                Seq.Name.c_str(), Prev.c_str(), Path.c_str());
            return;
        }
    }

    std::ofstream File(Path);
    if (!File) return;

    std::string TypeStr = (Seq.TextureType == EAnimation2DTextureType::Frame) ? "Frame" : "SpriteSheet";

    File << "ANIM2D 1\n";
    File << "AnimName "      << Seq.Name            << "\n";
    File << "TextureType "   << TypeStr             << "\n";
    File << "TextureName "   << Seq.TextureName      << "\n";
    File << "TextureRelPath "<< Seq.TextureRelPath   << "\n";
    //PlayTime은 이제 프레임별 Duration의 합으로 나오는 파생값이다.
    //예전 형식(프레임별 시간이 없는 파일)과의 호환을 위해 합계를 그대로 적어둔다.
    float TotalDuration = 0.f;
    for (const auto& FrameData : Seq.Frames) TotalDuration += FrameData.Duration;

    File << "PlayTime "      << TotalDuration        << "\n";
    File << "PlayRate "   << Seq.PlayRate    << "\n";
    File << "Loop "       << (Seq.Loop     ? 1 : 0) << "\n";
    File << "Reverse "    << (Seq.Reverse  ? 1 : 0) << "\n";
    File << "Symmetry "   << (Seq.Symmetry ? 1 : 0) << "\n";
    File << "Pivot "      << Seq.PivotX << " " << Seq.PivotY << "\n";
    File << "Frames "     << Seq.Frames.size() << "\n";
    for (auto& FrameData : Seq.Frames)
        File << FrameData.Start.x  << " " << FrameData.Start.y  << " "
             << FrameData.Size.x   << " " << FrameData.Size.y   << " "
             << FrameData.Offset.x << " " << FrameData.Offset.y << " "
             << FrameData.Duration << "\n";

    // 이제 이 애니메이션의 출처는 방금 저장한 파일이다.
    CAnimRegistry::SetSourceFile(Seq.Name, Path);

    // 방금 저장한 설정을 에셋에도 반영해둔다.
    // 이게 없으면 액터를 다시 고르는 순간 SyncFrames가 에셋의 예전 값을 다시 끌어와서,
    // 저장은 됐는데 화면은 옛날 값으로 돌아간 것처럼 보인다.
    if (auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D))
    {
        if (auto Anim = AnimManager->FindAnimation(Seq.Name).lock())
            Anim->SetPlaySettings(Seq.PlayRate, Seq.Loop, Seq.Reverse, Seq.Symmetry);
    }

    CAnimRegistry::SetPivot(Seq.Name, Seq.PivotX, Seq.PivotY);

    LOG_DEBUG("[AnimEditor] Saved: %s", Path.c_str());
}

// ── 공통 파싱: 파일 하나를 읽어 CAnimation2D 생성 + CAnimRegistry 등록 ──────

bool CAnimEditorUI::LoadAnimFromFile(const std::string& Path, FLoadedAnimInfo* OutInfo,
                                     bool bSkipDuplicateName)
{
    std::ifstream File(Path);
    if (!File) return false;

    std::string AnimName, TextureName, TextureRelPath;
    EAnimation2DTextureType TextureType = EAnimation2DTextureType::SpriteSheet;
    float PlayTime = 1.f, PlayRate = 1.f;
    float PivotX = 0.f, PivotY = 0.f;   // Pivot 줄이 없는 예전 파일이면 0으로 남는다
    bool  Loop = true, Reverse = false, Symmetry = false;
    std::vector<FFrameData> Frames;

    std::string Line;
    while (std::getline(File, Line))
    {
        if (Line.empty() || Line[0] == '#') continue;
        std::istringstream LineStream(Line);
        std::string Key; LineStream >> Key;

        if      (Key == "ANIM2D")        { /* 버전 */ }
        else if (Key == "AnimName")       LineStream >> AnimName;
        else if (Key == "TextureType")    { std::string TypeStr; LineStream >> TypeStr; TextureType = (TypeStr == "Frame") ? EAnimation2DTextureType::Frame : EAnimation2DTextureType::SpriteSheet; }
        else if (Key == "TextureName")    LineStream >> TextureName;
        else if (Key == "TextureRelPath") std::getline(LineStream >> std::ws, TextureRelPath);
        else if (Key == "PlayTime")       LineStream >> PlayTime;
        else if (Key == "PlayRate")       LineStream >> PlayRate;
        else if (Key == "Loop")           { int Flag; LineStream >> Flag; Loop     = (Flag != 0); }
        else if (Key == "Reverse")        { int Flag; LineStream >> Flag; Reverse  = (Flag != 0); }
        else if (Key == "Symmetry")       { int Flag; LineStream >> Flag; Symmetry = (Flag != 0); }
        else if (Key == "Pivot")          LineStream >> PivotX >> PivotY;
        else if (Key == "Frames")
        {
            int Count = 0; LineStream >> Count;
            for (int i = 0; i < Count; ++i)
            {
                std::string FLine;
                while (std::getline(File, FLine) && FLine.empty()) {}
                FFrameData FrameData;
                std::istringstream FrameStream(FLine);
                FrameStream >> FrameData.Start.x >> FrameData.Start.y >> FrameData.Size.x >> FrameData.Size.y >> FrameData.Offset.x >> FrameData.Offset.y;

                //7번째 숫자가 프레임별 재생 시간.
                //없는 예전 파일은 음수로 표시해두고 아래에서 PlayTime을 균등 분배한다.
                if (!(FrameStream >> FrameData.Duration))
                    FrameData.Duration = -1.f;

                Frames.push_back(FrameData);
            }
        }
    }

    if (AnimName.empty()) return false;

    // 같은 AnimName을 주장하는 .anim2d가 두 개 있으면, 폴더를 통째로 읽을 때
    // 나중에 읽힌 쪽이 앞의 것을 통째로 덮어쓴다. (프레임까지 전부)
    // 그러면 에디터에서 저장을 해도 다시 켜면 다른 파일의 내용이 나온다.
    // 조용히 덮어쓰지 말고 먼저 자리를 잡은 파일을 살리고, 무엇과 부딪혔는지 남긴다.
    {
        const std::string& Prev = CAnimRegistry::GetSourceFile(AnimName);

        if (!Prev.empty() && Prev != Path)
        {
            if (bSkipDuplicateName)
            {
                LOG_ERROR("[AnimEditor] AnimName '%s' 중복 - '%s' 를 건너뜁니다. (이미 '%s' 가 사용 중)",
                    AnimName.c_str(), Path.c_str(), Prev.c_str());
                return false;
            }

            LOG_WARNING("[AnimEditor] AnimName '%s' 를 '%s' 의 내용으로 덮어씁니다. (이전: '%s')",
                AnimName.c_str(), Path.c_str(), Prev.c_str());
        }

        CAnimRegistry::SetSourceFile(AnimName, Path);
    }

    //프레임별 재생 시간이 없는 예전 파일이면 PlayTime을 균등 분배한다.
    //(그래야 예전과 똑같은 속도로 재생된다)
    if (!Frames.empty())
    {
        float Each = PlayTime / (float)Frames.size();
        if (Each < 0.001f) Each = 0.001f;

        for (auto& FrameData : Frames)
        {
            if (FrameData.Duration < 0.f)
                FrameData.Duration = Each;
        }
    }

    auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimManager) return false;

    if (!AnimManager->FindAnimation(AnimName).lock())
        AnimManager->CreateAnimation(AnimName);

    auto Anim = AnimManager->FindAnimation(AnimName).lock();
    if (!Anim) return false;

    Anim->SetAnimationTextureType(TextureType);

    // 파일에 적힌 한글은 저장한 PC의 코드페이지를 따른다.
    // exe 경로(CP_ACP)와 이어붙이기 전에 이 PC 기준으로 맞춰둔다.
    TextureName    = DialogUtil::ToAcp(TextureName);
    TextureRelPath = DialogUtil::ToAcp(TextureRelPath);

    // TextureRelPath 우선, 없으면 TextureName 폴백
    if (!TextureRelPath.empty())
    {
        std::string FullPath = DialogUtil::GetExeDir() + TextureRelPath;
        std::wstring WPath = DialogUtil::ToWide(FullPath);
        Anim->SetTextureFullPath(TextureName, WPath.c_str());

        //텍스처가 안 붙으면 재생할 때 빈 텍스처를 만지게 되므로 여기서 남겨둔다.
        if (Anim->GetTexture().expired())
            LOG_ERROR("[AnimEditor] Texture load failed: %s", FullPath.c_str());
    }
    else if (!TextureName.empty())
    {
        Anim->SetTexture(TextureName);
    }

    Anim->ClearFrame();
    for (auto& FrameData : Frames)
        Anim->AddFrame(FrameData.Start, FrameData.Size, FrameData.Offset, FrameData.Duration);

    // 재생 설정도 에셋에 남겨둔다.
    // 예전에는 OutInfo로만 돌려줘서, 시작할 때 폴더를 통째로 읽는 LoadAllAnims는
    // 이 값을 그냥 버렸다. 그러면 월드에 적혀 있던 예전 PlayRate/Loop가 대신 쓰여서
    // 애님 에디터에서 고쳐 저장해도 다시 켜면 원래대로 보였다.
    Anim->SetPlaySettings(PlayRate, Loop, Reverse, Symmetry);

    CAnimRegistry::Register(AnimName);

    // 피벗은 CAnimation2D가 안 들고 있으므로 레지스트리에 맡겨둔다.
    // SyncFrames가 시퀀스를 채울 때 여기서 다시 꺼내간다.
    CAnimRegistry::SetPivot(AnimName, PivotX, PivotY);

    //불러오기에서 컴포넌트에 붙일 때 파일의 설정을 그대로 쓰도록 넘겨준다.
    if (OutInfo)
    {
        OutInfo->Name       = AnimName;
        OutInfo->PlayTime   = PlayTime;
        OutInfo->PlayRate   = PlayRate;
        OutInfo->Loop       = Loop;
        OutInfo->Reverse    = Reverse;
        OutInfo->Symmetry   = Symmetry;
        OutInfo->PivotX     = PivotX;
        OutInfo->PivotY     = PivotY;
        OutInfo->FrameCount = (int)Frames.size();
    }

    return true;
}

// ── 시작 시 전체 자동 로드 ────────────────────────────────────────────────────

void CAnimEditorUI::LoadAllAnims()
{
    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Anim\\";

    WIN32_FIND_DATAA FindData;
    HANDLE hFind = FindFirstFileA((Dir + "*.anim2d").c_str(), &FindData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    int Count = 0;
    do
    {
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        // 이름이 겹치는 파일은 건너뛴다. 폴더를 읽는 순서에 따라
        // 어느 쪽이 살아남을지 달라지는 게 제일 곤란하다.
        if (LoadAnimFromFile(Dir + FindData.cFileName, nullptr, true))
            ++Count;
    } while (FindNextFileA(hFind, &FindData));

    FindClose(hFind);
    LOG_DEBUG("[AnimEditor] Auto-loaded %d anim(s) from %s", Count, Dir.c_str());
}

void CAnimEditorUI::LoadAnim()
{
    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Anim\\";
    std::string Path = DialogUtil::OpenFile(
        "Anim Files\0*.anim2d\0All Files\0*.*\0", Dir.c_str());
    if (Path.empty()) return;

    // CAnimation2D 생성 + 레지스트리 등록. 파일에 저장된 설정도 같이 받아온다.
    FLoadedAnimInfo Info;
    if (!LoadAnimFromFile(Path, &Info) || Info.Name.empty())
    {
        LOG_ERROR("[AnimEditor] Load failed: %s", Path.c_str());
        return;
    }

    // 액터에 애니메이션 컴포넌트가 없으면 붙일 곳이 없다.
    // 에셋 등록은 이미 끝났으니 "+ 애니메이션 추가" 목록에는 나온다.
    if (mComps.empty())
    {
        LOG_WARNING("[AnimEditor] '%s' loaded, but the actor has no Animation2DComponent.",
            Info.Name.c_str());
        Rebuild();
        return;
    }

    if (mActiveComp < 0 || mActiveComp >= (int)mComps.size())
        mActiveComp = 0;

    auto& CompData = mComps[mActiveComp];

    // 이 컴포넌트가 실제로 그 애니메이션을 들고 있는지 본다.
    // 에디터 목록(Seqs)이 아니라 컴포넌트를 직접 확인해야 한다.
    // 월드에서 불러온 액터처럼 둘이 어긋나 있을 수 있기 때문이다.
    auto AnimComp = CompData.Comp.lock();
    bool bCompHas = false;

    if (AnimComp)
        bCompHas = (AnimComp->GetAnimationMap().find(Info.Name) != AnimComp->GetAnimationMap().end());

    // 없으면 파일에 저장돼 있던 설정 그대로 붙인다.

    if (AnimComp && !bCompHas)
    {
        AnimComp->AddAnimation(Info.Name, Info.PlayTime, Info.PlayRate,
            Info.Loop, Info.Reverse, Info.Symmetry);

        LOG_DEBUG("[AnimEditor] Added '%s' to %s (PlayTime=%.2f Loop=%d Frames=%d)",
            Info.Name.c_str(), CompData.CompName.c_str(),
            Info.PlayTime, Info.Loop ? 1 : 0, Info.FrameCount);
    }

    // 에디터 목록에도 없으면 새 행을 만든다.
    int SeqIdx = -1;
    for (int i = 0; i < (int)CompData.Seqs.size(); ++i)
        if (CompData.Seqs[i].Name == Info.Name) { SeqIdx = i; break; }

    if (SeqIdx < 0)
    {
        FSeqData Seq;
        Seq.Name     = Info.Name;
        Seq.PlayTime = Info.PlayTime;
        Seq.PlayRate = Info.PlayRate;
        Seq.Loop     = Info.Loop;
        Seq.Reverse  = Info.Reverse;
        Seq.Symmetry = Info.Symmetry;
        CompData.Seqs.push_back(Seq);
        SeqIdx = (int)CompData.Seqs.size() - 1;
    }

    CompData.Selected = SeqIdx;

    // 프레임/텍스처/피벗은 파일에서 읽은 내용으로 항상 다시 맞춘다.

    SyncFrames(mActiveComp, SeqIdx);

    // 불러온 시퀀스로 뷰어를 옮겨준다.
    RefreshSpriteViewer();

    LOG_DEBUG("[AnimEditor] Loaded: %s", Info.Name.c_str());
    Rebuild();
}

void CAnimEditorUI::UpdateHandles(float NewW, float NewH)
{
    if (auto Background = mBackground.lock())      Background->SetSize(NewW, NewH);
    if (auto Bar = mTitleBarWidget.lock()) Bar->SetSize(NewW, TITLE_H);
    if (auto Title = mTitleText.lock())    Title->SetSize(NewW, TITLE_H);

    if (auto Handle = mHandleTL.lock()) Handle->SetPos(0.f,              0.f);
    if (auto Handle = mHandleTR.lock()) Handle->SetPos(NewW - HANDLE_SZ, 0.f);
    if (auto Handle = mHandleBL.lock()) Handle->SetPos(0.f,              NewH - HANDLE_SZ);
    if (auto Handle = mHandleBR.lock()) Handle->SetPos(NewW - HANDLE_SZ, NewH - HANDLE_SZ);
}

// ── 위젯 재구성 ──────────────────────────────────────────────────────────────

void CAnimEditorUI::Rebuild()
{
    // 편집 중이던 행의 위젯이 사라지므로 먼저 편집을 끝낸다.
    CancelEdit();

    // 정적 위젯 목록으로 mChildList를 완전히 초기화 — 이전 Rebuild의 동적 위젯을 확실히 제거
    if (!mStaticWidgets.empty())
        mChildList = mStaticWidgets;
    else if ((int)mChildList.size() > mStaticChildCount)
        mChildList.resize(mStaticChildCount);

    mTabButtons.clear(); mSeqButtons.clear();
    mPropButtons.clear(); mToggleButtons.clear();
    mRegButtons.clear(); mFramePropButtons.clear();
    mAddToggleButton = {}; mFrameCountText = {}; mPlayTimeText = {};
    mToggleFrameButton = {}; mFramePrevButton = {}; mFrameNextButton = {};
    mFrameIdxLabel = {}; mAddFrameButton = {}; mDelFrameButton = {}; mClearFramesButton = {};
    mSetTextureButton = {}; mTypeToggleButton = {};
    mSaveAnimButton = {}; mLoadAnimButton = {}; mNewAnimButton = {};
    mOpenViewerButton = {};

    float PanelW = GetSize().x;
    float LayoutH = GetSize().y;
    float Y = TITLE_H + 4.f;

    // 스크롤을 쓰는 경우 패널 높이에 맞춰 잘라내지 않고 콘텐츠를 끝까지 만든다.
    // (아래 레이아웃 가드들이 전부 H와 비교하므로 여기서 한 번에 풀어준다)
    if (mScrollEnable)
    {
        LayoutH = 100000.f;
    }

    // ── Actor 이름 ────────────────────────────────────────────────────────────
    {
        std::string ActorName = "(none)";
        if (auto TargetActor = mTarget.lock()) ActorName = TargetActor->GetName();
        std::wstring WName(ActorName.begin(), ActorName.end());
        TCHAR TextBuffer[128]; wsprintf(TextBuffer, TEXT("Actor: %s"), WName.c_str());
        MakeLabel("ActorNameLabel_" + std::to_string(mDynIdx++), 6.f, Y, PanelW - 8.f, ROW_H, TextBuffer, 11.f);
        Y += ROW_H + 2.f;
    }

    // ── 새 애니메이션 만들기 버튼 (항상 표시) ─────────────────────────────────
    {
        auto NewButton = MakeButton("NewAnimButton_" + std::to_string(mDynIdx), 4.f, Y, PanelW - 8.f, ROW_H,
            0.28f, 0.22f, 0.10f);
        MakeLabel("NewAnimButtonLabel_" + std::to_string(mDynIdx++), 4.f, Y, PanelW - 8.f, ROW_H,
            TEXT("새 애니메이션 만들기"), 11.f, ETextAlignH::Center);
        mNewAnimButton = NewButton;
        Y += ROW_H + 4.f;
    }

    if (mComps.empty())
    {
        MakeLabel("NoAnimCompLabel_" + std::to_string(mDynIdx++), 6.f, Y, PanelW - 8.f, ROW_H,
            TEXT("AnimComp 없음 - 에셋만 생성됨"), 10.f);
        FinishLayout(Y + ROW_H);
        return;
    }

    // ── AnimComp 탭 ───────────────────────────────────────────────────────────
    {
        float TabW = min((PanelW - 8.f) / (float)mComps.size(), 90.f);
        float TabX = 4.f;
        for (int i = 0; i < (int)mComps.size(); ++i)
        {
            std::wstring WName(mComps[i].CompName.begin(), mComps[i].CompName.end());
            bool bActive = (i == mActiveComp);
            auto Button = MakeButton("CompTabButton_" + std::to_string(mDynIdx), TabX, Y, TabW - 2.f, 22.f,
                bActive ? 0.22f : 0.16f,
                bActive ? 0.38f : 0.22f,
                bActive ? 0.55f : 0.28f);
            MakeLabel("CompTabButtonLabel_" + std::to_string(mDynIdx++), TabX, Y, TabW - 2.f, 22.f,
                WName.c_str(), 9.f, ETextAlignH::Center);
            mTabButtons.push_back({ Button, i });
            TabX += TabW;
        }
        Y += 26.f;
    }

    if (mActiveComp >= (int)mComps.size()) mActiveComp = 0;
    auto& CompData = mComps[mActiveComp];

    // ── 시퀀스 목록 ───────────────────────────────────────────────────────────
    MakeLabel("SequenceHeaderLabel_" + std::to_string(mDynIdx++), 4.f, Y, PanelW - 8.f, 14.f,
        TEXT("- Sequences -"), 10.f);
    Y += 16.f;

    float MaxSeqY = LayoutH - 300.f;
    for (int i = 0; i < (int)CompData.Seqs.size(); ++i)
    {
        if (Y + ROW_H > MaxSeqY) break;
        std::wstring WName(CompData.Seqs[i].Name.begin(), CompData.Seqs[i].Name.end());
        bool bSel = (i == CompData.Selected);
        auto Button = MakeButton("SequenceButton_" + std::to_string(mDynIdx), 4.f, Y, PanelW - 8.f, ROW_H,
            bSel ? 0.18f : 0.14f,
            bSel ? 0.38f : 0.20f,
            bSel ? 0.65f : 0.26f);
        MakeLabel("SequenceButtonLabel_" + std::to_string(mDynIdx++), 8.f, Y, PanelW - 12.f, ROW_H,
            WName.c_str(), 11.f);
        mSeqButtons.push_back({ Button, i });
        Y += ROW_H + 2.f;
    }

    // ── 선택된 시퀀스 속성 ────────────────────────────────────────────────────
    if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
    {
        Y += 4.f;
        MakeLabel("PropertiesHeaderLabel_" + std::to_string(mDynIdx++), 4.f, Y, PanelW - 8.f, 14.f,
            TEXT("- Properties -"), 10.f);
        Y += 16.f;

        int CompIdx = mActiveComp, SeqIdx = CompData.Selected;
        std::weak_ptr<CAnimation2DComponent> WeakComp = CompData.Comp;

       
        {
            float Total = 0.f;
            for (const auto& FrameData : CompData.Seqs[SeqIdx].Frames)
                Total += FrameData.Duration;

            TCHAR TotalBuffer[32];
            swprintf_s(TotalBuffer, 32, L"%.2f s", Total);

            MakeLabel("PlayTimeLabel_" + std::to_string(mDynIdx++), 6.f, Y, 72.f, ROW_H,
                TEXT("PlayTime"), 11.f);

            mPlayTimeText = MakeLabel("PlayTimeValue_" + std::to_string(mDynIdx++),
                104.f, Y, PanelW - 140.f, ROW_H, TotalBuffer, 11.f, ETextAlignH::Center);

            Y += ROW_H + 2.f;
        }

        AddPropRow(Y, TEXT("PlayRate"), 0.1f,
            [this, CompIdx, SeqIdx]() -> float {
                if (CompIdx < (int)mComps.size() && SeqIdx < (int)mComps[CompIdx].Seqs.size())
                    return mComps[CompIdx].Seqs[SeqIdx].PlayRate;
                return 1.f;
            },
            [this, CompIdx, SeqIdx, WeakComp](float Value) {
                if (CompIdx >= (int)mComps.size() || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
                mComps[CompIdx].Seqs[SeqIdx].PlayRate = max(0.05f, Value);
                if (auto AnimComp = WeakComp.lock())
                    AnimComp->SetPlayRate(mComps[CompIdx].Seqs[SeqIdx].Name, mComps[CompIdx].Seqs[SeqIdx].PlayRate);
            });

        // ── 피벗 기준선 ────────────────────────────────────────────────────
        // 스프라이트 뷰어의 청록 세로선(X) / 자홍 가로선(Y) 위치.
        // 시퀀스 단위 값이라 프레임을 넘겨도 유지된다.
        AddPropRow(Y, TEXT("Pivot.X"), 1.f,
            [this, CompIdx, SeqIdx]() -> float {
                if (CompIdx < (int)mComps.size() && SeqIdx < (int)mComps[CompIdx].Seqs.size())
                    return mComps[CompIdx].Seqs[SeqIdx].PivotX;
                return 0.f;
            },
            [this, CompIdx, SeqIdx](float Value) {
                if (CompIdx >= (int)mComps.size() || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
                auto& Target = mComps[CompIdx].Seqs[SeqIdx];
                Target.PivotX = Value;
                CAnimRegistry::SetPivot(Target.Name, Target.PivotX, Target.PivotY);
            });

        AddPropRow(Y, TEXT("Pivot.Y"), 1.f,
            [this, CompIdx, SeqIdx]() -> float {
                if (CompIdx < (int)mComps.size() && SeqIdx < (int)mComps[CompIdx].Seqs.size())
                    return mComps[CompIdx].Seqs[SeqIdx].PivotY;
                return 0.f;
            },
            [this, CompIdx, SeqIdx](float Value) {
                if (CompIdx >= (int)mComps.size() || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
                auto& Target = mComps[CompIdx].Seqs[SeqIdx];
                Target.PivotY = Value;
                CAnimRegistry::SetPivot(Target.Name, Target.PivotX, Target.PivotY);
            });

        AddToggleRow(Y, TEXT("Loop"),
            [this, CompIdx, SeqIdx]() -> bool { return CompIdx < (int)mComps.size() && SeqIdx < (int)mComps[CompIdx].Seqs.size() ? mComps[CompIdx].Seqs[SeqIdx].Loop : true; },
            [this, CompIdx, SeqIdx, WeakComp](bool Value) {
                if (CompIdx >= (int)mComps.size() || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
                mComps[CompIdx].Seqs[SeqIdx].Loop = Value;
                if (auto AnimComp = WeakComp.lock()) AnimComp->SetLoop(mComps[CompIdx].Seqs[SeqIdx].Name, Value);
            });

        AddToggleRow(Y, TEXT("Reverse"),
            [this, CompIdx, SeqIdx]() -> bool { return CompIdx < (int)mComps.size() && SeqIdx < (int)mComps[CompIdx].Seqs.size() ? mComps[CompIdx].Seqs[SeqIdx].Reverse : false; },
            [this, CompIdx, SeqIdx, WeakComp](bool Value) {
                if (CompIdx >= (int)mComps.size() || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
                mComps[CompIdx].Seqs[SeqIdx].Reverse = Value;
                if (auto AnimComp = WeakComp.lock()) AnimComp->SetReverse(mComps[CompIdx].Seqs[SeqIdx].Name, Value);
            });

        AddToggleRow(Y, TEXT("Symmetry"),
            [this, CompIdx, SeqIdx]() -> bool { return CompIdx < (int)mComps.size() && SeqIdx < (int)mComps[CompIdx].Seqs.size() ? mComps[CompIdx].Seqs[SeqIdx].Symmetry : false; },
            [this, CompIdx, SeqIdx, WeakComp](bool Value) {
                if (CompIdx >= (int)mComps.size() || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
                mComps[CompIdx].Seqs[SeqIdx].Symmetry = Value;
                if (auto AnimComp = WeakComp.lock()) AnimComp->SetSymmetry(mComps[CompIdx].Seqs[SeqIdx].Name, Value);
            });

        Y += 4.f;

        // ── 텍스처 설정 ───────────────────────────────────────────────────────
        if (Y + ROW_H < LayoutH - 10.f)
        {
            auto& Seq = CompData.Seqs[CompData.Selected];
            //한글 텍스처 이름은 바이트를 그대로 넓히면 깨진다. 코드페이지를 거쳐야 한다.
            std::wstring WideTextureName = Seq.TextureName.empty()
                ? L"(없음)" : DialogUtil::ToWide(Seq.TextureName);
            TCHAR TextureBuffer[128];
            swprintf_s(TextureBuffer, 128, L"Tex: %s", WideTextureName.c_str());
            float LabelW = PanelW * 0.55f - 4.f;
            MakeLabel("TextureNameLabel_" + std::to_string(mDynIdx++), 6.f, Y, LabelW, ROW_H, TextureBuffer, 9.f);
            float ButtonX = 6.f + LabelW;
            auto TexButton = MakeButton("SetTextureButton_" + std::to_string(mDynIdx), ButtonX, Y + 1.f, PanelW - ButtonX - 6.f, ROW_H - 2.f, 0.18f, 0.20f, 0.38f);
            MakeLabel("SetTextureButtonLabel_" + std::to_string(mDynIdx++), ButtonX, Y + 1.f, PanelW - ButtonX - 6.f, ROW_H - 2.f, TEXT("Set Texture"), 9.f, ETextAlignH::Center);
            mSetTextureButton = TexButton;
            Y += ROW_H + 2.f;
        }

        // ── TextureType 토글 (Frame / SpriteSheet) ────────────────────────────
        if (Y + ROW_H < LayoutH - 10.f)
        {
            auto& Seq = CompData.Seqs[CompData.Selected];
            bool bFrame = (Seq.TextureType == EAnimation2DTextureType::Frame);
            MakeLabel("TextureTypeLabel_" + std::to_string(mDynIdx++), 6.f, Y, 52.f, ROW_H, TEXT("Type:"), 11.f);
            auto TypeButton = MakeButton("TextureTypeButton_" + std::to_string(mDynIdx), 60.f, Y + 1.f, PanelW - 68.f, ROW_H - 2.f,
                bFrame ? 0.30f : 0.18f, 0.20f, bFrame ? 0.18f : 0.30f);
            MakeLabel("TextureTypeButtonLabel_" + std::to_string(mDynIdx++), 60.f, Y + 1.f, PanelW - 68.f, ROW_H - 2.f,
                bFrame ? TEXT("Frame") : TEXT("SpriteSheet"), 11.f, ETextAlignH::Center);
            mTypeToggleButton = TypeButton;
            Y += ROW_H + 4.f;
        }

        // ── Sprite Viewer 열기 ────────────────────────────────────────────────
        if (Y + ROW_H < LayoutH - 10.f)
        {
            auto ViewButton = MakeButton("OpenSpriteViewerButton_" + std::to_string(mDynIdx), 4.f, Y, PanelW - 8.f, ROW_H,
                0.14f, 0.28f, 0.20f);
            MakeLabel("OpenSpriteViewerLabel_" + std::to_string(mDynIdx++), 4.f, Y, PanelW - 8.f, ROW_H,
                TEXT("Open Sprite Viewer"), 11.f, ETextAlignH::Center);
            mOpenViewerButton = ViewButton;
            Y += ROW_H + 4.f;
        }

        // ── 프레임 에디터 토글 ────────────────────────────────────────────────
        if (Y + ROW_H < LayoutH - 10.f)
        {
            auto& Seq = CompData.Seqs[CompData.Selected];
            TCHAR FrameToggleBuffer[64];
            swprintf_s(FrameToggleBuffer, 64, L"%s Frames (%d)",
                mShowFrameEditor ? L"▼" : L"▶", (int)Seq.Frames.size());

            auto TogButton = MakeButton("FrameEditorToggleButton_" + std::to_string(mDynIdx), 4.f, Y, PanelW - 8.f, ROW_H,
                0.16f, 0.26f, 0.35f);
            MakeLabel("FrameEditorToggleLabel_" + std::to_string(mDynIdx++), 4.f, Y, PanelW - 8.f, ROW_H,
                FrameToggleBuffer, 11.f, ETextAlignH::Center);
            mToggleFrameButton = TogButton;
            Y += ROW_H + 4.f;
        }

        // ── 프레임 에디터 본문 ────────────────────────────────────────────────
        if (mShowFrameEditor && Y + ROW_H < LayoutH - 10.f)
        {
            auto& Seq = CompData.Seqs[CompData.Selected];
            int FrameCount = (int)Seq.Frames.size();
            int SelFrame = Seq.SelectedFrame;

            // 프레임 인덱스 내비게이션 [< 프레임 X/N >]
            {
                auto PrevButton = MakeButton("FramePrevButton_" + std::to_string(mDynIdx), 4.f, Y, 24.f, ROW_H, 0.20f, 0.20f, 0.28f);
                MakeLabel("FramePrevButtonLabel_" + std::to_string(mDynIdx++), 4.f, Y, 24.f, ROW_H, TEXT("<"), 12.f, ETextAlignH::Center);
                mFramePrevButton = PrevButton;

                TCHAR FrameBuffer[64];
                if (FrameCount > 0)
                    swprintf_s(FrameBuffer, 64, L"Frame %d / %d", SelFrame + 1, FrameCount);
                else
                    wcscpy_s(FrameBuffer, 64, L"(no frames)");
                mFrameIdxLabel = MakeLabel("FrameIndexLabel_" + std::to_string(mDynIdx++),
                    30.f, Y, PanelW - 60.f, ROW_H, FrameBuffer, 10.f, ETextAlignH::Center);

                auto NextButton = MakeButton("FrameNextButton_" + std::to_string(mDynIdx), PanelW - 28.f, Y, 24.f, ROW_H, 0.20f, 0.20f, 0.28f);
                MakeLabel("FrameNextButtonLabel_" + std::to_string(mDynIdx++), PanelW - 28.f, Y, 24.f, ROW_H, TEXT(">"), 12.f, ETextAlignH::Center);
                mFrameNextButton = NextButton;
                Y += ROW_H + 2.f;
            }

            // 선택된 프레임 속성
            if (SelFrame >= 0 && SelFrame < FrameCount && Y + 6 * (ROW_H + 2.f) < LayoutH)
            {
                AddFrameFieldRow(Y, TEXT("Start.X"), 1.f,
                    [](FFrameData& Frame) -> float& { return Frame.Start.x; }, -100000.f);

                AddFrameFieldRow(Y, TEXT("Start.Y"), 1.f,
                    [](FFrameData& Frame) -> float& { return Frame.Start.y; }, -100000.f);

                AddFrameFieldRow(Y, TEXT("Size.W"), 1.f,
                    [](FFrameData& Frame) -> float& { return Frame.Size.x; }, 1.f);

                AddFrameFieldRow(Y, TEXT("Size.H"), 1.f,
                    [](FFrameData& Frame) -> float& { return Frame.Size.y; }, 1.f);

                AddFrameFieldRow(Y, TEXT("Offset.X"), 0.5f,
                    [](FFrameData& Frame) -> float& { return Frame.Offset.x; }, -100000.f);

                AddFrameFieldRow(Y, TEXT("Offset.Y"), 0.5f,
                    [](FFrameData& Frame) -> float& { return Frame.Offset.y; }, -100000.f);

                // 이 프레임이 머무는 시간(초). 시퀀스 총 재생 시간은 이 값들의 합이다.
                // 0.01초 단위로 조절하므로 소수 셋째 자리까지 보여준다.
                AddFrameFieldRow(Y, TEXT("Dur(s)"), 0.01f,
                    [](FFrameData& Frame) -> float& { return Frame.Duration; }, 0.001f, 3);
            }

            // 추가 / 삭제 / 전체 삭제 버튼 (3열)
            if (Y + ROW_H < LayoutH - 10.f)
            {
                float ButtonW = (PanelW - 16.f) / 3.f;

                auto AddFButton = MakeButton("AddFrameButton_" + std::to_string(mDynIdx), 4.f, Y, ButtonW, ROW_H, 0.14f, 0.30f, 0.14f);
                MakeLabel("AddFrameButtonLabel_" + std::to_string(mDynIdx++), 4.f, Y, ButtonW, ROW_H, TEXT("+ Frame"), 10.f, ETextAlignH::Center);
                mAddFrameButton = AddFButton;

                float DelButtonX = 4.f + ButtonW + 4.f;
                bool bCanDel = !Seq.Frames.empty() && SelFrame >= 0 && SelFrame < FrameCount;
                auto DelFButton = MakeButton("DeleteFrameButton_" + std::to_string(mDynIdx), DelButtonX, Y, ButtonW, ROW_H, bCanDel ? 0.35f : 0.20f, 0.14f, 0.14f);
                MakeLabel("DeleteFrameButtonLabel_" + std::to_string(mDynIdx++), DelButtonX, Y, ButtonW, ROW_H, TEXT("- Del"), 10.f, ETextAlignH::Center);
                mDelFrameButton = DelFButton;

                float ClearButtonX = DelButtonX + ButtonW + 4.f;
                auto ClrButton = MakeButton("ClearFramesButton_" + std::to_string(mDynIdx), ClearButtonX, Y, ButtonW, ROW_H, 0.25f, 0.10f, 0.10f);
                MakeLabel("ClearFramesButtonLabel_" + std::to_string(mDynIdx++), ClearButtonX, Y, ButtonW, ROW_H, TEXT("Clear"), 10.f, ETextAlignH::Center);
                mClearFramesButton = ClrButton;
                Y += ROW_H + 4.f;
            }
        }

        // ── 저장 / 불러오기 ──────────────────────────────────────────────────
        if (Y + ROW_H < LayoutH - 4.f)
        {
            float HalfW = (PanelW - 12.f) / 2.f;
            auto SaveButton = MakeButton("SaveAnimButton_" + std::to_string(mDynIdx), 4.f, Y, HalfW, ROW_H, 0.16f, 0.30f, 0.16f);
            MakeLabel("SaveAnimButtonLabel_" + std::to_string(mDynIdx++), 4.f, Y, HalfW, ROW_H, TEXT("Save Anim"), 10.f, ETextAlignH::Center);
            mSaveAnimButton = SaveButton;

            float LoadButtonX = 4.f + HalfW + 4.f;
            auto LoadButton = MakeButton("LoadAnimButton_" + std::to_string(mDynIdx), LoadButtonX, Y, HalfW, ROW_H, 0.16f, 0.22f, 0.35f);
            MakeLabel("LoadAnimButtonLabel_" + std::to_string(mDynIdx++), LoadButtonX, Y, HalfW, ROW_H, TEXT("Load Anim"), 10.f, ETextAlignH::Center);
            mLoadAnimButton = LoadButton;
            Y += ROW_H + 4.f;
        }
    }

    // ── Add Animation 토글 버튼 ───────────────────────────────────────────────
    if (Y + ROW_H < LayoutH - 10.f)
    {
        auto AddButton = MakeButton("AddAnimToggleButton_" + std::to_string(mDynIdx), 4.f, Y, PanelW - 8.f, ROW_H,
            0.14f, 0.30f, 0.14f);
        MakeLabel("AddAnimToggleLabel_" + std::to_string(mDynIdx++), 4.f, Y, PanelW - 8.f, ROW_H,
            mShowRegistry ? TEXT("▲ 닫기") : TEXT("+ 애니메이션 추가"),
            11.f, ETextAlignH::Center);
        mAddToggleButton = AddButton;
        Y += ROW_H + 4.f;
    }

    // ── 레지스트리 목록 ───────────────────────────────────────────────────────
    if (mShowRegistry)
    {
        for (auto& AnimName : CAnimRegistry::GetAll())
        {
            if (Y + ROW_H > LayoutH - 6.f) break;
            std::wstring WName(AnimName.begin(), AnimName.end());
            auto Button = MakeButton("RegistryItemButton_" + std::to_string(mDynIdx), 8.f, Y, PanelW - 16.f, ROW_H, 0.14f, 0.22f, 0.32f);
            MakeLabel("RegistryItemLabel_" + std::to_string(mDynIdx++), 12.f, Y, PanelW - 20.f, ROW_H, WName.c_str(), 10.f);
            mRegButtons.push_back({ Button, AnimName });
            Y += ROW_H + 2.f;
        }

        if (CAnimRegistry::GetAll().empty())
        {
            MakeLabel("RegistryEmptyLabel_" + std::to_string(mDynIdx++), 8.f, Y, PanelW - 16.f, ROW_H,
                TEXT("(등록된 애니메이션 없음)"), 10.f);
            Y += ROW_H + 2.f;
        }
    }

    FinishLayout(Y);
}

void CAnimEditorUI::FinishLayout(float ContentEndY)
{
    // Rebuild가 mChildList를 [정적 위젯..., 이번에 만든 동적 위젯...] 순으로 채워놓은
    // 직후에만 호출된다. (렌더/충돌에서 정렬되기 전이라 인덱스를 믿을 수 있다)
    size_t StaticCount = mStaticWidgets.empty()
        ? (size_t)mStaticChildCount
        : mStaticWidgets.size();

    for (size_t i = StaticCount; i < mChildList.size(); ++i)
    {
        mChildList[i]->SetScrollTarget(true);
    }

    // 리사이즈될 때마다 Rebuild되므로 스크롤 영역도 여기서 같이 갱신한다.
    SetScrollArea(TITLE_H, GetSize().y);
    SetScrollContentEnd(ContentEndY + 6.f);
}

// ── 업데이트 ─────────────────────────────────────────────────────────────────

// ── 값 직접 입력 (더블클릭 → 타이핑 → Enter) ──────────────────────────────────

// 인풋은 등록된 키만 GetKey로 조회할 수 있으므로 사용할 키를 미리 바인딩한다.
// SetWorld가 Init 이후에 호출되므로 Init이 아니라 첫 Update에서 처리한다.
void CAnimEditorUI::RegisterEditKeys()
{
    if (mKeysRegistered) return;

    auto World = mWorld.lock();
    if (!World) return;

    auto Input = World->GetInput().lock();
    if (!Input) return;

    for (int Digit = 0; Digit < 10; ++Digit)
    {
        Input->AddBindKey("AnimNum" + std::to_string(Digit), (unsigned char)('0' + Digit));
        Input->AddBindKey("AnimPad" + std::to_string(Digit), (unsigned char)(VK_NUMPAD0 + Digit));
    }

    Input->AddBindKey("AnimDot",      VK_OEM_PERIOD);
    Input->AddBindKey("AnimPadDot",   VK_DECIMAL);
    Input->AddBindKey("AnimMinus",    VK_OEM_MINUS);
    Input->AddBindKey("AnimPadMinus", VK_SUBTRACT);
    Input->AddBindKey("AnimBack",     VK_BACK);
    Input->AddBindKey("AnimEnter",    VK_RETURN);
    Input->AddBindKey("AnimEsc",      VK_ESCAPE);

    mKeysRegistered = true;
}

CAnimEditorUI::FPropButton* CAnimEditorUI::GetEditProp()
{
    if (!mEditActive) return nullptr;

    std::vector<FPropButton>* List = nullptr;
    if (mEditList == 0)      List = &mPropButtons;
    else if (mEditList == 1) List = &mFramePropButtons;
    if (!List) return nullptr;

    if (mEditIdx < 0 || mEditIdx >= (int)List->size()) return nullptr;
    return &(*List)[mEditIdx];
}

void CAnimEditorUI::RefreshEditLabel()
{
    auto* Prop = GetEditProp();
    if (!Prop) return;

    auto Label = Prop->Label.lock();
    if (!Label) return;

    std::wstring WideBuffer(mEditBuffer.begin(), mEditBuffer.end());
    WideBuffer += L"_";   // 캐럿
    Label->SetText(WideBuffer.c_str());
    Label->SetTextColor(FVector4(0.55f, 1.f, 0.65f, 1.f));
}

void CAnimEditorUI::BeginEdit(int ListIdx, int RowIdx)
{
    std::vector<FPropButton>* List = nullptr;
    if (ListIdx == 0)      List = &mPropButtons;
    else if (ListIdx == 1) List = &mFramePropButtons;
    if (!List) return;

    if (RowIdx < 0 || RowIdx >= (int)List->size()) return;

    // 다른 행을 편집 중이었다면 그 행부터 원래 표시로 되돌린다.
    if (mEditActive) CancelEdit();

    mEditActive = true;
    mEditList   = ListIdx;
    mEditIdx    = RowIdx;

    // 현재 값을 초기 문자열로 채워준다. (바로 Enter를 치면 값이 유지된다)
    auto& Prop = (*List)[RowIdx];
    char TextBuffer[32] = {};
    float CurrentValue = Prop.Getter ? Prop.Getter() : 0.f;
    // 포맷 문자열은 리터럴로 넘긴다 (비리터럴 포맷은 C4774 경고 대상)
    if (ListIdx == 0) sprintf_s(TextBuffer, 32, "%.2f", CurrentValue);
    else              sprintf_s(TextBuffer, 32, "%.1f", CurrentValue);
    mEditBuffer = TextBuffer;

    // 타이핑하는 숫자키가 스킬/타일모드 등 기존 바인딩을 같이 발동시키지 않게 막는다.
    if (auto World = mWorld.lock())
        if (auto Input = World->GetInput().lock())
            Input->SetBindKeyBlock(true);

    RefreshEditLabel();
}

void CAnimEditorUI::CancelEdit()
{
    // 편집 중이던 행을 실제 값 표시로 되돌린다.
    if (auto* Prop = GetEditProp())
    {
        if (auto Label = Prop->Label.lock())
        {
            TCHAR TextBuffer[32];
            float CurrentValue = Prop->Getter ? Prop->Getter() : 0.f;
            if (mEditList == 0) swprintf_s(TextBuffer, 32, L"%.2f", CurrentValue);
            else                swprintf_s(TextBuffer, 32, L"%.1f", CurrentValue);
            Label->SetText(TextBuffer);
            Label->SetTextColor(mEditList == 0
                ? FVector4(0.9f, 0.95f, 1.f, 1.f)
                : FVector4(0.85f, 1.f, 0.85f, 1.f));
        }
    }

    mEditActive = false;
    mEditList   = -1;
    mEditIdx    = -1;
    mEditBuffer.clear();

    if (auto World = mWorld.lock())
        if (auto Input = World->GetInput().lock())
            Input->SetBindKeyBlock(false);
}

void CAnimEditorUI::CommitEdit()
{
    if (!mEditActive) return;

    int  ListIdx = mEditList;
    bool Applied = false;

    if (auto* Prop = GetEditProp())
    {
        // "-" 나 "." 만 남은 미완성 입력은 무시한다.
        if (Prop->Setter && !mEditBuffer.empty() && mEditBuffer != "-" && mEditBuffer != ".")
        {
            // All이 켜진 행이면 모든 프레임을 방금 친 값으로 맞춘다.
            bool bAll = Prop->SetAll && Prop->AllIdx >= 0 && Prop->AllIdx < FRAME_PROP_MAX
                     && mFramePropAll[Prop->AllIdx];

            try
            {
                float Value = std::stof(mEditBuffer);

                if (bAll) Prop->SetAll(Value, false);
                else      Prop->Setter(Value);

                Applied = true;
            }
            catch (...)
            {
                // 파싱 실패 시 값을 건드리지 않는다.
            }
        }
    }

    // 편집을 끝내면서 라벨을 새 값으로 되돌린다.
    CancelEdit();

    // 프레임 속성이면 +/- 와 동일하게 CAnimation2D / SpriteViewer에 즉시 반영한다.
    if (Applied && ListIdx == 1 && mActiveComp < (int)mComps.size())
    {
        auto& CompData = mComps[mActiveComp];
        if (CompData.Selected >= 0)
        {
            ApplyFrames(mActiveComp, CompData.Selected);
            SyncSpriteViewer(mActiveComp, CompData.Selected);
        }
    }
}

void CAnimEditorUI::HandleValueEditInput()
{
    if (!mEditActive) return;

    auto World = mWorld.lock();
    if (!World) { CancelEdit(); return; }

    auto Input = World->GetInput().lock();
    if (!Input) { CancelEdit(); return; }

    // 편집 중인 값 영역 밖을 클릭하면 취소한다.
    if (Input->GetMouseState(EMouseType::LButton, EInputType::Press))
    {
        std::shared_ptr<CButton> EditButton;
        if (auto* Prop = GetEditProp())
            EditButton = Prop->ValButton.lock();

        EWidgetState::Type State = EditButton ? EditButton->GetWidgetState() : EWidgetState::Normal;

        if (State != EWidgetState::Hovered && State != EWidgetState::Clicked && State != EWidgetState::Release)
        {
            CancelEdit();
            return;
        }
    }

    // 확정 / 취소
    if (Input->GetKey(VK_RETURN, EInputType::Press)) { CommitEdit(); return; }
    if (Input->GetKey(VK_ESCAPE, EInputType::Press)) { CancelEdit(); return; }

    // 한 글자 지우기
    if (Input->GetKey(VK_BACK, EInputType::Press) && !mEditBuffer.empty())
        mEditBuffer.pop_back();

    // 숫자 (상단 숫자열 + 넘패드)
    for (int Digit = 0; Digit < 10; ++Digit)
    {
        bool bPressed = Input->GetKey((unsigned char)('0' + Digit), EInputType::Press)
                     || Input->GetKey((unsigned char)(VK_NUMPAD0 + Digit), EInputType::Press);

        if (bPressed && (int)mEditBuffer.size() < EDIT_BUF_MAX)
            mEditBuffer.push_back((char)('0' + Digit));
    }

    // 소수점 — 하나만 허용
    if (Input->GetKey(VK_OEM_PERIOD, EInputType::Press) || Input->GetKey(VK_DECIMAL, EInputType::Press))
    {
        if (mEditBuffer.find('.') == std::string::npos && (int)mEditBuffer.size() < EDIT_BUF_MAX)
            mEditBuffer.push_back('.');
    }

    // 부호 — 맨 앞에서만, 누를 때마다 토글
    if (Input->GetKey(VK_OEM_MINUS, EInputType::Press) || Input->GetKey(VK_SUBTRACT, EInputType::Press))
    {
        if (!mEditBuffer.empty() && mEditBuffer[0] == '-')
            mEditBuffer.erase(mEditBuffer.begin());
        else
            mEditBuffer.insert(mEditBuffer.begin(), '-');
    }
}

void CAnimEditorUI::DetectValueDoubleClick()
{
    std::vector<FPropButton>* Lists[2] = { &mPropButtons, &mFramePropButtons };

    for (int ListIdx = 0; ListIdx < 2; ++ListIdx)
    {
        auto& List = *Lists[ListIdx];

        for (int i = 0; i < (int)List.size(); ++i)
        {
            auto Button = List[i].ValButton.lock();
            if (!Button) continue;
            if (Button->GetWidgetState() != EWidgetState::Release) continue;

            void* ClickKey = Button.get();

            // 같은 버튼을 짧은 간격으로 두 번 → 편집 시작
            if (mLastClickKey == ClickKey && (mTimeAccum - mLastClickTime) <= DOUBLE_CLICK_SEC)
            {
                BeginEdit(ListIdx, i);
                mLastClickKey = nullptr;
            }
            else
            {
                mLastClickKey  = ClickKey;
                mLastClickTime = mTimeAccum;
            }

            return;
        }
    }
}

void CAnimEditorUI::Update(float DeltaTime)
{
    mTimeAccum += DeltaTime;
    RegisterEditKeys();

    // mComps가 actor 실제 상태와 다르면 자동 갱신
    {
        auto TargetActor = mTarget.lock();
        int ActualAnimCompCount = 0;
        if (TargetActor)
        {
            for (auto& Comp : TargetActor->GetActorCompList())
                if (std::dynamic_pointer_cast<CAnimation2DComponent>(Comp))
                    ++ActualAnimCompCount;
        }
        bool NeedsRefresh = (ActualAnimCompCount != (int)mComps.size());
        if (!NeedsRefresh)
        {
            for (auto& CompData : mComps)
                if (CompData.Comp.expired()) { NeedsRefresh = true; break; }
        }
        if (NeedsRefresh)
        {
            SetTarget(mTarget);
        }
    }

    CWidgetContainer::Update(DeltaTime);

    auto World = mWorld.lock();
    if (!World) return;

    // ── 리사이즈 (코너 드래그) ────────────────────────────────────────────────
    if (auto Input = World->GetInput().lock())
    {
        FVector2 Mouse   = Input->GetMousePos();
        FVector2 Delta   = Input->GetMouseMove();
        bool Press   = Input->GetMouseState(EMouseType::LButton, EInputType::Press);
        bool Held    = Input->GetMouseState(EMouseType::LButton, EInputType::Hold);
        bool Release = Input->GetMouseState(EMouseType::LButton, EInputType::Release);

        FVector3 PanelPos = GetPos(), PanelSize = GetSize();
        float PanelWidth = PanelSize.x, PanelHeight = PanelSize.y;
        float CornerX[4] = { PanelPos.x, PanelPos.x + PanelWidth - HANDLE_SZ, PanelPos.x, PanelPos.x + PanelWidth - HANDLE_SZ };
        float CornerY[4] = { PanelPos.y, PanelPos.y, PanelPos.y + PanelHeight - HANDLE_SZ, PanelPos.y + PanelHeight - HANDLE_SZ };

        if (mActiveCorner == -1 && Press)
            for (int i = 0; i < 4; ++i)
                if (Mouse.x >= CornerX[i] && Mouse.x < CornerX[i] + HANDLE_SZ &&
                    Mouse.y >= CornerY[i] && Mouse.y < CornerY[i] + HANDLE_SZ)
                { mActiveCorner = i; break; }

        if (mActiveCorner >= 0 && Release) mActiveCorner = -1;

        if (mActiveCorner >= 0 && Held && (Delta.x != 0.f || Delta.y != 0.f))
        {
            float NewX = PanelPos.x, NewY = PanelPos.y, NewWidth = PanelWidth, NewHeight = PanelHeight;
            switch (mActiveCorner)
            {
            case 0: NewX += Delta.x; NewY += Delta.y; NewWidth -= Delta.x; NewHeight -= Delta.y; break;
            case 1:                NewY += Delta.y; NewWidth += Delta.x; NewHeight -= Delta.y; break;
            case 2: NewX += Delta.x;                NewWidth -= Delta.x; NewHeight += Delta.y; break;
            case 3:                                NewWidth += Delta.x; NewHeight += Delta.y; break;
            }
            NewWidth = max(NewWidth, 220.f); NewHeight = max(NewHeight, 200.f);
            SetPos(NewX, NewY); SetSize(NewWidth, NewHeight);
            UpdateHandles(NewWidth, NewHeight);
            Rebuild();
            return;
        }
    }

    // ── 값 직접 입력 (더블클릭 → 타이핑 → Enter) ────────────────────────────
    HandleValueEditInput();

    if (mEditActive)
    {
        // 편집 중에는 버퍼를 그대로 보여주고, 아래의 다른 버튼 처리는 건너뛴다.
        RefreshEditLabel();
        return;
    }

    DetectValueDoubleClick();
    if (mEditActive) return;

    // ── 탭 버튼 ──────────────────────────────────────────────────────────────
    for (auto& Tab : mTabButtons)
    {
        if (auto Button = Tab.Button.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
        {
            mActiveComp = Tab.Idx;
            Rebuild();
            return;
        }
    }

    // ── 시퀀스 버튼 (클릭 → 재생) ────────────────────────────────────────────
    for (auto& SeqEntry : mSeqButtons)
    {
        if (auto Button = SeqEntry.Button.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
        {
            PlaySeq(mActiveComp, SeqEntry.Idx);
            return;
        }
    }

    // ── 시퀀스 속성 증감 (PlayTime / PlayRate / Pivot) ───────────────────────
    bool AnySeqPropChanged = false;

    for (auto& Prop : mPropButtons)
    {
        bool Changed = false;

        if (auto Button = Prop.Minus.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
        { Prop.Setter(Prop.Getter() - Prop.Step); Changed = true; }

        if (auto Button = Prop.Plus.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
        { Prop.Setter(Prop.Getter() + Prop.Step); Changed = true; }

        if (Changed)
        {
            if (auto Label = Prop.Label.lock())
            {
                TCHAR TextBuffer[32]; swprintf_s(TextBuffer, 32, L"%.2f", Prop.Getter());
                Label->SetText(TextBuffer);
            }
            AnySeqPropChanged = true;
        }
    }

    // Pivot.X / Pivot.Y가 여기 섞여 있으므로 뷰어의 기준선도 같이 갱신한다.
    if (AnySeqPropChanged && mActiveComp < (int)mComps.size())
    {
        auto& CompData = mComps[mActiveComp];
        if (CompData.Selected >= 0)
            SyncSpriteViewer(mActiveComp, CompData.Selected);
    }

    // ── 토글 (Loop / Reverse / Symmetry) ────────────────────────────────────
    for (auto& Toggle : mToggleButtons)
    {
        if (auto Button = Toggle.Button.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
        {
            bool NewVal = !Toggle.Getter();
            Toggle.Setter(NewVal);

            if (auto Label = Toggle.Label.lock()) Label->SetText(NewVal ? TEXT("ON") : TEXT("OFF"));

            Button->SetTint(EWidgetState::Normal,
                NewVal ? 0.12f : 0.22f,
                NewVal ? 0.38f : 0.22f,
                NewVal ? 0.12f : 0.28f, 1.f);
        }
    }

    // ── 프레임 에디터 토글 ────────────────────────────────────────────────────
    if (auto Button = mToggleFrameButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        mShowFrameEditor = !mShowFrameEditor;
        Rebuild();
        return;
    }

    // ── 프레임 Prev / Next ────────────────────────────────────────────────────
    if (auto Button = mFramePrevButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
            {
                auto& Seq = CompData.Seqs[CompData.Selected];
                if (!Seq.Frames.empty())
                {
                    Seq.SelectedFrame = (Seq.SelectedFrame - 1 + (int)Seq.Frames.size()) % (int)Seq.Frames.size();
                    if (auto Viewer = mSpriteViewer.lock()) Viewer->SelectFrame(Seq.SelectedFrame);
                    Rebuild(); return;
                }
            }
        }
    }
    if (auto Button = mFrameNextButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
            {
                auto& Seq = CompData.Seqs[CompData.Selected];
                if (!Seq.Frames.empty())
                {
                    Seq.SelectedFrame = (Seq.SelectedFrame + 1) % (int)Seq.Frames.size();
                    if (auto Viewer = mSpriteViewer.lock()) Viewer->SelectFrame(Seq.SelectedFrame);
                    Rebuild(); return;
                }
            }
        }
    }

    // ── 프레임 속성 All 체크 ────────────────────────────────────────────────
    // 켜두면 아래 +/- 와 직접 입력이 선택 프레임 하나가 아니라 모든 프레임에 들어간다.
    for (auto& Prop : mFramePropButtons)
    {
        auto Button = Prop.AllButton.lock();

        if (!Button || Button->GetWidgetState() != EWidgetState::Release) continue;
        if (Prop.AllIdx < 0 || Prop.AllIdx >= FRAME_PROP_MAX)       continue;

        bool bOn = !mFramePropAll[Prop.AllIdx];
        mFramePropAll[Prop.AllIdx] = bOn;

        Button->SetTint(EWidgetState::Normal,
            bOn ? 0.16f : 0.20f,
            bOn ? 0.40f : 0.20f,
            bOn ? 0.16f : 0.24f, 1.f);

        if (auto Label = Prop.AllLabel.lock())
            Label->SetTextColor(bOn ? FVector4(0.6f, 1.f, 0.6f, 1.f) : FVector4(0.55f, 0.55f, 0.6f, 1.f));
    }

    // ── 프레임 속성 +/- (변경 즉시 CAnimation2D에 적용) ─────────────────────
    {
        bool AnyChanged = false;
        for (auto& Prop : mFramePropButtons)
        {
            bool Changed = false;

            // All이 켜져 있으면 모든 프레임을 Step만큼 같이 밀어준다.
            bool bAll = Prop.SetAll && Prop.AllIdx >= 0 && Prop.AllIdx < FRAME_PROP_MAX
                     && mFramePropAll[Prop.AllIdx];

            if (auto Button = Prop.Minus.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
            {
                if (bAll) Prop.SetAll(-Prop.Step, true);
                else      Prop.Setter(Prop.Getter() - Prop.Step);
                Changed = true;
            }

            if (auto Button = Prop.Plus.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
            {
                if (bAll) Prop.SetAll(Prop.Step, true);
                else      Prop.Setter(Prop.Getter() + Prop.Step);
                Changed = true;
            }

            if (Changed)
            {
                if (auto Label = Prop.Label.lock())
                {
                    TCHAR Format[8]; swprintf_s(Format, 8, L"%%.%df", Prop.Decimals);
                    TCHAR TextBuffer[32]; swprintf_s(TextBuffer, 32, Format, Prop.Getter());
                    Label->SetText(TextBuffer);
                }
                AnyChanged = true;
            }
        }
        if (AnyChanged && mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0)
            {
                ApplyFrames(mActiveComp, CompData.Selected);
                SyncSpriteViewer(mActiveComp, CompData.Selected);

                // Dur(s)를 만졌으면 총 재생 시간 표시도 따라가야 한다.
                if (auto TotalLabel = mPlayTimeText.lock())
                {
                    float Total = 0.f;
                    for (const auto& FrameData : CompData.Seqs[CompData.Selected].Frames)
                        Total += FrameData.Duration;

                    TCHAR TextBuffer[32]; swprintf_s(TextBuffer, 32, L"%.2f s", Total);
                    TotalLabel->SetText(TextBuffer);
                }
            }
        }
    }

    // ── 프레임 추가 ──────────────────────────────────────────────────────────
    if (auto Button = mAddFrameButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
            {
                auto& Seq = CompData.Seqs[CompData.Selected];

                // 선택한 프레임을 복사해서 그 "바로 뒤"에 끼워 넣는다.
                // 맨 뒤에 붙이면 중간에 한 장을 더 넣고 싶을 때 매번 옮겨야 해서,
                // 지금 보고 있는 자리에서 늘어나도록 insert를 쓴다.
                // 선택이 없거나 비어 있으면 맨 뒤가 곧 끼워 넣을 자리다.
                FFrameData NewFrame;
                int InsertAt = (int)Seq.Frames.size();

                if (!Seq.Frames.empty() && Seq.SelectedFrame >= 0 && Seq.SelectedFrame < (int)Seq.Frames.size())
                {
                    NewFrame = Seq.Frames[Seq.SelectedFrame]; // 현재 프레임 복사
                    InsertAt = Seq.SelectedFrame + 1;
                }

                Seq.Frames.insert(Seq.Frames.begin() + InsertAt, NewFrame);

                // 방금 끼워 넣은 프레임을 그대로 선택해 둔다. (이어서 바로 편집할 수 있게)
                Seq.SelectedFrame = InsertAt;

                ApplyFrames(mActiveComp, CompData.Selected);
                SyncSpriteViewer(mActiveComp, CompData.Selected);
                Rebuild(); return;
            }
        }
    }

    // ── 선택 프레임 삭제 ─────────────────────────────────────────────────────
    if (auto Button = mDelFrameButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
            {
                auto& Seq = CompData.Seqs[CompData.Selected];
                int SelFrame = Seq.SelectedFrame;
                if (SelFrame >= 0 && SelFrame < (int)Seq.Frames.size())
                {
                    Seq.Frames.erase(Seq.Frames.begin() + SelFrame);
                    // 삭제 후 인덱스 보정
                    if (Seq.Frames.empty())
                        Seq.SelectedFrame = -1;
                    else if (SelFrame >= (int)Seq.Frames.size())
                        Seq.SelectedFrame = (int)Seq.Frames.size() - 1;
                    ApplyFrames(mActiveComp, CompData.Selected);
                    SyncSpriteViewer(mActiveComp, CompData.Selected);
                    Rebuild(); return;
                }
            }
        }
    }

    // ── 전체 프레임 삭제 ─────────────────────────────────────────────────────
    if (auto Button = mClearFramesButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
            {
                CompData.Seqs[CompData.Selected].Frames.clear();
                CompData.Seqs[CompData.Selected].SelectedFrame = -1;
                ApplyFrames(mActiveComp, CompData.Selected);
                SyncSpriteViewer(mActiveComp, CompData.Selected);
                Rebuild(); return;
            }
        }
    }

    // ── 새 애니메이션 만들기 ──────────────────────────────────────────────────
    if (auto Button = mNewAnimButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        CreateNewAnim();
        return;
    }

    // ── 텍스처 설정 ───────────────────────────────────────────────────────────
    if (auto Button = mSetTextureButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0)
                SetAnimTexture(mActiveComp, CompData.Selected);
        }
    }

    // ── TextureType 토글 ──────────────────────────────────────────────────────
    if (auto Button = mTypeToggleButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
            {
                auto& Seq = CompData.Seqs[CompData.Selected];
                Seq.TextureType = (Seq.TextureType == EAnimation2DTextureType::Frame)
                    ? EAnimation2DTextureType::SpriteSheet
                    : EAnimation2DTextureType::Frame;

                auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
                if (AnimManager) AnimManager->SetAnimationTextureType(Seq.Name, Seq.TextureType);

                Rebuild(); return;
            }
        }
    }

    // ── Add Animation 토글 ────────────────────────────────────────────────────
    if (auto Button = mAddToggleButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        mShowRegistry = !mShowRegistry;
        Rebuild();
        return;
    }

    // ── 레지스트리에서 선택 ───────────────────────────────────────────────────
    for (auto& RegEntry : mRegButtons)
    {
        if (auto Button = RegEntry.Button.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
        {
            AddSeq(mActiveComp, RegEntry.Name);
            return;
        }
    }

    // ── Sprite Viewer 열기 ────────────────────────────────────────────────────
    if (auto Button = mOpenViewerButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
                OpenSpriteViewer(mActiveComp, CompData.Selected);
        }
    }

    // ── 애니메이션 저장 ──────────────────────────────────────────────────────
    if (auto Button = mSaveAnimButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CompData = mComps[mActiveComp];
            SaveAnim(mActiveComp, CompData.Selected);
        }
    }

    // ── 애니메이션 불러오기 ──────────────────────────────────────────────────
    if (auto Button = mLoadAnimButton.lock(); Button && Button->GetWidgetState() == EWidgetState::Release)
    {
        LoadAnim();
        return;
    }

    // ── 프레임 인덱스 라벨 ───────────────────────────────────────────────────
    // 편집 중인 SelectedFrame 하나만 보여준다.
    // 재생 중인 프레임을 여기 섞으면 매 프레임 표시가 바뀌어 깜빡이고,
    // 재생 헤드로 덮어쓰면 < > 로 옮긴 선택이 곧바로 되돌아간 것처럼 보인다.
    if (mActiveComp < (int)mComps.size())
    {
        auto& CompData = mComps[mActiveComp];
        if (CompData.Selected >= 0 && CompData.Selected < (int)CompData.Seqs.size())
        {
            if (auto IdxLabel = mFrameIdxLabel.lock())
            {
                auto& Seq       = CompData.Seqs[CompData.Selected];
                int   FrameCount = (int)Seq.Frames.size();
                int   SelFrame   = Seq.SelectedFrame;

                TCHAR TextBuffer[64];
                if (FrameCount <= 0)
                    wcscpy_s(TextBuffer, 64, L"(no frames)");
                else if (SelFrame < 0 || SelFrame >= FrameCount)
                    swprintf_s(TextBuffer, 64, L"Frame - / %d", FrameCount);
                else
                    swprintf_s(TextBuffer, 64, L"Frame %d / %d", SelFrame + 1, FrameCount);

                IdxLabel->SetText(TextBuffer);
            }
        }
    }
}

CAnimEditorUI* CAnimEditorUI::Clone()
{
    return new CAnimEditorUI(*this);
}

// ── SpriteViewer 연동 ─────────────────────────────────────────────────────────

void CAnimEditorUI::SyncSpriteViewer(int CompIdx, int SeqIdx)
{
    auto Viewer = mSpriteViewer.lock();
    if (!Viewer || !Viewer->IsEnable()) return;
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    if (SeqIdx < 0 || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;

    auto& Seq = mComps[CompIdx].Seqs[SeqIdx];
    std::vector<CSpriteViewerUI::FFrameRect> Rects;
    for (auto& FrameData : Seq.Frames)
        Rects.push_back({ FrameData.Start, FrameData.Size });

    Viewer->SetFrames(Rects, Seq.SelectedFrame);
    Viewer->SetPivot(Seq.PivotX, Seq.PivotY);
}

void CAnimEditorUI::OpenSpriteViewer(int CompIdx, int SeqIdx)
{
    auto Viewer = mSpriteViewer.lock();
    if (!Viewer) return;
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    if (SeqIdx < 0 || SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;

    auto& Seq = mComps[CompIdx].Seqs[SeqIdx];

    // ── 텍스처 확보 ───────────────────────────────────────────────────────────
    // 뷰어는 텍스처를 "이름"으로 찾아 쓴다. 이름이 비어 있거나 아직 로드되지 않았으면
    // 흰 사각형만 나오므로, 여기서 끝까지 되살려본다.
    FVector2 TexSize = { 256.f, 256.f };

    auto AnimManager = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (AnimManager)
    {
        auto Anim = AnimManager->FindAnimation(Seq.Name).lock();
        if (Anim)
        {
            auto Texture = Anim->GetTexture().lock();

            // 아직 안 올라와 있으면 저장해둔 경로로 다시 로드한다.
            if (!Texture && !Seq.TextureRelPath.empty())
            {
                std::string TexName = Seq.TextureName.empty()
                    ? DialogUtil::ExtractBaseName(Seq.TextureRelPath)
                    : Seq.TextureName;

                std::string FullPath = DialogUtil::GetExeDir() + Seq.TextureRelPath;
                Anim->SetTextureFullPath(TexName, DialogUtil::ToWide(FullPath).c_str());
                Texture = Anim->GetTexture().lock();

                LOG_DEBUG("[AnimEditor] Texture reloaded for viewer: %s", FullPath.c_str());
            }

            if (Texture)
            {
                // 이름이 비어 있으면 여기서 채워준다. (없으면 뷰어가 텍스처를 못 찾는다)
                if (Seq.TextureName.empty())
                    Seq.TextureName = StripTexturePrefix(Texture->GetName());

                auto Info = Texture->GetTexture(0);
                if (Info && Info->Width > 0)
                    TexSize = { (float)Info->Width, (float)Info->Height };
            }
            else
            {
                LOG_DEBUG("[AnimEditor] Texture missing for anim: %s", Seq.Name.c_str());
            }
        }
    }

    Viewer->ShowTexture(Seq.TextureName, TexSize);

    // 현재 프레임 데이터 전달
    std::vector<CSpriteViewerUI::FFrameRect> Rects;
    for (auto& FrameData : Seq.Frames)
        Rects.push_back({ FrameData.Start, FrameData.Size });
    Viewer->SetFrames(Rects, Seq.SelectedFrame);

    // 콜백: SpriteViewer → AnimEditorUI 동기화
    Viewer->SetOnFrameChanged([this, CompIdx, SeqIdx](int FrameIdx, FVector2 Start, FVector2 Size)
    {
        if (CompIdx >= (int)mComps.size()) return;
        if (SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
        auto& Seq = mComps[CompIdx].Seqs[SeqIdx];
        if (FrameIdx < 0 || FrameIdx >= (int)Seq.Frames.size()) return;
        Seq.Frames[FrameIdx].Start = Start;
        Seq.Frames[FrameIdx].Size  = Size;
        ApplyFrames(CompIdx, SeqIdx);
        Rebuild();
    });
        
    Viewer->SetOnFrameSelected([this, CompIdx, SeqIdx](int FrameIdx)
    {
        if (CompIdx >= (int)mComps.size()) return;
        if (SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
        mComps[CompIdx].Seqs[SeqIdx].SelectedFrame = FrameIdx;
        Rebuild();
    });

    Viewer->SetOnFrameDeleted([this, CompIdx, SeqIdx](int FrameIdx)
    {
        if (CompIdx >= (int)mComps.size()) return;
        if (SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
        auto& Seq = mComps[CompIdx].Seqs[SeqIdx];
        if (FrameIdx < 0 || FrameIdx >= (int)Seq.Frames.size()) return;
        Seq.Frames.erase(Seq.Frames.begin() + FrameIdx);
        if (Seq.Frames.empty())
            Seq.SelectedFrame = -1;
        else if (Seq.SelectedFrame >= (int)Seq.Frames.size())
            Seq.SelectedFrame = (int)Seq.Frames.size() - 1;
        ApplyFrames(CompIdx, SeqIdx);
        Rebuild();
        // SpriteViewer에 다시 동기화
        if (auto Viewer2 = mSpriteViewer.lock())
        {
            std::vector<CSpriteViewerUI::FFrameRect> Rects;
            for (auto& FrameData : Seq.Frames) Rects.push_back({ FrameData.Start, FrameData.Size });
            Viewer2->SetFrames(Rects, Seq.SelectedFrame);
        }
    });

    Viewer->SetOnFrameAdded([this, CompIdx, SeqIdx](FVector2 Start, FVector2 Size)
    {
        if (CompIdx >= (int)mComps.size()) return;
        if (SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;
        auto& Seq = mComps[CompIdx].Seqs[SeqIdx];
        FFrameData NewFrame;
        NewFrame.Start = Start;
        NewFrame.Size  = Size;

        // 뷰어에서 새로 끌어 만든 칸도 "+ Frame"과 똑같이 선택한 프레임 바로 뒤에 끼워 넣는다.
        int InsertAt = (int)Seq.Frames.size();

        if (Seq.SelectedFrame >= 0 && Seq.SelectedFrame < (int)Seq.Frames.size())
            InsertAt = Seq.SelectedFrame + 1;

        Seq.Frames.insert(Seq.Frames.begin() + InsertAt, NewFrame);
        Seq.SelectedFrame = InsertAt;

        ApplyFrames(CompIdx, SeqIdx);
        Rebuild();
        // SpriteViewer에 다시 동기화
        if (auto Viewer2 = mSpriteViewer.lock())
        {
            std::vector<CSpriteViewerUI::FFrameRect> Rects;
            for (auto& FrameData : Seq.Frames) Rects.push_back({ FrameData.Start, FrameData.Size });
            Viewer2->SetFrames(Rects, Seq.SelectedFrame);
        }
    });

    // 피벗 기준선 — 값의 주인은 시퀀스다. 뷰어에서 끌면 여기로 돌려받는다.
    Viewer->SetPivot(Seq.PivotX, Seq.PivotY);

    Viewer->SetOnPivotChanged([this, CompIdx, SeqIdx](float PivotX, float PivotY)
    {
        if (CompIdx >= (int)mComps.size()) return;
        if (SeqIdx >= (int)mComps[CompIdx].Seqs.size()) return;

        auto& Target = mComps[CompIdx].Seqs[SeqIdx];
        Target.PivotX = PivotX;
        Target.PivotY = PivotY;
        CAnimRegistry::SetPivot(Target.Name, PivotX, PivotY);

        Rebuild();   // 에디터의 Pivot.X / Pivot.Y 표시 갱신
    });

    Viewer->SetEnable(true);
    LOG_DEBUG("[AnimEditor] Sprite Viewer opened for: %s", Seq.Name.c_str());
}
