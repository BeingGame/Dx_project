#include "InspectorUI.h"

#include "World/Button.h"
#include "World/TextBlock.h"
#include "World/TitleBar.h"
#include "World/Actor.h"
#include "World/SceneComponent.h"
#include "World/ActorComponent.h"
#include "World/MeshComponent.h"
#include "World/Animation2DComponent.h"
#include "World/ColliderBox2D.h"
#include "World/ColliderSphere2D.h"
#include "Asset/Material.h"
#include "Asset/AssetManager.h"
#include "Asset/MeshManager.h"
#include "Asset/MaterialManager.h"
#include "RenderState.h"
#include "World/Input.h"
#include "World/World.h"
#include "Component/ActionStateComponent.h"
#include "Component/HeightComponent.h"
#include "Component/DirectionInputComponent.h"
#include "Component/StatusComponent.h"
#include "World/MovementComponent.h"
#include "AnimEditorUI.h"   // CAnimRegistry (등록된 애니메이션 목록)
#include "DialogUtil.h"

#include <cstdio>   // sprintf_s (값 직접 입력 버퍼 초기화)

namespace
{
	// ── 속성 행의 칸 나누기 ──
	// 패널은 모서리를 잡고 늘릴 수 있으므로 위치를 박아두지 않고 폭에서 계산한다.
	// 오른쪽 칸(버튼·값 표시)은 폭이 고정이고, 남는 건 전부 이름 칸이 가져간다.
	// 이름 칸이 좁으면 "ComboReset"이 두 줄로 접혀서 아래쪽이 잘려 보인다.
	constexpr float ROW_LEFT  = 14.f;   // 이름 칸이 시작하는 x
	constexpr float ROW_RIGHT = 10.f;   // 오른쪽 여백
	constexpr float ROW_GAP   = 6.f;    // 칸 사이 간격

	constexpr float PROP_BTN_W = 18.f;  // [-] [+]
	constexpr float PROP_VAL_W = 66.f;  // 값 표시 (더블클릭하면 직접 입력)
	constexpr float ACT_DISP_W = 82.f;  // 값 표시 (액션 / 드롭다운)
	constexpr float ACT_BTN_W  = 44.f;  // Toggle / ▼ 버튼

	// 오른쪽 칸들이 쓰고 남은 폭을 이름 칸에 준다.
	// 패널을 최소 폭까지 좁히면 음수가 나오므로 하한을 둔다.
	float NameColumnW(float PanelW, float RestW)
	{
		const float Width = PanelW - ROW_RIGHT - RestW - ROW_GAP - ROW_LEFT;
		return (Width < 36.f) ? 36.f : Width;
	}
}

CInspectorUI::CInspectorUI()
{}

CInspectorUI::CInspectorUI(const CInspectorUI& src)
	: CWidgetContainer(src)
{}

CInspectorUI::~CInspectorUI()
{}

bool CInspectorUI::Init()
{
	CWidgetContainer::Init();

	SetPos(PANEL_X, PANEL_Y);
	SetSize(PANEL_W, PANEL_H);

	// 배경
	auto Background = CreateWidget<CButton>("InspectorBg", 0).lock();
	if (Background)
	{
		Background->SetPos(0.f, 0.f);
		Background->SetSize(PANEL_W, PANEL_H);
		Background->SetTint(EWidgetState::Normal,  0.13f, 0.13f, 0.15f, 0.92f);
		Background->SetTint(EWidgetState::Hovered, 0.13f, 0.13f, 0.15f, 0.92f);
		Background->SetTint(EWidgetState::Clicked, 0.13f, 0.13f, 0.15f, 0.92f);
		Background->SetTint(EWidgetState::Release, 0.13f, 0.13f, 0.15f, 0.92f);
		Background->SetTint(EWidgetState::Disable, 0.13f, 0.13f, 0.15f, 0.92f);
		mBackground = Background;
	}

	// 드래그 가능한 타이틀바
	auto TitleBar = CreateWidget<CTitleBar>("InspectorTitleBar", 1).lock();
	if (TitleBar)
	{
		TitleBar->SetPos(0.f, 0.f);
		TitleBar->SetSize(PANEL_W, TITLE_H);
		TitleBar->SetTint(0.22f, 0.22f, 0.28f, 1.f);
		TitleBar->SetUpdateWidget(GetThisPtr<CWidget>());
		mTitleBarWidget = TitleBar;
	}

	auto TitleText = CreateWidget<CTextBlock>("InspectorTitle", 2).lock();
	if (TitleText)
	{
		TitleText->SetPos(0.f, 0.f);
		TitleText->SetSize(PANEL_W, TITLE_H);
		TitleText->SetText(TEXT("Inspector"));
		TitleText->SetFontSize(15.f);
		TitleText->SetTextColor(FVector4::White);
		TitleText->SetAlignH(ETextAlignH::Center);
		TitleText->SetAlignV(ETextAlignV::Middle);
		mTitleText = TitleText;
	}

	mStaticChildCount = (int)mChildList.size();

	float Y = TITLE_H + 6.f;

	mActorNameText = AddRow(Y, TEXT("Name: -"));       Y += ROW_H + 2.f;
	mActorTagText  = AddRow(Y, TEXT("Tag:  -"));        Y += ROW_H + 6.f;

	mTransformHeader = AddRow(Y, TEXT("[ Transform ]"), 12.f); Y += ROW_H + 2.f;
	if (auto Text = mTransformHeader.lock())
		Text->SetTextColor(FVector4(0.6f, 0.85f, 1.f, 1.f));

	mRootNameText = AddRow(Y, TEXT("Root: -"));   Y += ROW_H + 2.f;
	mPosText      = AddRow(Y, TEXT("Pos: - "));   Y += ROW_H + 2.f;
	mRotText      = AddRow(Y, TEXT("Rot: - "));   Y += ROW_H + 2.f;
	mScaleText    = AddRow(Y, TEXT("Scale: - ")); Y += ROW_H + 6.f;

	mCompHeader = AddRow(Y, TEXT("[ Components ]"), 12.f); Y += ROW_H + 6.f;
	if (auto Text = mCompHeader.lock())
		Text->SetTextColor(FVector4(0.6f, 0.85f, 1.f, 1.f));

	mComponentsStartY = Y;

	// 코너 리사이즈 핸들 (ZOrder 10)
	auto MakeHandle = [&](const std::string& Name, float X, float PosY) -> std::weak_ptr<CButton>
	{
		auto Handle = CreateWidget<CButton>(Name, 10).lock();
		if (Handle)
		{
			Handle->SetPos(X, PosY);
			Handle->SetSize(HANDLE_SZ, HANDLE_SZ);
			Handle->SetTint(EWidgetState::Normal,  0.45f, 0.45f, 0.55f, 0.85f);
			Handle->SetTint(EWidgetState::Hovered, 0.70f, 0.90f, 1.00f, 1.f);
			Handle->SetTint(EWidgetState::Clicked, 1.00f, 1.00f, 1.00f, 1.f);
			Handle->SetTint(EWidgetState::Release, 1.00f, 1.00f, 1.00f, 1.f);
			Handle->SetTint(EWidgetState::Disable, 0.30f, 0.30f, 0.30f, 0.5f);
		}
		return Handle;
	};

	mHandleTL = MakeHandle("InspHandleTL", 0.f,              0.f);
	mHandleTR = MakeHandle("InspHandleTR", PANEL_W - HANDLE_SZ, 0.f);
	mHandleBL = MakeHandle("InspHandleBL", 0.f,              PANEL_H - HANDLE_SZ);
	mHandleBR = MakeHandle("InspHandleBR", PANEL_W - HANDLE_SZ, PANEL_H - HANDLE_SZ);

	// 컴포넌트 목록이 패널보다 길어지면 휠로 스크롤할 수 있게 한다.
	// (상단 고정 행 아래부터 패널 끝까지가 스크롤 영역)
	EnableScroll(true);
	SetScrollArea(mComponentsStartY - 2.f, PANEL_H);
	SetScrollStep(ROW_H + 2.f);

	return true;
}

// ── AddRow: Init에서만 호출되는 정적 미리할당 행 ────────────────────────────
std::weak_ptr<CTextBlock> CInspectorUI::AddRow(float Y, const wchar_t* Text, float FontSize)
{
	std::string Name = "InspRow_" + std::to_string(mDynIdx++);
	float CurW = GetSize().x;

	auto Row = CreateWidget<CTextBlock>(Name, 3).lock();
	if (Row)
	{
		Row->SetPos(6.f, Y);
		Row->SetSize(CurW - 8.f, ROW_H);
		Row->SetText(Text);
		Row->SetFontSize(FontSize);
		Row->SetTextColor(FVector4(0.85f, 0.85f, 0.85f, 1.f));
		Row->SetAlignH(ETextAlignH::Left);
		Row->SetAlignV(ETextAlignV::Middle);
	}
	return Row;
}

// ── AddDeleteButton: 컴포넌트 헤더 우측 [X] 버튼 ────────────────────────────
std::weak_ptr<CButton> CInspectorUI::AddDeleteButton(float Y, float PanelW, int WidgetIdx)
{
	float X = PanelW - DEL_BTN_W - 2.f;

	auto DelButton = CreateWidget<CButton>("ICX_" + std::to_string(WidgetIdx), 5).lock();
	if (DelButton)
	{
		DelButton->SetPos(X, Y);
		DelButton->SetSize(DEL_BTN_W, HEADER_H);
		DelButton->SetTint(EWidgetState::Normal,  0.42f, 0.16f, 0.18f, 1.f);
		DelButton->SetTint(EWidgetState::Hovered, 0.72f, 0.22f, 0.24f, 1.f);
		DelButton->SetTint(EWidgetState::Clicked, 0.92f, 0.32f, 0.34f, 1.f);
		DelButton->SetTint(EWidgetState::Release, 0.72f, 0.22f, 0.24f, 1.f);
		DelButton->SetTint(EWidgetState::Disable, 0.24f, 0.16f, 0.16f, 1.f);
		mDynamicRows.push_back(DelButton);
	}

	auto DelLabel = CreateWidget<CTextBlock>("ICXL_" + std::to_string(WidgetIdx), 6).lock();
	if (DelLabel)
	{
		DelLabel->SetPos(X, Y);
		DelLabel->SetSize(DEL_BTN_W, HEADER_H);
		DelLabel->SetText(TEXT("X"));
		DelLabel->SetFontSize(12.f);
		DelLabel->SetTextColor(FVector4::White);
		DelLabel->SetAlignH(ETextAlignH::Center);
		DelLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(DelLabel);
	}

	return DelButton;
}

// ── AddSectionHeader: 얇은 구분선 레이블 ─────────────────────────────────────
void CInspectorUI::AddSectionHeader(float& Y, const wchar_t* Text)
{
	int WidgetIdx = mDynIdx++;
	auto Label = CreateWidget<CTextBlock>("ISH_" + std::to_string(WidgetIdx), 3).lock();
	if (Label)
	{
		Label->SetPos(6.f, Y);
		Label->SetSize(GetSize().x - 8.f, 16.f);
		Label->SetText(Text);
		Label->SetFontSize(11.f);
		Label->SetTextColor(FVector4(0.55f, 0.75f, 1.f, 1.f));
		Label->SetAlignH(ETextAlignH::Left);
		Label->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(Label);
	}
	Y += 18.f;
}

// ── AddDropdownRow: 레이블 + 값 표시 + ▼ 토글 → 아래에 항목 목록 ──────────
void CInspectorUI::AddDropdownRow(float& Y,
                                  const wchar_t* Label,
                                  void* DropKey,
                                  std::vector<std::string> Items,
                                  std::function<std::string()>            Getter,
                                  std::function<void(const std::string&)> Setter,
                                  FInspCompEntry& Entry)
{
	int WidgetIdx = mDynIdx++;
	float PanelWidth = GetSize().x;
	bool bOpen = mDropdownOpen.count(DropKey) && mDropdownOpen[DropKey];

	// 칸 나누기: [이름] [값] [▼]
	const float NameW   = NameColumnW(PanelWidth, ACT_DISP_W + ROW_GAP + ACT_BTN_W);
	const float ButtonX = PanelWidth - ROW_RIGHT - ACT_BTN_W;
	const float ValueX  = ButtonX - ROW_GAP - ACT_DISP_W;

	// ── 헤더 행 ──────────────────────────────────────────────────────────────
	auto NameLabel = CreateWidget<CTextBlock>("IDL_" + std::to_string(WidgetIdx), 3).lock();
	if (NameLabel)
	{
		NameLabel->SetPos(ROW_LEFT, Y);
		NameLabel->SetSize(NameW, ROW_H - 2.f);
		NameLabel->SetText(Label);
		NameLabel->SetFontSize(11.f);
		NameLabel->SetTextColor(FVector4(0.72f, 0.72f, 0.72f, 1.f));
		NameLabel->SetAlignH(ETextAlignH::Left);
		NameLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(NameLabel);
	}

	std::string CurVal = Getter ? Getter() : "";
	std::wstring WCur(CurVal.begin(), CurVal.end());

	auto ValLabel = CreateWidget<CTextBlock>("IDV_" + std::to_string(WidgetIdx), 3).lock();
	if (ValLabel)
	{
		ValLabel->SetPos(ValueX, Y);
		ValLabel->SetSize(ACT_DISP_W, ROW_H - 2.f);
		ValLabel->SetText(WCur.c_str());
		ValLabel->SetFontSize(10.f);
		ValLabel->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		ValLabel->SetAlignH(ETextAlignH::Left);
		ValLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(ValLabel);
	}

	// 토글 버튼: ▼ / ▲
	auto TogButton = CreateWidget<CButton>("IDB_" + std::to_string(WidgetIdx), 3).lock();
	if (TogButton)
	{
		TogButton->SetPos(ButtonX, Y);
		TogButton->SetSize(ACT_BTN_W, ROW_H - 2.f);
		TogButton->SetTint(EWidgetState::Normal,  bOpen ? 0.30f : 0.20f, bOpen ? 0.44f : 0.30f, 0.46f, 1.f);
		TogButton->SetTint(EWidgetState::Hovered, 0.34f, 0.50f, 0.62f, 1.f);
		TogButton->SetTint(EWidgetState::Clicked, 0.44f, 0.62f, 0.80f, 1.f);
		TogButton->SetTint(EWidgetState::Release, 0.34f, 0.50f, 0.62f, 1.f);
		TogButton->SetTint(EWidgetState::Disable, 0.14f, 0.18f, 0.26f, 1.f);
		mDynamicRows.push_back(TogButton);
	}
	auto TogLabel = CreateWidget<CTextBlock>("IDBL_" + std::to_string(WidgetIdx), 4).lock();
	if (TogLabel)
	{
		TogLabel->SetPos(ButtonX, Y);
		TogLabel->SetSize(ACT_BTN_W, ROW_H - 2.f);
		TogLabel->SetText(bOpen ? TEXT("▲") : TEXT("▼"));
		TogLabel->SetFontSize(10.f);
		TogLabel->SetTextColor(FVector4::White);
		TogLabel->SetAlignH(ETextAlignH::Center);
		TogLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(TogLabel);
	}

	Y += ROW_H + 2.f;

	FInspDropdown Dropdown;
	Dropdown.DropKey   = DropKey;
	Dropdown.ToggleButton = TogButton;
	Dropdown.ValueLabel  = ValLabel;
	Dropdown.Items     = std::move(Items);
	Dropdown.Getter    = Getter;
	Dropdown.Setter    = std::move(Setter);

	// ── 항목 행 (열려 있을 때만 표시) ───────────────────────────────────────
	if (bOpen)
	{
		// 목록 배경 스트립
		auto ListBg = CreateWidget<CButton>("IDBG_" + std::to_string(WidgetIdx), 2).lock();
		if (ListBg)
		{
			ListBg->SetPos(14.f, Y);
			ListBg->SetSize(PanelWidth - 20.f, (float)Dropdown.Items.size() * (ROW_H + 1.f));
			ListBg->SetTint(EWidgetState::Normal,  0.10f, 0.10f, 0.12f, 0.95f);
			ListBg->SetTint(EWidgetState::Hovered, 0.10f, 0.10f, 0.12f, 0.95f);
			ListBg->SetTint(EWidgetState::Clicked, 0.10f, 0.10f, 0.12f, 0.95f);
			ListBg->SetTint(EWidgetState::Release, 0.10f, 0.10f, 0.12f, 0.95f);
			ListBg->SetTint(EWidgetState::Disable, 0.10f, 0.10f, 0.12f, 0.95f);
			mDynamicRows.push_back(ListBg);
		}

		for (auto& ItemName : Dropdown.Items)
		{
			int ItemIdx = mDynIdx++;
			bool bSelected = (ItemName == CurVal);

			auto ItemButton = CreateWidget<CButton>("IDIB_" + std::to_string(ItemIdx), 5).lock();
			if (ItemButton)
			{
				ItemButton->SetPos(14.f, Y);
				ItemButton->SetSize(PanelWidth - 20.f, ROW_H);
				ItemButton->SetTint(EWidgetState::Normal,  bSelected ? 0.18f : 0.10f, bSelected ? 0.36f : 0.14f, bSelected ? 0.28f : 0.18f, 1.f);
				ItemButton->SetTint(EWidgetState::Hovered, 0.22f, 0.38f, 0.46f, 1.f);
				ItemButton->SetTint(EWidgetState::Clicked, 0.30f, 0.50f, 0.60f, 1.f);
				ItemButton->SetTint(EWidgetState::Release, 0.22f, 0.38f, 0.46f, 1.f);
				ItemButton->SetTint(EWidgetState::Disable, 0.10f, 0.12f, 0.14f, 1.f);
				mDynamicRows.push_back(ItemButton);
			}

			auto ItemLabel = CreateWidget<CTextBlock>("IDIL_" + std::to_string(ItemIdx), 6).lock();
			if (ItemLabel)
			{
				ItemLabel->SetPos(20.f, Y);
				ItemLabel->SetSize(PanelWidth - 26.f, ROW_H);
				std::wstring WItem(ItemName.begin(), ItemName.end());
				ItemLabel->SetText(WItem.c_str());
				ItemLabel->SetFontSize(11.f);
				ItemLabel->SetTextColor(bSelected ? FVector4(0.6f, 1.f, 0.7f, 1.f) : FVector4(0.85f, 0.85f, 0.85f, 1.f));
				ItemLabel->SetAlignH(ETextAlignH::Left);
				ItemLabel->SetAlignV(ETextAlignV::Middle);
				mDynamicRows.push_back(ItemLabel);
			}

			Dropdown.ItemButtons.push_back(ItemButton);
			Y += ROW_H + 1.f;
		}
		Y += 2.f;
	}

	Entry.Dropdowns.push_back(std::move(Dropdown));
}

// ── AddActionRow: 레이블 + 표시 텍스트 + 액션 버튼 ─────────────────────────
void CInspectorUI::AddActionRow(float& Y,
                                const wchar_t* Label,
                                const wchar_t* ButtonText,
                                std::function<void()>        OnClick,
                                std::function<std::string()> GetDisplay,
                                FInspCompEntry& Entry)
{
	int WidgetIdx = mDynIdx++;

	// 칸 나누기: [이름] [값] [버튼]
	const float PanelWidth = GetSize().x;
	const float NameW   = NameColumnW(PanelWidth, ACT_DISP_W + ROW_GAP + ACT_BTN_W);
	const float ButtonX = PanelWidth - ROW_RIGHT - ACT_BTN_W;
	const float ValueX  = ButtonX - ROW_GAP - ACT_DISP_W;

	auto NameLabel = CreateWidget<CTextBlock>("IAL_" + std::to_string(WidgetIdx), 3).lock();
	if (NameLabel)
	{
		NameLabel->SetPos(ROW_LEFT, Y);
		NameLabel->SetSize(NameW, ROW_H - 2.f);
		NameLabel->SetText(Label);
		NameLabel->SetFontSize(11.f);
		NameLabel->SetTextColor(FVector4(0.72f, 0.72f, 0.72f, 1.f));
		NameLabel->SetAlignH(ETextAlignH::Left);
		NameLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(NameLabel);
	}

	// 현재 값 표시
	std::string DispStr = GetDisplay ? GetDisplay() : "";
	std::wstring WDisp(DispStr.begin(), DispStr.end());

	auto DispLabel = CreateWidget<CTextBlock>("IAD_" + std::to_string(WidgetIdx), 3).lock();
	if (DispLabel)
	{
		DispLabel->SetPos(ValueX, Y);
		DispLabel->SetSize(ACT_DISP_W, ROW_H - 2.f);
		DispLabel->SetText(WDisp.c_str());
		DispLabel->SetFontSize(10.f);
		DispLabel->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		DispLabel->SetAlignH(ETextAlignH::Left);
		DispLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(DispLabel);
	}

	auto ActButton = CreateWidget<CButton>("IAB_" + std::to_string(WidgetIdx), 3).lock();
	if (ActButton)
	{
		ActButton->SetPos(ButtonX, Y);
		ActButton->SetSize(ACT_BTN_W, ROW_H - 2.f);
		ActButton->SetTint(EWidgetState::Normal,  0.20f, 0.30f, 0.46f, 1.f);
		ActButton->SetTint(EWidgetState::Hovered, 0.30f, 0.44f, 0.66f, 1.f);
		ActButton->SetTint(EWidgetState::Clicked, 0.40f, 0.58f, 0.82f, 1.f);
		ActButton->SetTint(EWidgetState::Release, 0.30f, 0.44f, 0.66f, 1.f);
		ActButton->SetTint(EWidgetState::Disable, 0.14f, 0.18f, 0.26f, 1.f);
		mDynamicRows.push_back(ActButton);
	}
	auto ActLabel = CreateWidget<CTextBlock>("IABL_" + std::to_string(WidgetIdx), 4).lock();
	if (ActLabel)
	{
		ActLabel->SetPos(ButtonX, Y);
		ActLabel->SetSize(ACT_BTN_W, ROW_H - 2.f);
		ActLabel->SetText(ButtonText);
		ActLabel->SetFontSize(11.f);
		ActLabel->SetTextColor(FVector4::White);
		ActLabel->SetAlignH(ETextAlignH::Center);
		ActLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(ActLabel);
	}

	FInspAction Action;
	Action.ActionButton  = ActButton;
	Action.DisplayLabel = DispLabel;
	Action.OnClick    = std::move(OnClick);
	Action.GetDisplay = std::move(GetDisplay);
	Entry.Actions.push_back(std::move(Action));

	Y += ROW_H + 2.f;
}

// ── AddPropRow: [-] 값 [+] 형태의 속성 행 하나 ─────────────────────────────
void CInspectorUI::AddPropRow(float& Y,
                              const wchar_t* Label,
                              float Step,
                              std::function<float()>     Getter,
                              std::function<void(float)> Setter,
                              FInspCompEntry& Entry)
{
	int WidgetIdx = mDynIdx++;
	float CurW = GetSize().x;

	// 칸 나누기: [이름] [-] [값] [+]
	const float NameW  = NameColumnW(CurW, PROP_BTN_W * 2.f + PROP_VAL_W);
	const float MinusX = CurW - ROW_RIGHT - (PROP_BTN_W * 2.f + PROP_VAL_W);
	const float ValueX = MinusX + PROP_BTN_W;
	const float PlusX  = ValueX + PROP_VAL_W;

	FInspProp Prop;
	Prop.Step   = Step;
	Prop.Getter = std::move(Getter);
	Prop.Setter = std::move(Setter);

	// 속성 이름 레이블
	auto NameLabel = CreateWidget<CTextBlock>("IPL_" + std::to_string(WidgetIdx), 3).lock();
	if (NameLabel)
	{
		NameLabel->SetPos(ROW_LEFT, Y);
		NameLabel->SetSize(NameW, ROW_H - 2.f);
		NameLabel->SetText(Label);
		NameLabel->SetFontSize(11.f);
		NameLabel->SetTextColor(FVector4(0.72f, 0.72f, 0.72f, 1.f));
		NameLabel->SetAlignH(ETextAlignH::Left);
		NameLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(NameLabel);
	}

	// [-] 버튼
	auto MinButton = CreateWidget<CButton>("IPM_" + std::to_string(WidgetIdx), 3).lock();
	if (MinButton)
	{
		MinButton->SetPos(MinusX, Y);
		MinButton->SetSize(PROP_BTN_W, ROW_H - 2.f);
		MinButton->SetTint(EWidgetState::Normal,  0.26f, 0.26f, 0.32f, 1.f);
		MinButton->SetTint(EWidgetState::Hovered, 0.44f, 0.44f, 0.54f, 1.f);
		MinButton->SetTint(EWidgetState::Clicked, 0.60f, 0.60f, 0.72f, 1.f);
		MinButton->SetTint(EWidgetState::Release, 0.44f, 0.44f, 0.54f, 1.f);
		MinButton->SetTint(EWidgetState::Disable, 0.20f, 0.20f, 0.24f, 1.f);
		mDynamicRows.push_back(MinButton);
		Prop.MinusButton = MinButton;
	}
	auto MinLabel = CreateWidget<CTextBlock>("IPML_" + std::to_string(WidgetIdx), 4).lock();
	if (MinLabel)
	{
		MinLabel->SetPos(MinusX, Y);
		MinLabel->SetSize(PROP_BTN_W, ROW_H - 2.f);
		MinLabel->SetText(TEXT("-"));
		MinLabel->SetFontSize(12.f);
		MinLabel->SetTextColor(FVector4::White);
		MinLabel->SetAlignH(ETextAlignH::Center);
		MinLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(MinLabel);
	}

	// 값 표시 배경 — 더블클릭하면 직접 입력 모드로 들어간다.
	// (CTextBlock은 마우스를 먹지 않으므로 아래 ZOrder 2에 깔아도 클릭을 받는다)
	auto ValButton = CreateWidget<CButton>("IPVB_" + std::to_string(WidgetIdx), 2).lock();
	if (ValButton)
	{
		ValButton->SetPos(ValueX, Y);
		ValButton->SetSize(PROP_VAL_W, ROW_H - 2.f);
		ValButton->SetTint(EWidgetState::Normal,  0.17f, 0.17f, 0.21f, 1.f);
		ValButton->SetTint(EWidgetState::Hovered, 0.24f, 0.26f, 0.34f, 1.f);
		ValButton->SetTint(EWidgetState::Clicked, 0.30f, 0.34f, 0.44f, 1.f);
		ValButton->SetTint(EWidgetState::Release, 0.24f, 0.26f, 0.34f, 1.f);
		ValButton->SetTint(EWidgetState::Disable, 0.14f, 0.14f, 0.18f, 1.f);
		mDynamicRows.push_back(ValButton);
		Prop.ValueButton = ValButton;
	}

	// 값 표시
	float InitVal = Prop.Getter ? Prop.Getter() : 0.f;
	TCHAR ValueBuffer[32] = {};
	swprintf_s(ValueBuffer, 32, L"%.2f", InitVal);

	auto ValLabel = CreateWidget<CTextBlock>("IPV_" + std::to_string(WidgetIdx), 3).lock();
	if (ValLabel)
	{
		ValLabel->SetPos(ValueX, Y);
		ValLabel->SetSize(PROP_VAL_W, ROW_H - 2.f);
		ValLabel->SetText(ValueBuffer);
		ValLabel->SetFontSize(11.f);
		ValLabel->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		ValLabel->SetAlignH(ETextAlignH::Center);
		ValLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(ValLabel);
		Prop.ValueLabel = ValLabel;
	}

	// [+] 버튼
	auto PlusButton = CreateWidget<CButton>("IPP_" + std::to_string(WidgetIdx), 3).lock();
	if (PlusButton)
	{
		PlusButton->SetPos(PlusX, Y);
		PlusButton->SetSize(PROP_BTN_W, ROW_H - 2.f);
		PlusButton->SetTint(EWidgetState::Normal,  0.26f, 0.26f, 0.32f, 1.f);
		PlusButton->SetTint(EWidgetState::Hovered, 0.44f, 0.44f, 0.54f, 1.f);
		PlusButton->SetTint(EWidgetState::Clicked, 0.60f, 0.60f, 0.72f, 1.f);
		PlusButton->SetTint(EWidgetState::Release, 0.44f, 0.44f, 0.54f, 1.f);
		PlusButton->SetTint(EWidgetState::Disable, 0.20f, 0.20f, 0.24f, 1.f);
		mDynamicRows.push_back(PlusButton);
		Prop.PlusButton = PlusButton;
	}
	auto PlusLabel = CreateWidget<CTextBlock>("IPPL_" + std::to_string(WidgetIdx), 4).lock();
	if (PlusLabel)
	{
		PlusLabel->SetPos(PlusX, Y);
		PlusLabel->SetSize(PROP_BTN_W, ROW_H - 2.f);
		PlusLabel->SetText(TEXT("+"));
		PlusLabel->SetFontSize(12.f);
		PlusLabel->SetTextColor(FVector4::White);
		PlusLabel->SetAlignH(ETextAlignH::Center);
		PlusLabel->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(PlusLabel);
	}

	Entry.Props.push_back(std::move(Prop));
	Y += ROW_H + 2.f;
}

// ── 타겟 설정 / 위젯 재구성 ──────────────────────────────────────────────────
void CInspectorUI::SetTarget(std::weak_ptr<CActor> Actor)
{
	mTarget = Actor;
	Rebuild();
}

void CInspectorUI::Rebuild()
{
	// 편집 중이던 행의 위젯/인덱스가 모두 무효가 되므로 먼저 편집을 끝낸다.
	CancelEdit();

	// 이전에 추가된 모든 동적 위젯 제거 (정렬 순서 무관하게 안전)
	for (auto& Row : mDynamicRows)
		RemoveChild(Row);
	mDynamicRows.clear();
	mCompEntries.clear();

	auto Actor = mTarget.lock();

	// 정적 행 업데이트
	if (!Actor)
	{
		if (auto Text = mActorNameText.lock()) Text->SetText(TEXT("Name: -"));
		if (auto Text = mActorTagText.lock())  Text->SetText(TEXT("Tag:  -"));
		if (auto Text = mRootNameText.lock())  Text->SetText(TEXT("Root: -"));
		if (auto Text = mPosText.lock())       Text->SetText(TEXT("Pos: -"));
		if (auto Text = mRotText.lock())       Text->SetText(TEXT("Rot: -"));
		if (auto Text = mScaleText.lock())     Text->SetText(TEXT("Scale: -"));
		FinishLayout(mComponentsStartY);
		return;
	}

	{
		std::wstring WName(Actor->GetName().begin(), Actor->GetName().end());
		std::wstring WTag (Actor->GetActorTag().begin(), Actor->GetActorTag().end());
		TCHAR TextBuffer[128] = {};
		if (auto Text = mActorNameText.lock()) { wsprintf(TextBuffer, TEXT("Name: %s"), WName.c_str()); Text->SetText(TextBuffer); }
		if (auto Text = mActorTagText.lock())  { wsprintf(TextBuffer, TEXT("Tag:  %s"), WTag.c_str());  Text->SetText(TextBuffer); }
	}

	if (auto Root = Actor->GetRootComponent().lock())
	{
		std::wstring WRoot(Root->GetName().begin(), Root->GetName().end());
		TCHAR TextBuffer[128] = {};
		if (auto Text = mRootNameText.lock()) { wsprintf(TextBuffer, TEXT("Root: %s"), WRoot.c_str()); Text->SetText(TextBuffer); }
	}
	else
	{
		if (auto Text = mRootNameText.lock()) Text->SetText(TEXT("Root: (none)"));
		if (auto Text = mPosText.lock())      Text->SetText(TEXT("Pos: -"));
		if (auto Text = mRotText.lock())      Text->SetText(TEXT("Rot: -"));
		if (auto Text = mScaleText.lock())    Text->SetText(TEXT("Scale: -"));
	}

	// ── 씬 컴포넌트 ───────────────────────────────────────────────────────────
	float Y = mComponentsStartY;
	float PanelW = GetSize().x;

	for (const auto& Comp : Actor->GetSceneCompList())
	{
		if (!Comp) continue;

		bool bExpanded = false;
		auto Found = mExpandState.find(Comp.get());
		if (Found != mExpandState.end()) bExpanded = Found->second;

		int WidgetIdx = mDynIdx++;

		// 헤더 버튼 (클릭 가능한 배경) — 우측 [X] 버튼 자리를 비워둔다.
		auto HButton = CreateWidget<CButton>("ICH_" + std::to_string(WidgetIdx), 3).lock();
		if (HButton)
		{
			HButton->SetPos(2.f, Y);
			HButton->SetSize(PanelW - 6.f - DEL_BTN_W, HEADER_H);
			HButton->SetTint(EWidgetState::Normal,  0.17f, 0.21f, 0.30f, 1.f);
			HButton->SetTint(EWidgetState::Hovered, 0.24f, 0.34f, 0.50f, 1.f);
			HButton->SetTint(EWidgetState::Clicked, 0.30f, 0.42f, 0.62f, 1.f);
			HButton->SetTint(EWidgetState::Release, 0.24f, 0.34f, 0.50f, 1.f);
			HButton->SetTint(EWidgetState::Disable, 0.14f, 0.14f, 0.18f, 1.f);
			mDynamicRows.push_back(HButton);
		}

		// 헤더 레이블
		std::string CompName = Comp->GetName();
		std::string CompType = Comp->GetTypeName();
		std::wstring WName(CompName.begin(), CompName.end());
		std::wstring WType(CompType.begin(), CompType.end());
		//이름과 타입명을 줄을 나눠 적는다. 한 줄로 붙이면 패널 폭을 넘겨서 뒤가 잘린다.
		TCHAR HBuf[128] = {};
		wsprintf(HBuf, bExpanded ? TEXT("v %s\n  (%s)") : TEXT("> %s\n  (%s)"),
			WName.c_str(), WType.c_str());

		auto HLabel = CreateWidget<CTextBlock>("ICHL_" + std::to_string(WidgetIdx), 4).lock();
		if (HLabel)
		{
			HLabel->SetPos(8.f, Y);
			HLabel->SetSize(PanelW - 14.f - DEL_BTN_W, HEADER_H);
			HLabel->SetText(HBuf);
			HLabel->SetFontSize(12.f);
			HLabel->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
			HLabel->SetAlignH(ETextAlignH::Left);
			HLabel->SetAlignV(ETextAlignV::Middle);
			mDynamicRows.push_back(HLabel);
		}

		auto DelButton = AddDeleteButton(Y, PanelW, WidgetIdx);

		Y += HEADER_H + 2.f;

		FInspCompEntry Entry;
		Entry.CompKey   = Comp.get();
		Entry.CompRef   = std::static_pointer_cast<CComponent>(Comp);
		Entry.HeaderButton = HButton;
		Entry.HeaderLabel = HLabel;
		Entry.DeleteButton = DelButton;
		Entry.bExpanded = bExpanded;

		if (bExpanded)
		{
			std::weak_ptr<CSceneComponent> SceneCompWeak = Comp;

			// ── 트랜스폼 ───────────────────────────────────────────────
			AddSectionHeader(Y, TEXT("Transform"));

			AddPropRow(Y, TEXT("PosX"), 1.f,
				[SceneCompWeak]() -> float { auto SceneComp = SceneCompWeak.lock(); return SceneComp ? SceneComp->GetRelativePos().x : 0.f; },
				[SceneCompWeak](float Value)   { auto SceneComp = SceneCompWeak.lock(); if (SceneComp) { auto Pos = SceneComp->GetRelativePos(); SceneComp->SetRelativePos(Value, Pos.y, Pos.z); } },
				Entry);

			AddPropRow(Y, TEXT("PosY"), 1.f,
				[SceneCompWeak]() -> float { auto SceneComp = SceneCompWeak.lock(); return SceneComp ? SceneComp->GetRelativePos().y : 0.f; },
				[SceneCompWeak](float Value)   { auto SceneComp = SceneCompWeak.lock(); if (SceneComp) { auto Pos = SceneComp->GetRelativePos(); SceneComp->SetRelativePos(Pos.x, Value, Pos.z); } },
				Entry);

			AddPropRow(Y, TEXT("RotZ"), 1.f,
				[SceneCompWeak]() -> float { auto SceneComp = SceneCompWeak.lock(); return SceneComp ? SceneComp->GetRelativeRot().z : 0.f; },
				[SceneCompWeak](float Value)   { auto SceneComp = SceneCompWeak.lock(); if (SceneComp) { auto Rot = SceneComp->GetRelativeRot(); SceneComp->SetRelativeRotation(Rot.x, Rot.y, Value); } },
				Entry);

			AddPropRow(Y, TEXT("SclX"), 0.1f,
				[SceneCompWeak]() -> float { auto SceneComp = SceneCompWeak.lock(); return SceneComp ? SceneComp->GetRelativeScale().x : 1.f; },
				[SceneCompWeak](float Value)   { auto SceneComp = SceneCompWeak.lock(); if (SceneComp) { auto Scale = SceneComp->GetRelativeScale(); SceneComp->SetRelativeScale(Value, Scale.y, Scale.z); } },
				Entry);

			AddPropRow(Y, TEXT("SclY"), 0.1f,
				[SceneCompWeak]() -> float { auto SceneComp = SceneCompWeak.lock(); return SceneComp ? SceneComp->GetRelativeScale().y : 1.f; },
				[SceneCompWeak](float Value)   { auto SceneComp = SceneCompWeak.lock(); if (SceneComp) { auto Scale = SceneComp->GetRelativeScale(); SceneComp->SetRelativeScale(Scale.x, Value, Scale.z); } },
				Entry);

			// ── CMeshComponent 전용 속성 ──────────────────────────────
			auto MeshComp = std::dynamic_pointer_cast<CMeshComponent>(Comp);
			if (MeshComp)
			{
				std::weak_ptr<CMeshComponent> MeshCompWeak = MeshComp;

				AddSectionHeader(Y, TEXT("Mesh"));

				// Mesh 지오메트리 드롭다운
				{
					std::vector<std::string> MeshNames;
					auto MeshManager = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);
					if (MeshManager) MeshNames = MeshManager->GetMeshNames();
					if (MeshNames.empty()) MeshNames = { "TexRect", "ColorRect", "FrameRect", "Sphere" };

					void* DropdownKey = (void*)((char*)MeshComp.get() + 1); // MeshComp마다 고유 키
					AddDropdownRow(Y, TEXT("Mesh"), DropdownKey, std::move(MeshNames),
						// GetMeshName()은 "Mesh_TexRect"처럼 매니저 내부 접두사가 붙은 이름을 준다.
						// 목록 항목은 접두사가 빠진 형태라 그대로 쓰면 현재 값이 매칭되지 않는다.
						[MeshCompWeak]() -> std::string {
							auto MeshComp = MeshCompWeak.lock();
							if (!MeshComp) return "";
							std::string Name = MeshComp->GetMeshName();
							static const std::string Prefix = "Mesh_";
							if (Name.size() > Prefix.size() && Name.compare(0, Prefix.size(), Prefix) == 0)
								Name = Name.substr(Prefix.size());
							return Name;
						},
						[MeshCompWeak](const std::string& Name) { auto MeshComp = MeshCompWeak.lock(); if (MeshComp) MeshComp->SetMesh(Name); },
						Entry);
				}

				// Shader 드롭다운
				{
					static const std::vector<std::string> sShaderNames = { "Material", "Texture", "Animation2D", "Color2D" };
					void* ShaderKey = (void*)((char*)MeshComp.get() + 2);
					AddDropdownRow(Y, TEXT("Shader"), ShaderKey, sShaderNames,
						[MeshCompWeak]() -> std::string { auto MeshComp = MeshCompWeak.lock(); return MeshComp ? MeshComp->GetShaderName() : ""; },
						[MeshCompWeak](const std::string& Name) { auto MeshComp = MeshCompWeak.lock(); if (MeshComp) MeshComp->SetShader(Name); },
						Entry);
				}

				// Blend 드롭다운
				{
					static const std::vector<std::string> sBlendNames = { "(none)", "AlphaBlend" };
					void* BlendKey = (void*)((char*)MeshComp.get() + 3);
					AddDropdownRow(Y, TEXT("Blend"), BlendKey, sBlendNames,
						[MeshCompWeak]() -> std::string {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return "(none)";
							auto Material = MeshComp->GetMaterial(0);
							auto BlendState  = Material ? Material->GetBlendState().lock() : nullptr;
							return BlendState ? BlendState->GetName() : "(none)";
						},
						[MeshCompWeak](const std::string& BlendName) {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return;
							MeshComp->SetBlendState(0, BlendName == "(none)" ? "" : BlendName);
						},
						Entry);
				}

				AddSectionHeader(Y, TEXT("Material"));

				// Material 선택 드롭다운
				{
					std::vector<std::string> MatNames;
					auto MatManager = CAssetManager::GetInst()->GetSubManager<CMaterialManager>(EAssetType::Material);
					if (MatManager) MatNames = MatManager->GetAllMaterialNames();
					if (MatNames.empty()) MatNames = { "Default" };

					void* MatKey = (void*)((char*)MeshComp.get() + 4);
					AddDropdownRow(Y, TEXT("Mat"), MatKey, std::move(MatNames),
						[MeshCompWeak]() -> std::string {
							auto MeshComp = MeshCompWeak.lock(); return MeshComp ? MeshComp->GetMaterialName(0) : "";
						},
						[MeshCompWeak](const std::string& Name) {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return;
							auto MatManager2 = CAssetManager::GetInst()->GetSubManager<CMaterialManager>(EAssetType::Material);
							if (!MatManager2) return;
							auto NewMat = MatManager2->CreateMaterialInstance(Name);
							if (NewMat)
							{
								// 머티리얼만 갈아끼운다. Shader는 건드리지 않는다.
								// 예전에는 머티리얼의 셰이더로 덮어써서, Mat을 고르면
								// Shader가 Animation2D → Material로 되돌아가고
								// 애니메이션이 시트 전체로 풀려버렸다.
								MeshComp->SetMaterialSlot(0, NewMat);
							}
						},
						Entry);
				}

				// 머티리얼 속성 편집 (슬롯 0이 존재할 때만)
				if (MeshComp->GetMaterial(0))
				{
					AddSectionHeader(Y, TEXT("Mat Props"));

					// 알파는 Opac 하나만 쓴다.
					// 셰이더가 색을 이렇게 합치기 때문이다.
					//   output.Color.rgb = TextureColor.rgb * cbMatBaseColor.rgb;
					//   output.Color.a   = TextureColor.a   * cbMatOpacity;
					// BaseColor의 w는 어느 셰이더도 읽지 않아서, 예전에 있던 ColA 행은
					// 값만 저장되고 화면은 전혀 변하지 않았다. 그래서 없앴다.
					AddPropRow(Y, TEXT("Opac"), 0.05f,
						[MeshCompWeak]() -> float {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return 1.f;
							auto Material = MeshComp->GetMaterial(0); return Material ? Material->GetOpacity() : 1.f;
						},
						[MeshCompWeak](float Value) {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return;
							float Clamped = Value < 0.f ? 0.f : (Value > 1.f ? 1.f : Value);
							MeshComp->SetOpacity(0, Clamped);
						},
						Entry);

					AddPropRow(Y, TEXT("ColR"), 0.05f,
						[MeshCompWeak]() -> float {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return 1.f;
							auto Material = MeshComp->GetMaterial(0); return Material ? Material->GetBaseColor().x : 1.f;
						},
						[MeshCompWeak](float Value) {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return;
							auto Material = MeshComp->GetMaterial(0); if (!Material) return;
							float Clamped = Value < 0.f ? 0.f : (Value > 1.f ? 1.f : Value);
							const FVector4& BaseColor = Material->GetBaseColor();
							MeshComp->SetBaseColor(0, Clamped, BaseColor.y, BaseColor.z, BaseColor.w);
						},
						Entry);

					AddPropRow(Y, TEXT("ColG"), 0.05f,
						[MeshCompWeak]() -> float {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return 1.f;
							auto Material = MeshComp->GetMaterial(0); return Material ? Material->GetBaseColor().y : 1.f;
						},
						[MeshCompWeak](float Value) {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return;
							auto Material = MeshComp->GetMaterial(0); if (!Material) return;
							float Clamped = Value < 0.f ? 0.f : (Value > 1.f ? 1.f : Value);
							const FVector4& BaseColor = Material->GetBaseColor();
							MeshComp->SetBaseColor(0, BaseColor.x, Clamped, BaseColor.z, BaseColor.w);
						},
						Entry);

					AddPropRow(Y, TEXT("ColB"), 0.05f,
						[MeshCompWeak]() -> float {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return 1.f;
							auto Material = MeshComp->GetMaterial(0); return Material ? Material->GetBaseColor().z : 1.f;
						},
						[MeshCompWeak](float Value) {
							auto MeshComp = MeshCompWeak.lock(); if (!MeshComp) return;
							auto Material = MeshComp->GetMaterial(0); if (!Material) return;
							float Clamped = Value < 0.f ? 0.f : (Value > 1.f ? 1.f : Value);
							const FVector4& BaseColor = Material->GetBaseColor();
							MeshComp->SetBaseColor(0, BaseColor.x, BaseColor.y, Clamped, BaseColor.w);
						},
						Entry);

				}
			}

			// ── 콜라이더 전용 속성 ────────────────────────────────────
			AddColliderProps(Y, Comp, Entry);
		}

		mCompEntries.push_back(std::move(Entry));
	}

	// ── 액터 컴포넌트 (씬 컴포넌트와 동일하게 펼칠 수 있다) ──────────────────
	for (const auto& Comp : Actor->GetActorCompList())
	{
		if (!Comp) continue;

		bool bExpanded = false;
		auto Found = mExpandState.find(Comp.get());
		if (Found != mExpandState.end()) bExpanded = Found->second;

		int WidgetIdx = mDynIdx++;

		// 헤더 버튼 (클릭 가능한 배경) — 우측 [X] 버튼 자리를 비워둔다.
		auto HButton = CreateWidget<CButton>("ICAH_" + std::to_string(WidgetIdx), 3).lock();
		if (HButton)
		{
			HButton->SetPos(2.f, Y);
			HButton->SetSize(PanelW - 6.f - DEL_BTN_W, HEADER_H);
			HButton->SetTint(EWidgetState::Normal,  0.20f, 0.18f, 0.24f, 1.f);
			HButton->SetTint(EWidgetState::Hovered, 0.30f, 0.26f, 0.38f, 1.f);
			HButton->SetTint(EWidgetState::Clicked, 0.38f, 0.32f, 0.48f, 1.f);
			HButton->SetTint(EWidgetState::Release, 0.30f, 0.26f, 0.38f, 1.f);
			HButton->SetTint(EWidgetState::Disable, 0.14f, 0.14f, 0.18f, 1.f);
			mDynamicRows.push_back(HButton);
		}

		std::string CompName = Comp->GetName();
		std::string CompType = Comp->GetTypeName();
		std::wstring WName(CompName.begin(), CompName.end());
		std::wstring WType(CompType.begin(), CompType.end());
		//씬 컴포넌트와 같은 이유로 두 줄로 나눠 적는다.
		TCHAR ABuf[128] = {};
		wsprintf(ABuf, bExpanded ? TEXT("v [A] %s\n  (%s)") : TEXT("> [A] %s\n  (%s)"),
			WName.c_str(), WType.c_str());

		auto HLabel = CreateWidget<CTextBlock>("ICAHL_" + std::to_string(WidgetIdx), 4).lock();
		if (HLabel)
		{
			HLabel->SetPos(8.f, Y);
			HLabel->SetSize(PanelW - 14.f - DEL_BTN_W, HEADER_H);
			HLabel->SetText(ABuf);
			HLabel->SetFontSize(12.f);
			HLabel->SetTextColor(FVector4(0.86f, 0.80f, 0.94f, 1.f));
			HLabel->SetAlignH(ETextAlignH::Left);
			HLabel->SetAlignV(ETextAlignV::Middle);
			mDynamicRows.push_back(HLabel);
		}

		auto DelButton = AddDeleteButton(Y, PanelW, WidgetIdx);

		Y += HEADER_H + 2.f;

		FInspCompEntry Entry;
		Entry.CompKey   = Comp.get();
		Entry.CompRef   = std::static_pointer_cast<CComponent>(Comp);
		Entry.HeaderButton = HButton;
		Entry.HeaderLabel = HLabel;
		Entry.DeleteButton = DelButton;
		Entry.bExpanded = bExpanded;

		if (bExpanded)
			AddActorCompProps(Y, Comp, Entry);

		mCompEntries.push_back(std::move(Entry));
	}

	FinishLayout(Y);
}

// ── 콜라이더 속성 ────────────────────────────────────────────────────────────

void CInspectorUI::AddColliderProps(float& Y,
                                    const std::shared_ptr<CSceneComponent>& Comp,
                                    FInspCompEntry& Entry)
{
	auto Collider = std::dynamic_pointer_cast<CCollider>(Comp);
	if (!Collider) return;

	AddSectionHeader(Y, TEXT("Collider"));

	if (auto Box = std::dynamic_pointer_cast<CColliderBox2D>(Comp))
	{
		std::weak_ptr<CColliderBox2D> BoxWeak = Box;

		// Halfsize를 들고 있으므로 표시는 전체 크기로 환산한다.
		AddPropRow(Y, TEXT("Size.W"), 1.f,
			[BoxWeak]() -> float { auto Box = BoxWeak.lock(); return Box ? Box->GetInfo().Halfsize.x * 2.f : 0.f; },
			[BoxWeak](float Value) {
				auto Box = BoxWeak.lock(); if (!Box) return;
				Box->SetBoxSize(Value < 1.f ? 1.f : Value, Box->GetInfo().Halfsize.y * 2.f);
			},
			Entry);

		AddPropRow(Y, TEXT("Size.H"), 1.f,
			[BoxWeak]() -> float { auto Box = BoxWeak.lock(); return Box ? Box->GetInfo().Halfsize.y * 2.f : 0.f; },
			[BoxWeak](float Value) {
				auto Box = BoxWeak.lock(); if (!Box) return;
				Box->SetBoxSize(Box->GetInfo().Halfsize.x * 2.f, Value < 1.f ? 1.f : Value);
			},
			Entry);
	}
	else if (auto Sphere = std::dynamic_pointer_cast<CColliderSphere2D>(Comp))
	{
		std::weak_ptr<CColliderSphere2D> SphereWeak = Sphere;

		AddPropRow(Y, TEXT("Radius"), 1.f,
			[SphereWeak]() -> float { auto Sphere = SphereWeak.lock(); return Sphere ? Sphere->GetInfo().Radius : 0.f; },
			[SphereWeak](float Value) { auto Sphere = SphereWeak.lock(); if (Sphere) Sphere->SetRadius(Value < 1.f ? 1.f : Value); },
			Entry);
	}

	// 디버그 외곽선 표시 토글 (두 종류 공통)
	std::weak_ptr<CCollider> SceneCompWeak = Collider;

	AddActionRow(Y, TEXT("Debug"), TEXT("Toggle"),
		[SceneCompWeak]() { auto SceneComp = SceneCompWeak.lock(); if (SceneComp) SceneComp->SetDebugDraw(!SceneComp->GetDebugDraw()); },
		[SceneCompWeak]() -> std::string {
			auto SceneComp = SceneCompWeak.lock();
			return SceneComp ? (SceneComp->GetDebugDraw() ? "ON" : "OFF") : "-";
		},
		Entry);
}

// ── 액터 컴포넌트별 속성 ─────────────────────────────────────────────────────
// 지금은 Animation2DComponent만 다룬다. 다른 타입은 헤더만 펼쳐지고 내용이 비어 있다.
void CInspectorUI::AddActorCompProps(float& Y,
                                     const std::shared_ptr<CActorComponent>& Comp,
                                     FInspCompEntry& Entry)
{
	// ── 액션 상태 (걷기/달리기/공격/피격 …) ──────────────────────────────────
	if (auto ActionComp = std::dynamic_pointer_cast<CActionStateComponent>(Comp))
	{
		std::weak_ptr<CActionStateComponent> ActionStateWeak = ActionComp;

		AddSectionHeader(Y, TEXT("Action State"));

		// 지금 어떤 상태인지. 표시 전용이라 돌려보면서 확인할 수 있다.
		AddActionRow(Y, TEXT("Current"), TEXT("-"),
			nullptr,
			[ActionStateWeak]() -> std::string {
				auto ActionState = ActionStateWeak.lock();
				return ActionState ? CActionStateComponent::GetStateName(ActionState->GetState()) : "-";
			},
			Entry);

		// 상태 9개를 전부 펼치면 패널이 감당이 안 되므로 하나를 골라서 편집한다.
		{
			std::vector<std::string> StateNames;
			for (int i = 0; i < EActionState::End; ++i)
				StateNames.push_back(CActionStateComponent::GetStateName((EActionState::Type)i));

			void* DropdownKey = (void*)((char*)ActionComp.get() + 1);
			AddDropdownRow(Y, TEXT("Edit"), DropdownKey, std::move(StateNames),
				[this]() -> std::string {
					return CActionStateComponent::GetStateName((EActionState::Type)mActionEditIdx);
				},
				[this](const std::string& Name) {
					EActionState::Type Found = CActionStateComponent::FindStateByName(Name);
					if (Found != EActionState::End) mActionEditIdx = (int)Found;
				},
				Entry);
		}

		const EActionState::Type Edit = (EActionState::Type)mActionEditIdx;

		// 이 상태에 걸어줄 애니메이션.
		// 후보는 같은 액터의 Animation2DComponent에 실제로 등록된 시퀀스에서 뽑는다.
		// (컴포넌트에 없는 이름을 넣으면 재생 자체가 안 되기 때문이다)
		{
			std::vector<std::string> AnimNames = { "(none)" };

			if (auto Owner = ActionComp->GetOwner().lock())
			{
				for (const auto& OtherComp : Owner->GetActorCompList())
				{
					auto Anim = std::dynamic_pointer_cast<CAnimation2DComponent>(OtherComp);
					if (!Anim) continue;

					for (const auto& Pair : Anim->GetAnimationMap())
						AnimNames.push_back(Pair.first);

					break;
				}
			}

			// 컴포넌트에 시퀀스가 아직 없으면 에셋으로 불러온 목록이라도 보여준다.
			if (AnimNames.size() == 1)
			{
				for (const auto& Name : CAnimRegistry::GetAll())
					AnimNames.push_back(Name);
			}

			void* DropdownKey = (void*)((char*)ActionComp.get() + 2);
			AddDropdownRow(Y, TEXT("Anim"), DropdownKey, std::move(AnimNames),
				[ActionStateWeak, Edit]() -> std::string {
					auto ActionState = ActionStateWeak.lock();
					if (!ActionState) return "(none)";
					const std::string& Name = ActionState->GetStateAnim(Edit);
					return Name.empty() ? "(none)" : Name;
				},
				[ActionStateWeak, Edit](const std::string& Name) {
					auto ActionState = ActionStateWeak.lock();
					if (ActionState) ActionState->SetStateAnim(Edit, (Name == "(none)") ? std::string() : Name);
				},
				Entry);
		}

		// 상태 정의 한 벌을 읽고 쓰는 헬퍼. (정의는 값으로 주고받는다)
		auto GetDef = [ActionStateWeak, Edit]() -> FActionStateDef {
			auto ActionState = ActionStateWeak.lock();
			return ActionState ? ActionState->GetStateDef(Edit) : FActionStateDef();
		};

		auto SetDef = [ActionStateWeak, Edit](const FActionStateDef& StateDef) {
			auto ActionState = ActionStateWeak.lock();
			if (ActionState) ActionState->SetStateDef(Edit, StateDef);
		};

		// 우선순위 — 이 값이 "피격이 공격을 끊는다"를 결정한다.
		AddPropRow(Y, TEXT("Priority"), 5.f,
			[GetDef]() -> float { return (float)GetDef().Priority; },
			[GetDef, SetDef](float Value) {
				FActionStateDef StateDef = GetDef();
				StateDef.Priority = (int)Value;
				SetDef(StateDef);
			},
			Entry);

		// 콤보 선입력을 받아주기 시작하는 시각. 음수면 끝날 때까지 안 받는다.
		AddPropRow(Y, TEXT("Cancel"), 0.05f,
			[GetDef]() -> float { return GetDef().CancelTime; },
			[GetDef, SetDef](float Value) {
				FActionStateDef StateDef = GetDef();
				StateDef.CancelTime = Value;
				SetDef(StateDef);
			},
			Entry);

		// 이 상태에 들어갈 때 바라보는 쪽으로 밀고 나갈 거리. (기본 공격이 파고드는 양)
		// 위 Edit에서 Attack1/2/3을 고른 다음 이 값을 올리면 그 타수가 더 파고든다.
		// 애니메이션이 재생되는 시간에 걸쳐 나눠 밀기 때문에 값이 곧 이동 거리다.
		// 이동이 잠긴 상태(CanMove OFF)에서만 의미가 있다.
		// 켜져 있으면 입력 쪽이 매 프레임 속도를 다시 덮어써서 서로 싸운다.
		AddPropRow(Y, TEXT("MoveDist"), 0.01f,
			[GetDef]() -> float { return GetDef().MoveDist; },
			[GetDef, SetDef](float Value) {
				FActionStateDef StateDef = GetDef();
				StateDef.MoveDist = (Value < 0.f) ? 0.f : Value;
				SetDef(StateDef);
			},
			Entry);

		// 그 거리를 밀어내는 데 쓰는 시간. 상태에 들어간 직후부터 이만큼만 움직인다.
		// 짧으면 치고 나가는 느낌, 길면 동작 내내 미끄러진다.
		// 0으로 두면 애니메이션 길이 전체에 걸쳐 나눠 민다.
		AddPropRow(Y, TEXT("StepTime"), 0.02f,
			[GetDef]() -> float { return GetDef().StepTime; },
			[GetDef, SetDef](float Value) {
				FActionStateDef StateDef = GetDef();
				StateDef.StepTime = (Value < 0.f) ? 0.f : Value;
				SetDef(StateDef);
			},
			Entry);

		// 이 동작에 맞은 상대를 띄우는 힘. 0보다 크면 상대가 Airborne으로 뜬다.
		// (판정이 붙기 전이라 아래 TestHit 버튼으로 자기 자신에게 걸어볼 수 있다)
		AddPropRow(Y, TEXT("Launch"), 0.1f,
			[GetDef]() -> float { return GetDef().LaunchPower; },
			[GetDef, SetDef](float Value) {
				FActionStateDef StateDef = GetDef();
				StateDef.LaunchPower = (Value < 0.f) ? 0.f : Value;
				SetDef(StateDef);
			},
			Entry);

		// 뒤로 밀어내는 속도. 띄우지 않아도 밀 수는 있다.
		AddPropRow(Y, TEXT("Knock"), 0.05f,
			[GetDef]() -> float { return GetDef().KnockPower; },
			[GetDef, SetDef](float Value) {
				FActionStateDef StateDef = GetDef();
				StateDef.KnockPower = (Value < 0.f) ? 0.f : Value;
				SetDef(StateDef);
			},
			Entry);

		// ON/OFF 세 개
		auto AddDefFlag = [&](const wchar_t* Label, bool FActionStateDef::* Member)
		{
			AddActionRow(Y, Label, TEXT("Toggle"),
				[GetDef, SetDef, Member]() {
					FActionStateDef StateDef = GetDef();
					StateDef.*Member = !(StateDef.*Member);
					SetDef(StateDef);
				},
				[GetDef, Member]() -> std::string {
					return (GetDef().*Member) ? "ON" : "OFF";
				},
				Entry);
		};

		AddDefFlag(TEXT("Loop"),    &FActionStateDef::bLoop);
		AddDefFlag(TEXT("CanMove"), &FActionStateDef::bCanMove);
		AddDefFlag(TEXT("CanTurn"), &FActionStateDef::bCanTurn);

		// 공격키를 누르고 있는 동안 첫 프레임에서 멈춰 세울지. (모아치기)
		AddDefFlag(TEXT("Charge"),  &FActionStateDef::bChargeHold);

		// 애니메이션 역재생. 기상(GetUp)이 쓰러지는 동작을 뒤집어 쓴다.
		AddDefFlag(TEXT("Reverse"), &FActionStateDef::bReverse);

		// 액션계만 눌러서 시험 재생한다. 이동계는 입력으로 바뀌므로 의미가 없다.
		if (!CActionStateComponent::IsLocomotion(Edit))
		{
			AddActionRow(Y, TEXT("Test"), TEXT("Play"),
				[ActionStateWeak, Edit]() {
					auto ActionState = ActionStateWeak.lock();
					if (ActionState) ActionState->RequestState(Edit);
				},
				[ActionStateWeak, Edit]() -> std::string {
					auto ActionState = ActionStateWeak.lock();
					return (ActionState && ActionState->GetState() == Edit) ? "playing" : "-";
				},
				Entry);
		}

		// 편집 중인 동작의 Launch/Knock을 자기 자신에게 걸어본다.
		// 판정이 아직 없어서 띄우기 → 다운 → 기상을 눈으로 볼 방법이 이것뿐이다.
		AddActionRow(Y, TEXT("TestHit"), TEXT("Hit"),
			[ActionStateWeak, Edit]() {
				auto ActionState = ActionStateWeak.lock();
				if (!ActionState) return;

				const FActionStateDef& StateDef = ActionState->GetStateDef(Edit);
				ActionState->RequestHit(StateDef.LaunchPower, StateDef.KnockPower);
			},
			[ActionStateWeak]() -> std::string {
				auto ActionState = ActionStateWeak.lock();
				if (!ActionState) return "-";
				return CActionStateComponent::IsKnockdown(ActionState->GetState()) ? "down" : "-";
			},
			Entry);

		// 쓰러진 뒤 그대로 누워 있는 시간. 다운 무적이 유지되는 길이다.
		AddPropRow(Y, TEXT("DownTime"), 0.1f,
			[ActionStateWeak]() -> float { auto A = ActionStateWeak.lock(); return A ? A->GetDownTime() : 0.f; },
			[ActionStateWeak](float Value) { auto A = ActionStateWeak.lock(); if (A) A->SetDownTime(Value); },
			Entry);

		// 컴포넌트 전체 설정
		AddPropRow(Y, TEXT("MinHold"), 0.01f,
			[ActionStateWeak]() -> float { auto ActionState = ActionStateWeak.lock(); return ActionState ? ActionState->GetMinStateTime() : 0.f; },
			[ActionStateWeak](float Value) { auto ActionState = ActionStateWeak.lock(); if (ActionState) ActionState->SetMinStateTime(Value < 0.f ? 0.f : Value); },
			Entry);

		AddPropRow(Y, TEXT("Timeout"), 0.5f,
			[ActionStateWeak]() -> float { auto ActionState = ActionStateWeak.lock(); return ActionState ? ActionState->GetActionTimeout() : 0.f; },
			[ActionStateWeak](float Value) { auto ActionState = ActionStateWeak.lock(); if (ActionState) ActionState->SetActionTimeout(Value < 0.1f ? 0.1f : Value); },
			Entry);

		// 공격이 끝난 뒤 다음 타를 기다려주는 시간.
		// 이 시간 안에 공격키를 다시 누르면 2타 → 3타로 이어지고, 넘기면 1타부터 다시 나간다.
		AddPropRow(Y, TEXT("ComboReset"), 0.05f,
			[ActionStateWeak]() -> float { auto ActionState = ActionStateWeak.lock(); return ActionState ? ActionState->GetComboResetTime() : 0.f; },
			[ActionStateWeak](float Value) { auto ActionState = ActionStateWeak.lock(); if (ActionState) ActionState->SetComboResetTime(Value); },
			Entry);

		// 접수 시간 동안 이만큼 움직이면 시간이 남았어도 콤보를 버린다.
		// 방향키를 걸친 것만으로는 안 끊기게 하려고 "상태"가 아니라 "거리"로 잰다.
		// 걷기가 0.3이니 0.06이면 0.2초쯤 걸어야 끊긴다. 0으로 두면 이동으로는 안 끊긴다.
		AddPropRow(Y, TEXT("ComboMove"), 0.02f,
			[ActionStateWeak]() -> float { auto ActionState = ActionStateWeak.lock(); return ActionState ? ActionState->GetComboMoveLimit() : 0.f; },
			[ActionStateWeak](float Value) { auto ActionState = ActionStateWeak.lock(); if (ActionState) ActionState->SetComboMoveLimit(Value); },
			Entry);

		// ── 점프 ──
		// 뛰어오르는 높이. 월드 유닛이라 캐릭터 한 칸(0.8) 기준으로 잡으면 된다.
		AddPropRow(Y, TEXT("JumpHeight"), 0.05f,
			[ActionStateWeak]() -> float { auto ActionState = ActionStateWeak.lock(); return ActionState ? ActionState->GetJumpHeight() : 0.f; },
			[ActionStateWeak](float Value) { auto ActionState = ActionStateWeak.lock(); if (ActionState) ActionState->SetJumpHeight(Value); },
			Entry);

		// 공중에 머무는 시간. 0이면 점프 애니메이션 길이에 맞춰서 착지한다.
		AddPropRow(Y, TEXT("JumpTime"), 0.05f,
			[ActionStateWeak]() -> float { auto ActionState = ActionStateWeak.lock(); return ActionState ? ActionState->GetJumpTime() : 0.f; },
			[ActionStateWeak](float Value) { auto ActionState = ActionStateWeak.lock(); if (ActionState) ActionState->SetJumpTime(Value); },
			Entry);

		// 공중에서 방향키 이동에 곱하는 배율.
		// 1이면 땅에서와 같은 속도(WalkSpeed/RunSpeed)로 움직이고,
		// 0이면 뛴 자리에 그대로 떨어진다. 0.5쯤이면 공중에서 굼떠진다.
		AddPropRow(Y, TEXT("JumpMove"), 0.1f,
			[ActionStateWeak]() -> float { auto ActionState = ActionStateWeak.lock(); return ActionState ? ActionState->GetJumpMoveScale() : 0.f; },
			[ActionStateWeak](float Value) { auto ActionState = ActionStateWeak.lock(); if (ActionState) ActionState->SetJumpMoveScale(Value); },
			Entry);

		// 점프를 눈으로 확인하는 용도라 표시만 한다.
		// 떠 있는 동안에는 지금 높이를, 땅에 있을 때는 실제로 적용되는 체공 시간을 보여준다.
		// (JumpTime이 0일 때 그 "0"이 몇 초로 풀리는지 여기서 확인하면 된다)
		AddActionRow(Y, TEXT("Air"), TEXT("-"),
			nullptr,
			[ActionStateWeak]() -> std::string {
				auto ActionState = ActionStateWeak.lock();
				if (!ActionState) return "-";

				// 표시는 바이트 단위로 넓혀지므로 ASCII만 쓴다.
				char Buf[32] = {};

				if (ActionState->IsInAir())
					sprintf_s(Buf, "%.2f up", ActionState->GetAirHeight());
				else
					sprintf_s(Buf, "T %.2f", ActionState->GetAirTime());

				return Buf;
			},
			Entry);

		// 지금 몇 타까지 이어져 있는지. 돌려보면서 확인하는 용도라 표시만 한다.
		AddActionRow(Y, TEXT("Combo"), TEXT("-"),
			nullptr,
			[ActionStateWeak]() -> std::string {
				auto ActionState = ActionStateWeak.lock();
				if (!ActionState) return "-";

				const int Stage = ActionState->GetComboStage();
				if (Stage <= 0) return "-";

				// 표시는 바이트 단위로 넓혀지므로 ASCII만 쓴다.
				std::string Text = std::to_string(Stage) + " hit";
				if (ActionState->IsComboWindowOpen()) Text += " (open)";

				return Text;
			},
			Entry);

		return;
	}

	// ── 방향키 입력 ──────────────────────────────────────────────────────────
	if (auto InputComp = std::dynamic_pointer_cast<CDirectionInputComponent>(Comp))
	{
		std::weak_ptr<CDirectionInputComponent> DirectionInputWeak = InputComp;

		AddSectionHeader(Y, TEXT("Direction Input"));

		//증감 단위는 1. 이 월드는 1 유닛이 수십 픽셀이라 10씩 움직이면 너무 거칠다.
		//정확한 값이 필요하면 값을 더블클릭해서 직접 입력하면 된다.
		AddPropRow(Y, TEXT("WalkSpeed"), 1.f,
			[DirectionInputWeak]() -> float { auto DirectionInput = DirectionInputWeak.lock(); return DirectionInput ? DirectionInput->GetWalkSpeed() : 0.f; },
			[DirectionInputWeak](float Value) { auto DirectionInput = DirectionInputWeak.lock(); if (DirectionInput) DirectionInput->SetWalkSpeed(Value < 0.f ? 0.f : Value); },
			Entry);

		AddPropRow(Y, TEXT("RunSpeed"), 1.f,
			[DirectionInputWeak]() -> float { auto DirectionInput = DirectionInputWeak.lock(); return DirectionInput ? DirectionInput->GetRunSpeed() : 0.f; },
			[DirectionInputWeak](float Value) { auto DirectionInput = DirectionInputWeak.lock(); if (DirectionInput) DirectionInput->SetRunSpeed(Value < 0.f ? 0.f : Value); },
			Entry);

		//달리는 중 위/아래로만 갈 때의 속도. WalkSpeed에 이 값을 곱한다.
		//위/아래는 화면 안쪽 깊이라 좌우와 같은 속도로 뛰면 너무 빠르게 파고든다.
		//1로 두면 걸을 때와 같고, 올릴수록 안쪽으로 빨리 들어간다.
		AddPropRow(Y, TEXT("RunVert"), 0.1f,
			[DirectionInputWeak]() -> float { auto DirectionInput = DirectionInputWeak.lock(); return DirectionInput ? DirectionInput->GetRunVertScale() : 0.f; },
			[DirectionInputWeak](float Value) { auto DirectionInput = DirectionInputWeak.lock(); if (DirectionInput) DirectionInput->SetRunVertScale(Value); },
			Entry);

		// 같은 방향키를 이 시간 안에 두 번 누르면 달리기로 본다.
		AddPropRow(Y, TEXT("DoubleTap"), 0.02f,
			[DirectionInputWeak]() -> float { auto DirectionInput = DirectionInputWeak.lock(); return DirectionInput ? DirectionInput->GetDoubleTapTime() : 0.f; },
			[DirectionInputWeak](float Value) { auto DirectionInput = DirectionInputWeak.lock(); if (DirectionInput) DirectionInput->SetDoubleTapTime(Value < 0.05f ? 0.05f : Value); },
			Entry);

		// 공격키. 인풋이 VK 코드만 받으므로 후보를 정해두고 고르게 한다.
		{
			std::vector<std::string> KeyNames = { "Z", "X", "C", "Space", "Ctrl" };

			void* DropdownKey = (void*)((char*)InputComp.get() + 1);
			AddDropdownRow(Y, TEXT("AttackKey"), DropdownKey, std::move(KeyNames),
				[DirectionInputWeak]() -> std::string {
					auto DirectionInput = DirectionInputWeak.lock();
					if (!DirectionInput) return "Z";

					switch (DirectionInput->GetAttackKey())
					{
					case 'X':        return "X";
					case 'C':        return "C";
					case VK_SPACE:   return "Space";
					case VK_LCONTROL: return "Ctrl";
					default:         return "Z";
					}
				},
				[DirectionInputWeak](const std::string& Name) {
					auto DirectionInput = DirectionInputWeak.lock();
					if (!DirectionInput) return;

					unsigned char Key = 'Z';
					if      (Name == "X")     Key = 'X';
					else if (Name == "C")     Key = 'C';
					else if (Name == "Space") Key = VK_SPACE;
					else if (Name == "Ctrl")  Key = VK_LCONTROL;

					DirectionInput->SetAttackKey(Key);
				},
				Entry);
		}

		// 점프키. 공격키와 같은 후보에서 고른다. (기본 Space)
		{
			std::vector<std::string> KeyNames = { "Space", "Z", "X", "C", "Ctrl" };

			void* DropdownKey = (void*)((char*)InputComp.get() + 2);
			AddDropdownRow(Y, TEXT("JumpKey"), DropdownKey, std::move(KeyNames),
				[DirectionInputWeak]() -> std::string {
					auto DirectionInput = DirectionInputWeak.lock();
					if (!DirectionInput) return "Space";

					switch (DirectionInput->GetJumpKey())
					{
					case 'Z':         return "Z";
					case 'X':         return "X";
					case 'C':         return "C";
					case VK_LCONTROL: return "Ctrl";
					default:          return "Space";
					}
				},
				[DirectionInputWeak](const std::string& Name) {
					auto DirectionInput = DirectionInputWeak.lock();
					if (!DirectionInput) return;

					unsigned char Key = VK_SPACE;
					if      (Name == "Z")    Key = 'Z';
					else if (Name == "X")    Key = 'X';
					else if (Name == "C")    Key = 'C';
					else if (Name == "Ctrl") Key = VK_LCONTROL;

					DirectionInput->SetJumpKey(Key);
				},
				Entry);
		}

		return;
	}

	// ── 높이 (점프 / 띄우기 / 낙하) ──────────────────────────────────────────
	if (auto HeightComp = std::dynamic_pointer_cast<CHeightComponent>(Comp))
	{
		std::weak_ptr<CHeightComponent> HeightWeak = HeightComp;

		AddSectionHeader(Y, TEXT("Height"));

		//끌어내리는 가속도. 점프는 높이/체공시간에서 이 값을 스스로 정하지만,
		//피격으로 뜬 뒤 떨어지는 속도는 이 값이 그대로 결정한다.
		AddPropRow(Y, TEXT("Gravity"), 0.5f,
			[HeightWeak]() -> float { auto H = HeightWeak.lock(); return H ? H->GetGravity() : 0.f; },
			[HeightWeak](float Value) { auto H = HeightWeak.lock(); if (H) H->SetGravity(Value); },
			Entry);

		//착지 반발. 0이면 안 튀고, 0.3쯤이면 다운될 때 한 번 통 튕긴다.
		AddPropRow(Y, TEXT("Bounce"), 0.05f,
			[HeightWeak]() -> float { auto H = HeightWeak.lock(); return H ? H->GetBounce() : 0.f; },
			[HeightWeak](float Value) { auto H = HeightWeak.lock(); if (H) H->SetBounce(Value); },
			Entry);

		//맞고 밀려나는 속도가 잦아드는 정도. 클수록 빨리 멈춘다.
		AddPropRow(Y, TEXT("KnockDamp"), 0.5f,
			[HeightWeak]() -> float { auto H = HeightWeak.lock(); return H ? H->GetKnockDamp() : 0.f; },
			[HeightWeak](float Value) { auto H = HeightWeak.lock(); if (H) H->SetKnockDamp(Value); },
			Entry);

		//지금 얼마나 떠 있는지. 돌려보며 확인하는 용도라 표시만 한다.
		AddActionRow(Y, TEXT("Air"), TEXT("-"),
			nullptr,
			[HeightWeak]() -> std::string {
				auto H = HeightWeak.lock();
				if (!H) return "-";
				if (!H->IsInAir()) return "ground";

				char Buf[32] = {};
				sprintf_s(Buf, "%.2f up", H->GetHeight());
				return Buf;
			},
			Entry);

		return;
	}

	// ── 체력 ─────────────────────────────────────────────────────────────────
	if (auto StatusComp = std::dynamic_pointer_cast<CStatusComponent>(Comp))
	{
		std::weak_ptr<CStatusComponent> StatusWeak = StatusComp;

		AddSectionHeader(Y, TEXT("Status"));

		AddPropRow(Y, TEXT("HP"), 1.f,
			[StatusWeak]() -> float { auto Status = StatusWeak.lock(); return Status ? Status->GetHP() : 0.f; },
			[StatusWeak](float Value) { auto Status = StatusWeak.lock(); if (Status) Status->SetHP(Value); },
			Entry);

		AddPropRow(Y, TEXT("HPMax"), 1.f,
			[StatusWeak]() -> float { auto Status = StatusWeak.lock(); return Status ? Status->GetHPMax() : 0.f; },
			[StatusWeak](float Value) { auto Status = StatusWeak.lock(); if (Status) Status->SetHPMaxOnly(Value); },
			Entry);

		// 피격/사망 상태까지 눌러서 확인할 수 있게 데미지 버튼을 둔다.
		AddActionRow(Y, TEXT("Damage"), TEXT("-1"),
			[StatusWeak]() { auto Status = StatusWeak.lock(); if (Status) Status->AddHP(-1.f); },
			[StatusWeak]() -> std::string {
				auto Status = StatusWeak.lock();
				if (!Status) return "-";
				return std::to_string((int)Status->GetHP()) + " / " + std::to_string((int)Status->GetHPMax());
			},
			Entry);

		return;
	}

	// ── 이동 ─────────────────────────────────────────────────────────────────
	if (auto MoveComp = std::dynamic_pointer_cast<CMovementComponent>(Comp))
	{
		std::weak_ptr<CMovementComponent> MovementWeak = MoveComp;

		AddSectionHeader(Y, TEXT("Movement"));

		// DirectionInput이 붙어 있으면 매 프레임 Walk/Run 속도로 덮어쓴다.
		AddPropRow(Y, TEXT("Speed"), 1.f,
			[MovementWeak]() -> float { auto Movement = MovementWeak.lock(); return Movement ? Movement->GetMoveSpeed() : 0.f; },
			[MovementWeak](float Value) { auto Movement = MovementWeak.lock(); if (Movement) Movement->SetSpeed(Value < 0.f ? 0.f : Value); },
			Entry);

		return;
	}

	auto AnimComp = std::dynamic_pointer_cast<CAnimation2DComponent>(Comp);
	if (!AnimComp) return;

	std::weak_ptr<CAnimation2DComponent> AnimCompWeak = AnimComp;

	AddSectionHeader(Y, TEXT("Animation"));

	// 현재 시퀀스 — 드롭다운으로 갈아탈 수 있다.
	{
		std::vector<std::string> SeqNames;
		for (const auto& Pair : AnimComp->GetAnimationMap())
			SeqNames.push_back(Pair.first);

		if (SeqNames.empty()) SeqNames.push_back("(none)");

		void* DropdownKey = (void*)((char*)AnimComp.get() + 1);
		AddDropdownRow(Y, TEXT("Seq"), DropdownKey, std::move(SeqNames),
			[AnimCompWeak]() -> std::string {
				auto AnimComp = AnimCompWeak.lock();
				if (!AnimComp) return "(none)";
				std::string CurrentName = AnimComp->GetCurrentAnimationName();
				return CurrentName.empty() ? "(none)" : CurrentName;
			},
			[AnimCompWeak](const std::string& Name) {
				auto AnimComp = AnimCompWeak.lock();
				if (AnimComp && Name != "(none)") AnimComp->ChangeAnimation(Name);
			},
			Entry);
	}

	// 재생 중인 프레임 — 표시 전용. 클릭해도 아무 일 없다.
	AddActionRow(Y, TEXT("Frame"), TEXT("-"),
		nullptr,
		[AnimCompWeak]() -> std::string {
			auto AnimComp = AnimCompWeak.lock();
			if (!AnimComp) return "-";

			auto Found = AnimComp->GetAnimationMap().find(AnimComp->GetCurrentAnimationName());
			if (Found == AnimComp->GetAnimationMap().end()) return "-";

			return std::to_string(AnimComp->GetAnimationFrame() + 1) + " / "
			     + std::to_string(Found->second->GetFrameCount());
		},
		Entry);

	// 현재 시퀀스를 집어오는 헬퍼. 시퀀스가 없으면 nullptr.
	auto GetSeq = [](const std::weak_ptr<CAnimation2DComponent>& Weak)
		-> std::shared_ptr<CAnimation2DSequence>
	{
		auto AnimComp = Weak.lock();
		if (!AnimComp) return nullptr;

		auto Found = AnimComp->GetAnimationMap().find(AnimComp->GetCurrentAnimationName());
		return (Found != AnimComp->GetAnimationMap().end()) ? Found->second : nullptr;
	};

	AddPropRow(Y, TEXT("PlayTime"), 0.05f,
		[AnimCompWeak, GetSeq]() -> float { auto Sequence = GetSeq(AnimCompWeak); return Sequence ? Sequence->GetPlayTime() : 1.f; },
		[AnimCompWeak, GetSeq](float Value) {
			auto AnimComp = AnimCompWeak.lock(); if (!AnimComp) return;
			AnimComp->SetPlayTime(AnimComp->GetCurrentAnimationName(), Value < 0.05f ? 0.05f : Value);
		},
		Entry);

	AddPropRow(Y, TEXT("PlayRate"), 0.1f,
		[AnimCompWeak, GetSeq]() -> float { auto Sequence = GetSeq(AnimCompWeak); return Sequence ? Sequence->GetPlayRate() : 1.f; },
		[AnimCompWeak, GetSeq](float Value) {
			auto AnimComp = AnimCompWeak.lock(); if (!AnimComp) return;
			AnimComp->SetPlayRate(AnimComp->GetCurrentAnimationName(), Value < 0.05f ? 0.05f : Value);
		},
		Entry);

	// ON/OFF 세 개 — 누르면 뒤집힌다.
	auto AddFlagRow = [&](const wchar_t* Label,
	                      bool (CAnimation2DSequence::*Get)() const,
	                      void (CAnimation2DComponent::*Set)(const std::string&, bool))
	{
		AddActionRow(Y, Label, TEXT("Toggle"),
			[AnimCompWeak, GetSeq, Get, Set]() {
				auto AnimComp = AnimCompWeak.lock(); if (!AnimComp) return;
				auto Sequence = GetSeq(AnimCompWeak);  if (!Sequence)  return;
				(AnimComp.get()->*Set)(AnimComp->GetCurrentAnimationName(), !(Sequence.get()->*Get)());
			},
			[AnimCompWeak, GetSeq, Get]() -> std::string {
				auto Sequence = GetSeq(AnimCompWeak);
				return Sequence ? ((Sequence.get()->*Get)() ? "ON" : "OFF") : "-";
			},
			Entry);
	};

	AddFlagRow(TEXT("Loop"),     &CAnimation2DSequence::GetLoop,     &CAnimation2DComponent::SetLoop);
	AddFlagRow(TEXT("Reverse"),  &CAnimation2DSequence::GetReverse,  &CAnimation2DComponent::SetReverse);
	AddFlagRow(TEXT("Symmetry"), &CAnimation2DSequence::GetSymmetry, &CAnimation2DComponent::SetSymmetry);
}

// 동적으로 만든 컴포넌트 행들만 스크롤 대상으로 표시한다.
// (이름/트랜스폼 같은 상단 고정 행과 리사이즈 핸들은 제자리에 남는다)
void CInspectorUI::FinishLayout(float ContentEndY)
{
	for (auto& Row : mDynamicRows)
	{
		if (Row)
		{
			Row->SetScrollTarget(true);
		}
	}

	// 패널 크기가 바뀌어도 Rebuild를 거치므로 여기서 같이 갱신한다.
	SetScrollArea(mComponentsStartY - 2.f, GetSize().y);
	SetScrollContentEnd(ContentEndY + 4.f);
}

// ── 삭제 버튼 처리 ───────────────────────────────────────────────────────────
// 제거에 성공하면 true. 호출부는 즉시 Rebuild 후 반환해야 한다.
// (Rebuild가 mCompEntries를 통째로 갈아엎으므로 순회를 계속하면 안 된다.)
bool CInspectorUI::HandleDeleteButtons()
{
	for (auto& Entry : mCompEntries)
	{
		auto DelButton = Entry.DeleteButton.lock();
		if (!DelButton) continue;
		if (DelButton->GetWidgetState() != EWidgetState::Release) continue;

		auto Actor = mTarget.lock();
		auto Comp  = Entry.CompRef.lock();
		if (!Actor || !Comp) return false;

		std::string RemovedName = Comp->GetName();

		if (!Actor->RemoveComponent(Comp))
			return false;

		// 이 컴포넌트에 딸린 UI 상태를 정리한다.
		mExpandState.erase(Entry.CompKey);
		for (auto& Dropdown : Entry.Dropdowns)
			mDropdownOpen.erase(Dropdown.DropKey);

		if (mOnComponentRemoved)
			mOnComponentRemoved(RemovedName);

		return true;
	}

	return false;
}

// ── 값 직접 입력 (더블클릭 → 타이핑 → Enter) ────────────────────────────────

// 인풋은 등록된 키만 GetKey로 조회할 수 있으므로 사용할 키를 미리 바인딩한다.
// SetWorld가 Init 이후에 호출되므로 Init이 아니라 첫 Update에서 처리한다.
void CInspectorUI::RegisterEditKeys()
{
	if (mKeysRegistered) return;

	auto World = mWorld.lock();
	if (!World) return;

	auto Input = World->GetInput().lock();
	if (!Input) return;

	for (int Digit = 0; Digit < 10; ++Digit)
	{
		Input->AddBindKey("InspNum" + std::to_string(Digit), (unsigned char)('0' + Digit));
		Input->AddBindKey("InspPad" + std::to_string(Digit), (unsigned char)(VK_NUMPAD0 + Digit));
	}

	Input->AddBindKey("InspDot",      VK_OEM_PERIOD);
	Input->AddBindKey("InspPadDot",   VK_DECIMAL);
	Input->AddBindKey("InspMinus",    VK_OEM_MINUS);
	Input->AddBindKey("InspPadMinus", VK_SUBTRACT);
	Input->AddBindKey("InspBack",     VK_BACK);
	Input->AddBindKey("InspEnter",    VK_RETURN);
	Input->AddBindKey("InspEsc",      VK_ESCAPE);

	mKeysRegistered = true;
}

void CInspectorUI::BeginEdit(int CompIdx, int PropIdx)
{
	if (CompIdx < 0 || CompIdx >= (int)mCompEntries.size()) return;

	auto& Entry = mCompEntries[CompIdx];
	if (PropIdx < 0 || PropIdx >= (int)Entry.Props.size()) return;

	mEditActive  = true;
	mEditCompIdx = CompIdx;
	mEditPropIdx = PropIdx;

	// 현재 값을 초기 문자열로 채워준다. (바로 Enter를 치면 값이 유지된다)
	auto& Prop = Entry.Props[PropIdx];
	char TextBuffer[32] = {};
	sprintf_s(TextBuffer, 32, "%.2f", Prop.Getter ? Prop.Getter() : 0.f);
	mEditBuffer = TextBuffer;

	// 타이핑하는 숫자키가 스킬/타일모드 등 기존 바인딩을 같이 발동시키지 않게 막는다.
	if (auto World = mWorld.lock())
		if (auto Input = World->GetInput().lock())
			Input->SetBindKeyBlock(true);
}

void CInspectorUI::CancelEdit()
{
	mEditActive  = false;
	mEditCompIdx = -1;
	mEditPropIdx = -1;
	mEditBuffer.clear();

	if (auto World = mWorld.lock())
		if (auto Input = World->GetInput().lock())
			Input->SetBindKeyBlock(false);
}

void CInspectorUI::CommitEdit()
{
	if (!mEditActive) return;

	if (mEditCompIdx >= 0 && mEditCompIdx < (int)mCompEntries.size())
	{
		auto& Entry = mCompEntries[mEditCompIdx];

		if (mEditPropIdx >= 0 && mEditPropIdx < (int)Entry.Props.size())
		{
			auto& Prop = Entry.Props[mEditPropIdx];

			// "-" 나 "." 만 남은 미완성 입력은 무시한다.
			if (Prop.Setter && !mEditBuffer.empty() && mEditBuffer != "-" && mEditBuffer != ".")
			{
				try
				{
					Prop.Setter(std::stof(mEditBuffer));
				}
				catch (...)
				{
					// 파싱 실패 시 값을 건드리지 않는다.
				}
			}
		}
	}

	CancelEdit();
}

void CInspectorUI::HandleValueEditInput()
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

		if (mEditCompIdx >= 0 && mEditCompIdx < (int)mCompEntries.size())
		{
			auto& Entry = mCompEntries[mEditCompIdx];
			if (mEditPropIdx >= 0 && mEditPropIdx < (int)Entry.Props.size())
				EditButton = Entry.Props[mEditPropIdx].ValueButton.lock();
		}

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
	for (int d = 0; d < 10; ++d)
	{
		bool bPressed = Input->GetKey((unsigned char)('0' + d), EInputType::Press)
		             || Input->GetKey((unsigned char)(VK_NUMPAD0 + d), EInputType::Press);

		if (bPressed && (int)mEditBuffer.size() < EDIT_BUF_MAX)
			mEditBuffer.push_back((char)('0' + d));
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

void CInspectorUI::DetectValueDoubleClick()
{
	for (int CompIdx = 0; CompIdx < (int)mCompEntries.size(); ++CompIdx)
	{
		auto& Entry = mCompEntries[CompIdx];
		if (!Entry.bExpanded) continue;

		for (int PropIdx = 0; PropIdx < (int)Entry.Props.size(); ++PropIdx)
		{
			auto Button = Entry.Props[PropIdx].ValueButton.lock();
			if (!Button) continue;
			if (Button->GetWidgetState() != EWidgetState::Release) continue;

			void* ClickKey = Button.get();

			// 같은 버튼을 짧은 간격으로 두 번 → 편집 시작
			if (mLastClickKey == ClickKey && (mTimeAccum - mLastClickTime) <= DOUBLE_CLICK_SEC)
			{
				BeginEdit(CompIdx, PropIdx);
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

// ── 업데이트 ─────────────────────────────────────────────────────────────────
void CInspectorUI::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);

	mTimeAccum += DeltaTime;
	RegisterEditKeys();

	auto World = mWorld.lock();
	if (!World) return;

	// ── 리사이즈 로직 ────────────────────────────────────────────────────────
	if (auto Input = World->GetInput().lock())
	{
		FVector2 MousePos = Input->GetMousePos();
		FVector2 Delta    = Input->GetMouseMove();
		bool Press   = Input->GetMouseState(EMouseType::LButton, EInputType::Press);
		bool Held    = Input->GetMouseState(EMouseType::LButton, EInputType::Hold);
		bool Release = Input->GetMouseState(EMouseType::LButton, EInputType::Release);

		FVector3 PanelPos  = GetPos();
		FVector3 PanelSize = GetSize();
		float PanelWidth = PanelSize.x, PanelHeight = PanelSize.y;

		float CornerX[4] = { PanelPos.x, PanelPos.x + PanelWidth - HANDLE_SZ, PanelPos.x, PanelPos.x + PanelWidth - HANDLE_SZ };
		float CornerY[4] = { PanelPos.y, PanelPos.y, PanelPos.y + PanelHeight - HANDLE_SZ, PanelPos.y + PanelHeight - HANDLE_SZ };

		if (mActiveCorner == -1 && Press)
		{
			for (int i = 0; i < 4; ++i)
			{
				if (MousePos.x >= CornerX[i] && MousePos.x < CornerX[i] + HANDLE_SZ &&
					MousePos.y >= CornerY[i] && MousePos.y < CornerY[i] + HANDLE_SZ)
				{
					mActiveCorner = i; break;
				}
			}
		}
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
			NewWidth = max(NewWidth, 120.f); NewHeight = max(NewHeight, 80.f);
			SetPos(NewX, NewY); SetSize(NewWidth, NewHeight);
			if (auto Background = mBackground.lock())    Background->SetSize(NewWidth, NewHeight);
			if (auto TitleBar = mTitleBarWidget.lock()) TitleBar->SetSize(NewWidth, TITLE_H);
			if (auto Handle = mHandleTL.lock()) Handle->SetPos(0.f,           0.f);
			if (auto Handle = mHandleTR.lock()) Handle->SetPos(NewWidth - HANDLE_SZ, 0.f);
			if (auto Handle = mHandleBL.lock()) Handle->SetPos(0.f,           NewHeight - HANDLE_SZ);
			if (auto Handle = mHandleBR.lock()) Handle->SetPos(NewWidth - HANDLE_SZ, NewHeight - HANDLE_SZ);
			UpdateAllRowWidths(NewWidth);
			// 리사이즈는 Rebuild를 거치지 않으므로 스크롤 영역을 직접 갱신한다.
			SetScrollArea(mComponentsStartY - 2.f, NewHeight);
		}
	}

	// ── 루트 트랜스폼 실시간 표시 ────────────────────────────────────────────
	auto Actor = mTarget.lock();
	if (Actor)
	{
		if (auto Root = Actor->GetRootComponent().lock())
		{
			TCHAR TextBuffer[128] = {};
			auto Pos   = Root->GetWorldPos();
			auto Rot   = Root->GetWorldRot();
			auto Scale = Root->GetWorldScale();
			if (auto Text = mPosText.lock())   { swprintf_s(TextBuffer, 128, L"Pos: %.1f %.1f", Pos.x, Pos.y);    Text->SetText(TextBuffer); }
			if (auto Text = mRotText.lock())   { swprintf_s(TextBuffer, 128, L"Rot: %.1f",       Rot.z);            Text->SetText(TextBuffer); }
			if (auto Text = mScaleText.lock()) { swprintf_s(TextBuffer, 128, L"Scl: %.2f %.2f",  Scale.x, Scale.y); Text->SetText(TextBuffer); }
		}
	}

	// ── 값 직접 입력: 편집 중 키 처리 → 새 더블클릭 감지 순서 ───────────────
	HandleValueEditInput();
	DetectValueDoubleClick();

	// ── 액션 버튼 폴링 (사이클 / 탐색) ─────────────────────────────────────
	for (auto& Entry : mCompEntries)
	{
		if (!Entry.bExpanded) continue;
		for (auto& Action : Entry.Actions)
		{
			if (auto Button = Action.ActionButton.lock())
			{
				if (Button->GetWidgetState() == EWidgetState::Release)
				{
					if (Action.OnClick) Action.OnClick();
				}
			}

			// 표시 값은 매 프레임 다시 읽는다.
			// 클릭했을 때만 갱신하면 실행 중에 바뀌는 값(현재 상태, 재생 프레임, HP)이
			// 처음 만들어질 때의 값으로 굳어버린다.
			if (Action.GetDisplay)
			{
				if (auto Disp = Action.DisplayLabel.lock())
				{
					std::string Display = Action.GetDisplay();

					if (Display != Action.LastDisplay)
					{
						Action.LastDisplay = Display;

						std::wstring WideText = DialogUtil::ToWide(Display);
						Disp->SetText(WideText.c_str());
					}
				}
			}
		}
	}

	// ── 드롭다운 토글 + 항목 선택 ───────────────────────────────────────────
	{
		bool bDDRebuild = false;
		for (auto& Entry : mCompEntries)
		{
			if (!Entry.bExpanded) continue;
			for (auto& Dropdown : Entry.Dropdowns)
			{
				// 열기/닫기 토글
				if (auto ToggleButton = Dropdown.ToggleButton.lock())
				{
					if (ToggleButton->GetWidgetState() == EWidgetState::Release)
					{
						bool bCur = mDropdownOpen.count(Dropdown.DropKey) && mDropdownOpen[Dropdown.DropKey];
						mDropdownOpen[Dropdown.DropKey] = !bCur;
						bDDRebuild = true;
						break;
					}
				}

				// 항목 선택
				for (int k = 0; k < (int)Dropdown.ItemButtons.size(); ++k)
				{
					if (auto ItemButton = Dropdown.ItemButtons[k].lock())
					{
						if (ItemButton->GetWidgetState() == EWidgetState::Release)
						{
							if (Dropdown.Setter) Dropdown.Setter(Dropdown.Items[k]);
							// 값 레이블 즉시 업데이트
							if (auto ValueLabel = Dropdown.ValueLabel.lock())
							{
								std::wstring WideText(Dropdown.Items[k].begin(), Dropdown.Items[k].end());
								ValueLabel->SetText(WideText.c_str());
							}
							mDropdownOpen[Dropdown.DropKey] = false;
							bDDRebuild = true;
							break;
						}
					}
				}
				if (bDDRebuild) break;
			}
			if (bDDRebuild) break;
		}
		if (bDDRebuild) { Rebuild(); return; }
	}

	// ── 컴포넌트 [X] 클릭 → 액터에서 제거 ───────────────────────────────────
	if (HandleDeleteButtons()) { Rebuild(); return; }

	// ── 컴포넌트 헤더 클릭 → 펼치기/접기 토글 ──────────────────────────────
	bool bNeedRebuild = false;
	for (auto& Entry : mCompEntries)
	{
		if (auto Button = Entry.HeaderButton.lock())
		{
			if (Button->GetWidgetState() == EWidgetState::Release)
			{
				mExpandState[Entry.CompKey] = !Entry.bExpanded;
				// 헤더 토글 시 해당 컴포넌트의 드롭다운은 모두 닫음
				for (auto& Dropdown : Entry.Dropdowns)
					mDropdownOpen[Dropdown.DropKey] = false;
				bNeedRebuild = true;
				break;
			}
		}
	}
	if (bNeedRebuild) { Rebuild(); return; }

	// ── 속성 +/- 버튼 인터랙션 ──────────────────────────────────────────────
	for (int CompIdx = 0; CompIdx < (int)mCompEntries.size(); ++CompIdx)
	{
		auto& Entry = mCompEntries[CompIdx];
		if (!Entry.bExpanded) continue;

		for (int PropIdx = 0; PropIdx < (int)Entry.Props.size(); ++PropIdx)
		{
			auto& Prop = Entry.Props[PropIdx];

			// 편집 중인 행은 +/- 와 실시간 표시를 건너뛰고 입력 버퍼를 보여준다.
			if (mEditActive && CompIdx == mEditCompIdx && PropIdx == mEditPropIdx)
			{
				if (auto Value = Prop.ValueLabel.lock())
				{
					std::wstring WideBuffer(mEditBuffer.begin(), mEditBuffer.end());
					WideBuffer += L"_";   // 캐럿
					Value->SetText(WideBuffer.c_str());
					Value->SetTextColor(FVector4(0.55f, 1.f, 0.65f, 1.f));
				}
				continue;
			}

			if (!Prop.Getter || !Prop.Setter) continue;

			// 감소
			if (auto MinButton = Prop.MinusButton.lock())
			{
				EWidgetState::Type State = MinButton->GetWidgetState();
				if (State == EWidgetState::Clicked)
				{
					Prop.MinHoldTime += DeltaTime;
					int Expected = (Prop.MinHoldTime > 0.35f)
						? (int)((Prop.MinHoldTime - 0.35f) / 0.08f) + 1 : 0;
					while (Prop.MinRepeat < Expected)
					{
						Prop.Setter(Prop.Getter() - Prop.Step);
						++Prop.MinRepeat;
					}
				}
				else
				{
					if (State == EWidgetState::Release && Prop.MinHoldTime < 0.35f)
						Prop.Setter(Prop.Getter() - Prop.Step);
					Prop.MinHoldTime = 0.f;
					Prop.MinRepeat   = 0;
				}
			}

			// 증가
			if (auto PlusButton = Prop.PlusButton.lock())
			{
				EWidgetState::Type State = PlusButton->GetWidgetState();
				if (State == EWidgetState::Clicked)
				{
					Prop.PlusHoldTime += DeltaTime;
					int Expected = (Prop.PlusHoldTime > 0.35f)
						? (int)((Prop.PlusHoldTime - 0.35f) / 0.08f) + 1 : 0;
					while (Prop.PlusRepeat < Expected)
					{
						Prop.Setter(Prop.Getter() + Prop.Step);
						++Prop.PlusRepeat;
					}
				}
				else
				{
					if (State == EWidgetState::Release && Prop.PlusHoldTime < 0.35f)
						Prop.Setter(Prop.Getter() + Prop.Step);
					Prop.PlusHoldTime = 0.f;
					Prop.PlusRepeat   = 0;
				}
			}

			// 실시간 값 표시 업데이트
			if (auto Value = Prop.ValueLabel.lock())
			{
				TCHAR TextBuffer[32] = {};
				swprintf_s(TextBuffer, 32, L"%.2f", Prop.Getter());
				Value->SetText(TextBuffer);
				Value->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
			}
		}
	}
}

// ── 모든 행 너비 갱신 ────────────────────────────────────────────────────────
void CInspectorUI::UpdateAllRowWidths(float NewWidth)
{
	auto SetRowWidth = [&](std::weak_ptr<CTextBlock> Weak)
	{
		if (auto Text = Weak.lock()) Text->SetSize(NewWidth - 8.f, ROW_H);
	};
	if (auto Text = mTitleText.lock()) Text->SetSize(NewWidth, TITLE_H);
	SetRowWidth(mActorNameText);
	SetRowWidth(mActorTagText);
	SetRowWidth(mTransformHeader);
	SetRowWidth(mRootNameText);
	SetRowWidth(mPosText);
	SetRowWidth(mRotText);
	SetRowWidth(mScaleText);
	SetRowWidth(mCompHeader);
	Rebuild();
}

CInspectorUI* CInspectorUI::Clone()
{
	return new CInspectorUI(*this);
}
