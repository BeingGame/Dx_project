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
#include "DialogUtil.h"

#include <cstdio>   // sprintf_s (값 직접 입력 버퍼 초기화)

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
	auto Bg = CreateWidget<CButton>("InspectorBg", 0).lock();
	if (Bg)
	{
		Bg->SetPos(0.f, 0.f);
		Bg->SetSize(PANEL_W, PANEL_H);
		Bg->SetTint(EWidgetState::Normal,  0.13f, 0.13f, 0.15f, 0.92f);
		Bg->SetTint(EWidgetState::Hovered, 0.13f, 0.13f, 0.15f, 0.92f);
		Bg->SetTint(EWidgetState::Clicked, 0.13f, 0.13f, 0.15f, 0.92f);
		Bg->SetTint(EWidgetState::Release, 0.13f, 0.13f, 0.15f, 0.92f);
		Bg->SetTint(EWidgetState::Disable, 0.13f, 0.13f, 0.15f, 0.92f);
		mBackground = Bg;
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
	if (auto T = mTransformHeader.lock())
		T->SetTextColor(FVector4(0.6f, 0.85f, 1.f, 1.f));

	mRootNameText = AddRow(Y, TEXT("Root: -"));   Y += ROW_H + 2.f;
	mPosText      = AddRow(Y, TEXT("Pos: - "));   Y += ROW_H + 2.f;
	mRotText      = AddRow(Y, TEXT("Rot: - "));   Y += ROW_H + 2.f;
	mScaleText    = AddRow(Y, TEXT("Scale: - ")); Y += ROW_H + 6.f;

	mCompHeader = AddRow(Y, TEXT("[ Components ]"), 12.f); Y += ROW_H + 6.f;
	if (auto T = mCompHeader.lock())
		T->SetTextColor(FVector4(0.6f, 0.85f, 1.f, 1.f));

	mComponentsStartY = Y;

	// 코너 리사이즈 핸들 (ZOrder 10)
	auto MakeHandle = [&](const std::string& Name, float X, float Y2) -> std::weak_ptr<CButton>
	{
		auto H = CreateWidget<CButton>(Name, 10).lock();
		if (H)
		{
			H->SetPos(X, Y2);
			H->SetSize(HANDLE_SZ, HANDLE_SZ);
			H->SetTint(EWidgetState::Normal,  0.45f, 0.45f, 0.55f, 0.85f);
			H->SetTint(EWidgetState::Hovered, 0.70f, 0.90f, 1.00f, 1.f);
			H->SetTint(EWidgetState::Clicked, 1.00f, 1.00f, 1.00f, 1.f);
			H->SetTint(EWidgetState::Release, 1.00f, 1.00f, 1.00f, 1.f);
			H->SetTint(EWidgetState::Disable, 0.30f, 0.30f, 0.30f, 0.5f);
		}
		return H;
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
std::weak_ptr<CButton> CInspectorUI::AddDeleteButton(float Y, float PanelW, int I)
{
	float X = PanelW - DEL_BTN_W - 2.f;

	auto DelBtn = CreateWidget<CButton>("ICX_" + std::to_string(I), 5).lock();
	if (DelBtn)
	{
		DelBtn->SetPos(X, Y);
		DelBtn->SetSize(DEL_BTN_W, ROW_H);
		DelBtn->SetTint(EWidgetState::Normal,  0.42f, 0.16f, 0.18f, 1.f);
		DelBtn->SetTint(EWidgetState::Hovered, 0.72f, 0.22f, 0.24f, 1.f);
		DelBtn->SetTint(EWidgetState::Clicked, 0.92f, 0.32f, 0.34f, 1.f);
		DelBtn->SetTint(EWidgetState::Release, 0.72f, 0.22f, 0.24f, 1.f);
		DelBtn->SetTint(EWidgetState::Disable, 0.24f, 0.16f, 0.16f, 1.f);
		mDynamicRows.push_back(DelBtn);
	}

	auto DelLbl = CreateWidget<CTextBlock>("ICXL_" + std::to_string(I), 6).lock();
	if (DelLbl)
	{
		DelLbl->SetPos(X, Y);
		DelLbl->SetSize(DEL_BTN_W, ROW_H);
		DelLbl->SetText(TEXT("X"));
		DelLbl->SetFontSize(12.f);
		DelLbl->SetTextColor(FVector4::White);
		DelLbl->SetAlignH(ETextAlignH::Center);
		DelLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(DelLbl);
	}

	return DelBtn;
}

// ── AddSectionHeader: 얇은 구분선 레이블 ─────────────────────────────────────
void CInspectorUI::AddSectionHeader(float& Y, const wchar_t* Text)
{
	int I = mDynIdx++;
	auto Lbl = CreateWidget<CTextBlock>("ISH_" + std::to_string(I), 3).lock();
	if (Lbl)
	{
		Lbl->SetPos(6.f, Y);
		Lbl->SetSize(GetSize().x - 8.f, 16.f);
		Lbl->SetText(Text);
		Lbl->SetFontSize(11.f);
		Lbl->SetTextColor(FVector4(0.55f, 0.75f, 1.f, 1.f));
		Lbl->SetAlignH(ETextAlignH::Left);
		Lbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(Lbl);
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
	int I = mDynIdx++;
	float W = GetSize().x;
	bool bOpen = mDropdownOpen.count(DropKey) && mDropdownOpen[DropKey];

	// ── 헤더 행 ──────────────────────────────────────────────────────────────
	auto NameLbl = CreateWidget<CTextBlock>("IDL_" + std::to_string(I), 3).lock();
	if (NameLbl)
	{
		NameLbl->SetPos(14.f, Y);
		NameLbl->SetSize(40.f, ROW_H - 2.f);
		NameLbl->SetText(Label);
		NameLbl->SetFontSize(11.f);
		NameLbl->SetTextColor(FVector4(0.72f, 0.72f, 0.72f, 1.f));
		NameLbl->SetAlignH(ETextAlignH::Left);
		NameLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(NameLbl);
	}

	std::string CurVal = Getter ? Getter() : "";
	std::wstring WCur(CurVal.begin(), CurVal.end());

	auto ValLbl = CreateWidget<CTextBlock>("IDV_" + std::to_string(I), 3).lock();
	if (ValLbl)
	{
		ValLbl->SetPos(56.f, Y);
		ValLbl->SetSize(82.f, ROW_H - 2.f);
		ValLbl->SetText(WCur.c_str());
		ValLbl->SetFontSize(10.f);
		ValLbl->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		ValLbl->SetAlignH(ETextAlignH::Left);
		ValLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(ValLbl);
	}

	// 토글 버튼: ▼ / ▲
	auto TogBtn = CreateWidget<CButton>("IDB_" + std::to_string(I), 3).lock();
	if (TogBtn)
	{
		TogBtn->SetPos(140.f, Y);
		TogBtn->SetSize(44.f, ROW_H - 2.f);
		TogBtn->SetTint(EWidgetState::Normal,  bOpen ? 0.30f : 0.20f, bOpen ? 0.44f : 0.30f, 0.46f, 1.f);
		TogBtn->SetTint(EWidgetState::Hovered, 0.34f, 0.50f, 0.62f, 1.f);
		TogBtn->SetTint(EWidgetState::Clicked, 0.44f, 0.62f, 0.80f, 1.f);
		TogBtn->SetTint(EWidgetState::Release, 0.34f, 0.50f, 0.62f, 1.f);
		TogBtn->SetTint(EWidgetState::Disable, 0.14f, 0.18f, 0.26f, 1.f);
		mDynamicRows.push_back(TogBtn);
	}
	auto TogLbl = CreateWidget<CTextBlock>("IDBL_" + std::to_string(I), 4).lock();
	if (TogLbl)
	{
		TogLbl->SetPos(140.f, Y);
		TogLbl->SetSize(44.f, ROW_H - 2.f);
		TogLbl->SetText(bOpen ? TEXT("▲") : TEXT("▼"));
		TogLbl->SetFontSize(10.f);
		TogLbl->SetTextColor(FVector4::White);
		TogLbl->SetAlignH(ETextAlignH::Center);
		TogLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(TogLbl);
	}

	Y += ROW_H + 2.f;

	FInspDropdown DD;
	DD.DropKey   = DropKey;
	DD.ToggleBtn = TogBtn;
	DD.ValueLbl  = ValLbl;
	DD.Items     = std::move(Items);
	DD.Getter    = Getter;
	DD.Setter    = std::move(Setter);

	// ── 항목 행 (열려 있을 때만 표시) ───────────────────────────────────────
	if (bOpen)
	{
		// 목록 배경 스트립
		auto ListBg = CreateWidget<CButton>("IDBG_" + std::to_string(I), 2).lock();
		if (ListBg)
		{
			ListBg->SetPos(14.f, Y);
			ListBg->SetSize(W - 20.f, (float)DD.Items.size() * (ROW_H + 1.f));
			ListBg->SetTint(EWidgetState::Normal,  0.10f, 0.10f, 0.12f, 0.95f);
			ListBg->SetTint(EWidgetState::Hovered, 0.10f, 0.10f, 0.12f, 0.95f);
			ListBg->SetTint(EWidgetState::Clicked, 0.10f, 0.10f, 0.12f, 0.95f);
			ListBg->SetTint(EWidgetState::Release, 0.10f, 0.10f, 0.12f, 0.95f);
			ListBg->SetTint(EWidgetState::Disable, 0.10f, 0.10f, 0.12f, 0.95f);
			mDynamicRows.push_back(ListBg);
		}

		for (auto& ItemName : DD.Items)
		{
			int J = mDynIdx++;
			bool bSelected = (ItemName == CurVal);

			auto ItemBtn = CreateWidget<CButton>("IDIB_" + std::to_string(J), 5).lock();
			if (ItemBtn)
			{
				ItemBtn->SetPos(14.f, Y);
				ItemBtn->SetSize(W - 20.f, ROW_H);
				ItemBtn->SetTint(EWidgetState::Normal,  bSelected ? 0.18f : 0.10f, bSelected ? 0.36f : 0.14f, bSelected ? 0.28f : 0.18f, 1.f);
				ItemBtn->SetTint(EWidgetState::Hovered, 0.22f, 0.38f, 0.46f, 1.f);
				ItemBtn->SetTint(EWidgetState::Clicked, 0.30f, 0.50f, 0.60f, 1.f);
				ItemBtn->SetTint(EWidgetState::Release, 0.22f, 0.38f, 0.46f, 1.f);
				ItemBtn->SetTint(EWidgetState::Disable, 0.10f, 0.12f, 0.14f, 1.f);
				mDynamicRows.push_back(ItemBtn);
			}

			auto ItemLbl = CreateWidget<CTextBlock>("IDIL_" + std::to_string(J), 6).lock();
			if (ItemLbl)
			{
				ItemLbl->SetPos(20.f, Y);
				ItemLbl->SetSize(W - 26.f, ROW_H);
				std::wstring WItem(ItemName.begin(), ItemName.end());
				ItemLbl->SetText(WItem.c_str());
				ItemLbl->SetFontSize(11.f);
				ItemLbl->SetTextColor(bSelected ? FVector4(0.6f, 1.f, 0.7f, 1.f) : FVector4(0.85f, 0.85f, 0.85f, 1.f));
				ItemLbl->SetAlignH(ETextAlignH::Left);
				ItemLbl->SetAlignV(ETextAlignV::Middle);
				mDynamicRows.push_back(ItemLbl);
			}

			DD.ItemBtns.push_back(ItemBtn);
			Y += ROW_H + 1.f;
		}
		Y += 2.f;
	}

	Entry.Dropdowns.push_back(std::move(DD));
}

// ── AddActionRow: 레이블 + 표시 텍스트 + 액션 버튼 ─────────────────────────
void CInspectorUI::AddActionRow(float& Y,
                                const wchar_t* Label,
                                const wchar_t* BtnText,
                                std::function<void()>        OnClick,
                                std::function<std::string()> GetDisplay,
                                FInspCompEntry& Entry)
{
	int I = mDynIdx++;

	auto Lbl = CreateWidget<CTextBlock>("IAL_" + std::to_string(I), 3).lock();
	if (Lbl)
	{
		Lbl->SetPos(14.f, Y);
		Lbl->SetSize(40.f, ROW_H - 2.f);
		Lbl->SetText(Label);
		Lbl->SetFontSize(11.f);
		Lbl->SetTextColor(FVector4(0.72f, 0.72f, 0.72f, 1.f));
		Lbl->SetAlignH(ETextAlignH::Left);
		Lbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(Lbl);
	}

	// 현재 값 표시
	std::string DispStr = GetDisplay ? GetDisplay() : "";
	std::wstring WDisp(DispStr.begin(), DispStr.end());

	auto DispLbl = CreateWidget<CTextBlock>("IAD_" + std::to_string(I), 3).lock();
	if (DispLbl)
	{
		DispLbl->SetPos(56.f, Y);
		DispLbl->SetSize(82.f, ROW_H - 2.f);
		DispLbl->SetText(WDisp.c_str());
		DispLbl->SetFontSize(10.f);
		DispLbl->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		DispLbl->SetAlignH(ETextAlignH::Left);
		DispLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(DispLbl);
	}

	auto ActBtn = CreateWidget<CButton>("IAB_" + std::to_string(I), 3).lock();
	if (ActBtn)
	{
		ActBtn->SetPos(140.f, Y);
		ActBtn->SetSize(44.f, ROW_H - 2.f);
		ActBtn->SetTint(EWidgetState::Normal,  0.20f, 0.30f, 0.46f, 1.f);
		ActBtn->SetTint(EWidgetState::Hovered, 0.30f, 0.44f, 0.66f, 1.f);
		ActBtn->SetTint(EWidgetState::Clicked, 0.40f, 0.58f, 0.82f, 1.f);
		ActBtn->SetTint(EWidgetState::Release, 0.30f, 0.44f, 0.66f, 1.f);
		ActBtn->SetTint(EWidgetState::Disable, 0.14f, 0.18f, 0.26f, 1.f);
		mDynamicRows.push_back(ActBtn);
	}
	auto ActLbl = CreateWidget<CTextBlock>("IABL_" + std::to_string(I), 4).lock();
	if (ActLbl)
	{
		ActLbl->SetPos(140.f, Y);
		ActLbl->SetSize(44.f, ROW_H - 2.f);
		ActLbl->SetText(BtnText);
		ActLbl->SetFontSize(11.f);
		ActLbl->SetTextColor(FVector4::White);
		ActLbl->SetAlignH(ETextAlignH::Center);
		ActLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(ActLbl);
	}

	FInspAction Action;
	Action.ActionBtn  = ActBtn;
	Action.DisplayLbl = DispLbl;
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
	int I = mDynIdx++;
	float CurW = GetSize().x;

	FInspProp Prop;
	Prop.Step   = Step;
	Prop.Getter = std::move(Getter);
	Prop.Setter = std::move(Setter);

	// 속성 이름 레이블
	auto Lbl = CreateWidget<CTextBlock>("IPL_" + std::to_string(I), 3).lock();
	if (Lbl)
	{
		Lbl->SetPos(14.f, Y);
		Lbl->SetSize(38.f, ROW_H - 2.f);
		Lbl->SetText(Label);
		Lbl->SetFontSize(11.f);
		Lbl->SetTextColor(FVector4(0.72f, 0.72f, 0.72f, 1.f));
		Lbl->SetAlignH(ETextAlignH::Left);
		Lbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(Lbl);
	}

	// [-] 버튼
	auto MinBtn = CreateWidget<CButton>("IPM_" + std::to_string(I), 3).lock();
	if (MinBtn)
	{
		MinBtn->SetPos(54.f, Y);
		MinBtn->SetSize(18.f, ROW_H - 2.f);
		MinBtn->SetTint(EWidgetState::Normal,  0.26f, 0.26f, 0.32f, 1.f);
		MinBtn->SetTint(EWidgetState::Hovered, 0.44f, 0.44f, 0.54f, 1.f);
		MinBtn->SetTint(EWidgetState::Clicked, 0.60f, 0.60f, 0.72f, 1.f);
		MinBtn->SetTint(EWidgetState::Release, 0.44f, 0.44f, 0.54f, 1.f);
		MinBtn->SetTint(EWidgetState::Disable, 0.20f, 0.20f, 0.24f, 1.f);
		mDynamicRows.push_back(MinBtn);
		Prop.MinusBtn = MinBtn;
	}
	auto MinLbl = CreateWidget<CTextBlock>("IPML_" + std::to_string(I), 4).lock();
	if (MinLbl)
	{
		MinLbl->SetPos(54.f, Y);
		MinLbl->SetSize(18.f, ROW_H - 2.f);
		MinLbl->SetText(TEXT("-"));
		MinLbl->SetFontSize(12.f);
		MinLbl->SetTextColor(FVector4::White);
		MinLbl->SetAlignH(ETextAlignH::Center);
		MinLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(MinLbl);
	}

	// 값 표시 배경 — 더블클릭하면 직접 입력 모드로 들어간다.
	// (CTextBlock은 마우스를 먹지 않으므로 아래 ZOrder 2에 깔아도 클릭을 받는다)
	auto ValBtn = CreateWidget<CButton>("IPVB_" + std::to_string(I), 2).lock();
	if (ValBtn)
	{
		ValBtn->SetPos(74.f, Y);
		ValBtn->SetSize(66.f, ROW_H - 2.f);
		ValBtn->SetTint(EWidgetState::Normal,  0.17f, 0.17f, 0.21f, 1.f);
		ValBtn->SetTint(EWidgetState::Hovered, 0.24f, 0.26f, 0.34f, 1.f);
		ValBtn->SetTint(EWidgetState::Clicked, 0.30f, 0.34f, 0.44f, 1.f);
		ValBtn->SetTint(EWidgetState::Release, 0.24f, 0.26f, 0.34f, 1.f);
		ValBtn->SetTint(EWidgetState::Disable, 0.14f, 0.14f, 0.18f, 1.f);
		mDynamicRows.push_back(ValBtn);
		Prop.ValueBtn = ValBtn;
	}

	// 값 표시
	float InitVal = Prop.Getter ? Prop.Getter() : 0.f;
	TCHAR ValBuf[32] = {};
	swprintf_s(ValBuf, 32, L"%.2f", InitVal);

	auto ValLbl = CreateWidget<CTextBlock>("IPV_" + std::to_string(I), 3).lock();
	if (ValLbl)
	{
		ValLbl->SetPos(74.f, Y);
		ValLbl->SetSize(66.f, ROW_H - 2.f);
		ValLbl->SetText(ValBuf);
		ValLbl->SetFontSize(11.f);
		ValLbl->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		ValLbl->SetAlignH(ETextAlignH::Center);
		ValLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(ValLbl);
		Prop.ValueLabel = ValLbl;
	}

	// [+] 버튼
	auto PlusBtn = CreateWidget<CButton>("IPP_" + std::to_string(I), 3).lock();
	if (PlusBtn)
	{
		PlusBtn->SetPos(142.f, Y);
		PlusBtn->SetSize(18.f, ROW_H - 2.f);
		PlusBtn->SetTint(EWidgetState::Normal,  0.26f, 0.26f, 0.32f, 1.f);
		PlusBtn->SetTint(EWidgetState::Hovered, 0.44f, 0.44f, 0.54f, 1.f);
		PlusBtn->SetTint(EWidgetState::Clicked, 0.60f, 0.60f, 0.72f, 1.f);
		PlusBtn->SetTint(EWidgetState::Release, 0.44f, 0.44f, 0.54f, 1.f);
		PlusBtn->SetTint(EWidgetState::Disable, 0.20f, 0.20f, 0.24f, 1.f);
		mDynamicRows.push_back(PlusBtn);
		Prop.PlusBtn = PlusBtn;
	}
	auto PlusLbl = CreateWidget<CTextBlock>("IPPL_" + std::to_string(I), 4).lock();
	if (PlusLbl)
	{
		PlusLbl->SetPos(142.f, Y);
		PlusLbl->SetSize(18.f, ROW_H - 2.f);
		PlusLbl->SetText(TEXT("+"));
		PlusLbl->SetFontSize(12.f);
		PlusLbl->SetTextColor(FVector4::White);
		PlusLbl->SetAlignH(ETextAlignH::Center);
		PlusLbl->SetAlignV(ETextAlignV::Middle);
		mDynamicRows.push_back(PlusLbl);
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
		if (auto T = mActorNameText.lock()) T->SetText(TEXT("Name: -"));
		if (auto T = mActorTagText.lock())  T->SetText(TEXT("Tag:  -"));
		if (auto T = mRootNameText.lock())  T->SetText(TEXT("Root: -"));
		if (auto T = mPosText.lock())       T->SetText(TEXT("Pos: -"));
		if (auto T = mRotText.lock())       T->SetText(TEXT("Rot: -"));
		if (auto T = mScaleText.lock())     T->SetText(TEXT("Scale: -"));
		FinishLayout(mComponentsStartY);
		return;
	}

	{
		std::wstring WName(Actor->GetName().begin(), Actor->GetName().end());
		std::wstring WTag (Actor->GetActorTag().begin(), Actor->GetActorTag().end());
		TCHAR Buf[128] = {};
		if (auto T = mActorNameText.lock()) { wsprintf(Buf, TEXT("Name: %s"), WName.c_str()); T->SetText(Buf); }
		if (auto T = mActorTagText.lock())  { wsprintf(Buf, TEXT("Tag:  %s"), WTag.c_str());  T->SetText(Buf); }
	}

	if (auto Root = Actor->GetRootComponent().lock())
	{
		std::wstring WRoot(Root->GetName().begin(), Root->GetName().end());
		TCHAR Buf[128] = {};
		if (auto T = mRootNameText.lock()) { wsprintf(Buf, TEXT("Root: %s"), WRoot.c_str()); T->SetText(Buf); }
	}
	else
	{
		if (auto T = mRootNameText.lock()) T->SetText(TEXT("Root: (none)"));
		if (auto T = mPosText.lock())      T->SetText(TEXT("Pos: -"));
		if (auto T = mRotText.lock())      T->SetText(TEXT("Rot: -"));
		if (auto T = mScaleText.lock())    T->SetText(TEXT("Scale: -"));
	}

	// ── 씬 컴포넌트 ───────────────────────────────────────────────────────────
	float Y = mComponentsStartY;
	float PanelW = GetSize().x;

	for (const auto& Comp : Actor->GetSceneCompList())
	{
		if (!Comp) continue;

		bool bExpanded = false;
		auto It = mExpandState.find(Comp.get());
		if (It != mExpandState.end()) bExpanded = It->second;

		int I = mDynIdx++;

		// 헤더 버튼 (클릭 가능한 배경) — 우측 [X] 버튼 자리를 비워둔다.
		auto HBtn = CreateWidget<CButton>("ICH_" + std::to_string(I), 3).lock();
		if (HBtn)
		{
			HBtn->SetPos(2.f, Y);
			HBtn->SetSize(PanelW - 6.f - DEL_BTN_W, ROW_H);
			HBtn->SetTint(EWidgetState::Normal,  0.17f, 0.21f, 0.30f, 1.f);
			HBtn->SetTint(EWidgetState::Hovered, 0.24f, 0.34f, 0.50f, 1.f);
			HBtn->SetTint(EWidgetState::Clicked, 0.30f, 0.42f, 0.62f, 1.f);
			HBtn->SetTint(EWidgetState::Release, 0.24f, 0.34f, 0.50f, 1.f);
			HBtn->SetTint(EWidgetState::Disable, 0.14f, 0.14f, 0.18f, 1.f);
			mDynamicRows.push_back(HBtn);
		}

		// 헤더 레이블
		std::string CompName = Comp->GetName();
		std::string CompType = Comp->GetTypeName();
		std::wstring WName(CompName.begin(), CompName.end());
		std::wstring WType(CompType.begin(), CompType.end());
		TCHAR HBuf[128] = {};
		wsprintf(HBuf, bExpanded ? TEXT("v %s (%s)") : TEXT("> %s (%s)"), WName.c_str(), WType.c_str());

		auto HLbl = CreateWidget<CTextBlock>("ICHL_" + std::to_string(I), 4).lock();
		if (HLbl)
		{
			HLbl->SetPos(8.f, Y);
			HLbl->SetSize(PanelW - 14.f - DEL_BTN_W, ROW_H);
			HLbl->SetText(HBuf);
			HLbl->SetFontSize(12.f);
			HLbl->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
			HLbl->SetAlignH(ETextAlignH::Left);
			HLbl->SetAlignV(ETextAlignV::Middle);
			mDynamicRows.push_back(HLbl);
		}

		auto DelBtn = AddDeleteButton(Y, PanelW, I);

		Y += ROW_H + 2.f;

		FInspCompEntry Entry;
		Entry.CompKey   = Comp.get();
		Entry.CompRef   = std::static_pointer_cast<CComponent>(Comp);
		Entry.HeaderBtn = HBtn;
		Entry.HeaderLbl = HLbl;
		Entry.DeleteBtn = DelBtn;
		Entry.bExpanded = bExpanded;

		if (bExpanded)
		{
			std::weak_ptr<CSceneComponent> CW = Comp;

			// ── 트랜스폼 ───────────────────────────────────────────────
			AddSectionHeader(Y, TEXT("Transform"));

			AddPropRow(Y, TEXT("PosX"), 1.f,
				[CW]() -> float { auto C = CW.lock(); return C ? C->GetRelativePos().x : 0.f; },
				[CW](float V)   { auto C = CW.lock(); if (C) { auto P = C->GetRelativePos(); C->SetRelativePos(V, P.y, P.z); } },
				Entry);

			AddPropRow(Y, TEXT("PosY"), 1.f,
				[CW]() -> float { auto C = CW.lock(); return C ? C->GetRelativePos().y : 0.f; },
				[CW](float V)   { auto C = CW.lock(); if (C) { auto P = C->GetRelativePos(); C->SetRelativePos(P.x, V, P.z); } },
				Entry);

			AddPropRow(Y, TEXT("RotZ"), 1.f,
				[CW]() -> float { auto C = CW.lock(); return C ? C->GetRelativeRot().z : 0.f; },
				[CW](float V)   { auto C = CW.lock(); if (C) { auto R = C->GetRelativeRot(); C->SetRelativeRotation(R.x, R.y, V); } },
				Entry);

			AddPropRow(Y, TEXT("SclX"), 0.1f,
				[CW]() -> float { auto C = CW.lock(); return C ? C->GetRelativeScale().x : 1.f; },
				[CW](float V)   { auto C = CW.lock(); if (C) { auto S = C->GetRelativeScale(); C->SetRelativeScale(V, S.y, S.z); } },
				Entry);

			AddPropRow(Y, TEXT("SclY"), 0.1f,
				[CW]() -> float { auto C = CW.lock(); return C ? C->GetRelativeScale().y : 1.f; },
				[CW](float V)   { auto C = CW.lock(); if (C) { auto S = C->GetRelativeScale(); C->SetRelativeScale(S.x, V, S.z); } },
				Entry);

			// ── CMeshComponent 전용 속성 ──────────────────────────────
			auto MeshComp = std::dynamic_pointer_cast<CMeshComponent>(Comp);
			if (MeshComp)
			{
				std::weak_ptr<CMeshComponent> MCW = MeshComp;

				AddSectionHeader(Y, TEXT("Mesh"));

				// Mesh 지오메트리 드롭다운
				{
					std::vector<std::string> MeshNames;
					auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);
					if (MeshMgr) MeshNames = MeshMgr->GetMeshNames();
					if (MeshNames.empty()) MeshNames = { "TexRect", "ColorRect", "FrameRect", "Sphere" };

					void* DDKey = (void*)((char*)MeshComp.get() + 1); // MeshComp마다 고유 키
					AddDropdownRow(Y, TEXT("Mesh"), DDKey, std::move(MeshNames),
						// GetMeshName()은 "Mesh_TexRect"처럼 매니저 내부 접두사가 붙은 이름을 준다.
						// 목록 항목은 접두사가 빠진 형태라 그대로 쓰면 현재 값이 매칭되지 않는다.
						[MCW]() -> std::string {
							auto MC = MCW.lock();
							if (!MC) return "";
							std::string Name = MC->GetMeshName();
							static const std::string Prefix = "Mesh_";
							if (Name.size() > Prefix.size() && Name.compare(0, Prefix.size(), Prefix) == 0)
								Name = Name.substr(Prefix.size());
							return Name;
						},
						[MCW](const std::string& Name) { auto MC = MCW.lock(); if (MC) MC->SetMesh(Name); },
						Entry);
				}

				// Shader 드롭다운
				{
					static const std::vector<std::string> sShaderNames = { "Material", "Texture", "Animation2D", "Color2D" };
					void* ShaderKey = (void*)((char*)MeshComp.get() + 2);
					AddDropdownRow(Y, TEXT("Shader"), ShaderKey, sShaderNames,
						[MCW]() -> std::string { auto MC = MCW.lock(); return MC ? MC->GetShaderName() : ""; },
						[MCW](const std::string& Name) { auto MC = MCW.lock(); if (MC) MC->SetShader(Name); },
						Entry);
				}

				// Blend 드롭다운
				{
					static const std::vector<std::string> sBlendNames = { "(none)", "AlphaBlend" };
					void* BlendKey = (void*)((char*)MeshComp.get() + 3);
					AddDropdownRow(Y, TEXT("Blend"), BlendKey, sBlendNames,
						[MCW]() -> std::string {
							auto MC = MCW.lock(); if (!MC) return "(none)";
							auto Mat = MC->GetMaterial(0);
							auto RS  = Mat ? Mat->GetBlendState().lock() : nullptr;
							return RS ? RS->GetName() : "(none)";
						},
						[MCW](const std::string& N) {
							auto MC = MCW.lock(); if (!MC) return;
							MC->SetBlendState(0, N == "(none)" ? "" : N);
						},
						Entry);
				}

				AddSectionHeader(Y, TEXT("Material"));

				// Material 선택 드롭다운
				{
					std::vector<std::string> MatNames;
					auto MatMgr = CAssetManager::GetInst()->GetSubManager<CMaterialManager>(EAssetType::Material);
					if (MatMgr) MatNames = MatMgr->GetAllMaterialNames();
					if (MatNames.empty()) MatNames = { "Default" };

					void* MatKey = (void*)((char*)MeshComp.get() + 4);
					AddDropdownRow(Y, TEXT("Mat"), MatKey, std::move(MatNames),
						[MCW]() -> std::string {
							auto MC = MCW.lock(); return MC ? MC->GetMaterialName(0) : "";
						},
						[MCW](const std::string& Name) {
							auto MC = MCW.lock(); if (!MC) return;
							auto MatMgr2 = CAssetManager::GetInst()->GetSubManager<CMaterialManager>(EAssetType::Material);
							if (!MatMgr2) return;
							auto NewMat = MatMgr2->CreateMaterialInstance(Name);
							if (NewMat)
							{
								// 머티리얼만 갈아끼운다. Shader는 건드리지 않는다.
								// 예전에는 머티리얼의 셰이더로 덮어써서, Mat을 고르면
								// Shader가 Animation2D → Material로 되돌아가고
								// 애니메이션이 시트 전체로 풀려버렸다.
								MC->SetMaterialSlot(0, NewMat);
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
						[MCW]() -> float {
							auto MC = MCW.lock(); if (!MC) return 1.f;
							auto Mat = MC->GetMaterial(0); return Mat ? Mat->GetOpacity() : 1.f;
						},
						[MCW](float V) {
							auto MC = MCW.lock(); if (!MC) return;
							float Clamped = V < 0.f ? 0.f : (V > 1.f ? 1.f : V);
							MC->SetOpacity(0, Clamped);
						},
						Entry);

					AddPropRow(Y, TEXT("ColR"), 0.05f,
						[MCW]() -> float {
							auto MC = MCW.lock(); if (!MC) return 1.f;
							auto Mat = MC->GetMaterial(0); return Mat ? Mat->GetBaseColor().x : 1.f;
						},
						[MCW](float V) {
							auto MC = MCW.lock(); if (!MC) return;
							auto Mat = MC->GetMaterial(0); if (!Mat) return;
							float Cl = V < 0.f ? 0.f : (V > 1.f ? 1.f : V);
							const FVector4& C = Mat->GetBaseColor();
							MC->SetBaseColor(0, Cl, C.y, C.z, C.w);
						},
						Entry);

					AddPropRow(Y, TEXT("ColG"), 0.05f,
						[MCW]() -> float {
							auto MC = MCW.lock(); if (!MC) return 1.f;
							auto Mat = MC->GetMaterial(0); return Mat ? Mat->GetBaseColor().y : 1.f;
						},
						[MCW](float V) {
							auto MC = MCW.lock(); if (!MC) return;
							auto Mat = MC->GetMaterial(0); if (!Mat) return;
							float Cl = V < 0.f ? 0.f : (V > 1.f ? 1.f : V);
							const FVector4& C = Mat->GetBaseColor();
							MC->SetBaseColor(0, C.x, Cl, C.z, C.w);
						},
						Entry);

					AddPropRow(Y, TEXT("ColB"), 0.05f,
						[MCW]() -> float {
							auto MC = MCW.lock(); if (!MC) return 1.f;
							auto Mat = MC->GetMaterial(0); return Mat ? Mat->GetBaseColor().z : 1.f;
						},
						[MCW](float V) {
							auto MC = MCW.lock(); if (!MC) return;
							auto Mat = MC->GetMaterial(0); if (!Mat) return;
							float Cl = V < 0.f ? 0.f : (V > 1.f ? 1.f : V);
							const FVector4& C = Mat->GetBaseColor();
							MC->SetBaseColor(0, C.x, C.y, Cl, C.w);
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
		auto It = mExpandState.find(Comp.get());
		if (It != mExpandState.end()) bExpanded = It->second;

		int I = mDynIdx++;

		// 헤더 버튼 (클릭 가능한 배경) — 우측 [X] 버튼 자리를 비워둔다.
		auto HBtn = CreateWidget<CButton>("ICAH_" + std::to_string(I), 3).lock();
		if (HBtn)
		{
			HBtn->SetPos(2.f, Y);
			HBtn->SetSize(PanelW - 6.f - DEL_BTN_W, ROW_H);
			HBtn->SetTint(EWidgetState::Normal,  0.20f, 0.18f, 0.24f, 1.f);
			HBtn->SetTint(EWidgetState::Hovered, 0.30f, 0.26f, 0.38f, 1.f);
			HBtn->SetTint(EWidgetState::Clicked, 0.38f, 0.32f, 0.48f, 1.f);
			HBtn->SetTint(EWidgetState::Release, 0.30f, 0.26f, 0.38f, 1.f);
			HBtn->SetTint(EWidgetState::Disable, 0.14f, 0.14f, 0.18f, 1.f);
			mDynamicRows.push_back(HBtn);
		}

		std::string CompName = Comp->GetName();
		std::string CompType = Comp->GetTypeName();
		std::wstring WName(CompName.begin(), CompName.end());
		std::wstring WType(CompType.begin(), CompType.end());
		TCHAR ABuf[128] = {};
		wsprintf(ABuf, bExpanded ? TEXT("v [A] %s (%s)") : TEXT("> [A] %s (%s)"),
			WName.c_str(), WType.c_str());

		auto HLbl = CreateWidget<CTextBlock>("ICAHL_" + std::to_string(I), 4).lock();
		if (HLbl)
		{
			HLbl->SetPos(8.f, Y);
			HLbl->SetSize(PanelW - 14.f - DEL_BTN_W, ROW_H);
			HLbl->SetText(ABuf);
			HLbl->SetFontSize(12.f);
			HLbl->SetTextColor(FVector4(0.86f, 0.80f, 0.94f, 1.f));
			HLbl->SetAlignH(ETextAlignH::Left);
			HLbl->SetAlignV(ETextAlignV::Middle);
			mDynamicRows.push_back(HLbl);
		}

		auto DelBtn = AddDeleteButton(Y, PanelW, I);

		Y += ROW_H + 2.f;

		FInspCompEntry Entry;
		Entry.CompKey   = Comp.get();
		Entry.CompRef   = std::static_pointer_cast<CComponent>(Comp);
		Entry.HeaderBtn = HBtn;
		Entry.HeaderLbl = HLbl;
		Entry.DeleteBtn = DelBtn;
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
		std::weak_ptr<CColliderBox2D> BW = Box;

		// Halfsize를 들고 있으므로 표시는 전체 크기로 환산한다.
		AddPropRow(Y, TEXT("Size.W"), 1.f,
			[BW]() -> float { auto B = BW.lock(); return B ? B->GetInfo().Halfsize.x * 2.f : 0.f; },
			[BW](float V) {
				auto B = BW.lock(); if (!B) return;
				B->SetBoxSize(V < 1.f ? 1.f : V, B->GetInfo().Halfsize.y * 2.f);
			},
			Entry);

		AddPropRow(Y, TEXT("Size.H"), 1.f,
			[BW]() -> float { auto B = BW.lock(); return B ? B->GetInfo().Halfsize.y * 2.f : 0.f; },
			[BW](float V) {
				auto B = BW.lock(); if (!B) return;
				B->SetBoxSize(B->GetInfo().Halfsize.x * 2.f, V < 1.f ? 1.f : V);
			},
			Entry);
	}
	else if (auto Sphere = std::dynamic_pointer_cast<CColliderSphere2D>(Comp))
	{
		std::weak_ptr<CColliderSphere2D> SW = Sphere;

		AddPropRow(Y, TEXT("Radius"), 1.f,
			[SW]() -> float { auto S = SW.lock(); return S ? S->GetInfo().Radius : 0.f; },
			[SW](float V) { auto S = SW.lock(); if (S) S->SetRadius(V < 1.f ? 1.f : V); },
			Entry);
	}

	// 디버그 외곽선 표시 토글 (두 종류 공통)
	std::weak_ptr<CCollider> CW = Collider;

	AddActionRow(Y, TEXT("Debug"), TEXT("Toggle"),
		[CW]() { auto C = CW.lock(); if (C) C->SetDebugDraw(!C->GetDebugDraw()); },
		[CW]() -> std::string {
			auto C = CW.lock();
			return C ? (C->GetDebugDraw() ? "ON" : "OFF") : "-";
		},
		Entry);
}

// ── 액터 컴포넌트별 속성 ─────────────────────────────────────────────────────
// 지금은 Animation2DComponent만 다룬다. 다른 타입은 헤더만 펼쳐지고 내용이 비어 있다.
void CInspectorUI::AddActorCompProps(float& Y,
                                     const std::shared_ptr<CActorComponent>& Comp,
                                     FInspCompEntry& Entry)
{
	auto AnimComp = std::dynamic_pointer_cast<CAnimation2DComponent>(Comp);
	if (!AnimComp) return;

	std::weak_ptr<CAnimation2DComponent> ACW = AnimComp;

	AddSectionHeader(Y, TEXT("Animation"));

	// 현재 시퀀스 — 드롭다운으로 갈아탈 수 있다.
	{
		std::vector<std::string> SeqNames;
		for (const auto& Pair : AnimComp->GetAnimationMap())
			SeqNames.push_back(Pair.first);

		if (SeqNames.empty()) SeqNames.push_back("(none)");

		void* DDKey = (void*)((char*)AnimComp.get() + 1);
		AddDropdownRow(Y, TEXT("Seq"), DDKey, std::move(SeqNames),
			[ACW]() -> std::string {
				auto AC = ACW.lock();
				if (!AC) return "(none)";
				std::string Cur = AC->GetCurrentAnimationName();
				return Cur.empty() ? "(none)" : Cur;
			},
			[ACW](const std::string& Name) {
				auto AC = ACW.lock();
				if (AC && Name != "(none)") AC->ChangeAnimation(Name);
			},
			Entry);
	}

	// 재생 중인 프레임 — 표시 전용. 클릭해도 아무 일 없다.
	AddActionRow(Y, TEXT("Frame"), TEXT("-"),
		nullptr,
		[ACW]() -> std::string {
			auto AC = ACW.lock();
			if (!AC) return "-";

			auto It = AC->GetAnimationMap().find(AC->GetCurrentAnimationName());
			if (It == AC->GetAnimationMap().end()) return "-";

			return std::to_string(AC->GetAnimationFrame() + 1) + " / "
			     + std::to_string(It->second->GetFrameCount());
		},
		Entry);

	// 현재 시퀀스를 집어오는 헬퍼. 시퀀스가 없으면 nullptr.
	auto GetSeq = [](const std::weak_ptr<CAnimation2DComponent>& Weak)
		-> std::shared_ptr<CAnimation2DSequence>
	{
		auto AC = Weak.lock();
		if (!AC) return nullptr;

		auto It = AC->GetAnimationMap().find(AC->GetCurrentAnimationName());
		return (It != AC->GetAnimationMap().end()) ? It->second : nullptr;
	};

	AddPropRow(Y, TEXT("PlayTime"), 0.05f,
		[ACW, GetSeq]() -> float { auto S = GetSeq(ACW); return S ? S->GetPlayTime() : 1.f; },
		[ACW, GetSeq](float V) {
			auto AC = ACW.lock(); if (!AC) return;
			AC->SetPlayTime(AC->GetCurrentAnimationName(), V < 0.05f ? 0.05f : V);
		},
		Entry);

	AddPropRow(Y, TEXT("PlayRate"), 0.1f,
		[ACW, GetSeq]() -> float { auto S = GetSeq(ACW); return S ? S->GetPlayRate() : 1.f; },
		[ACW, GetSeq](float V) {
			auto AC = ACW.lock(); if (!AC) return;
			AC->SetPlayRate(AC->GetCurrentAnimationName(), V < 0.05f ? 0.05f : V);
		},
		Entry);

	// ON/OFF 세 개 — 누르면 뒤집힌다.
	auto AddFlagRow = [&](const wchar_t* Label,
	                      bool (CAnimation2DSequence::*Get)() const,
	                      void (CAnimation2DComponent::*Set)(const std::string&, bool))
	{
		AddActionRow(Y, Label, TEXT("Toggle"),
			[ACW, GetSeq, Get, Set]() {
				auto AC = ACW.lock(); if (!AC) return;
				auto S = GetSeq(ACW);  if (!S)  return;
				(AC.get()->*Set)(AC->GetCurrentAnimationName(), !(S.get()->*Get)());
			},
			[ACW, GetSeq, Get]() -> std::string {
				auto S = GetSeq(ACW);
				return S ? ((S.get()->*Get)() ? "ON" : "OFF") : "-";
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
		auto DelBtn = Entry.DeleteBtn.lock();
		if (!DelBtn) continue;
		if (DelBtn->GetWidgetState() != EWidgetState::Release) continue;

		auto Actor = mTarget.lock();
		auto Comp  = Entry.CompRef.lock();
		if (!Actor || !Comp) return false;

		std::string RemovedName = Comp->GetName();

		if (!Actor->RemoveComponent(Comp))
			return false;

		// 이 컴포넌트에 딸린 UI 상태를 정리한다.
		mExpandState.erase(Entry.CompKey);
		for (auto& DD : Entry.Dropdowns)
			mDropdownOpen.erase(DD.DropKey);

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

	for (int d = 0; d < 10; ++d)
	{
		Input->AddBindKey("InspNum" + std::to_string(d), (unsigned char)('0' + d));
		Input->AddBindKey("InspPad" + std::to_string(d), (unsigned char)(VK_NUMPAD0 + d));
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
	char Buf[32] = {};
	sprintf_s(Buf, 32, "%.2f", Prop.Getter ? Prop.Getter() : 0.f);
	mEditBuffer = Buf;

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
		std::shared_ptr<CButton> EditBtn;

		if (mEditCompIdx >= 0 && mEditCompIdx < (int)mCompEntries.size())
		{
			auto& Entry = mCompEntries[mEditCompIdx];
			if (mEditPropIdx >= 0 && mEditPropIdx < (int)Entry.Props.size())
				EditBtn = Entry.Props[mEditPropIdx].ValueBtn.lock();
		}

		EWidgetState::Type S = EditBtn ? EditBtn->GetWidgetState() : EWidgetState::Normal;

		if (S != EWidgetState::Hovered && S != EWidgetState::Clicked && S != EWidgetState::Release)
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
	for (int ci = 0; ci < (int)mCompEntries.size(); ++ci)
	{
		auto& Entry = mCompEntries[ci];
		if (!Entry.bExpanded) continue;

		for (int pi = 0; pi < (int)Entry.Props.size(); ++pi)
		{
			auto Btn = Entry.Props[pi].ValueBtn.lock();
			if (!Btn) continue;
			if (Btn->GetWidgetState() != EWidgetState::Release) continue;

			void* ClickKey = Btn.get();

			// 같은 버튼을 짧은 간격으로 두 번 → 편집 시작
			if (mLastClickKey == ClickKey && (mTimeAccum - mLastClickTime) <= DOUBLE_CLICK_SEC)
			{
				BeginEdit(ci, pi);
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
		float W = PanelSize.x, H = PanelSize.y;

		float CX[4] = { PanelPos.x, PanelPos.x + W - HANDLE_SZ, PanelPos.x, PanelPos.x + W - HANDLE_SZ };
		float CY[4] = { PanelPos.y, PanelPos.y, PanelPos.y + H - HANDLE_SZ, PanelPos.y + H - HANDLE_SZ };

		if (mActiveCorner == -1 && Press)
		{
			for (int i = 0; i < 4; ++i)
			{
				if (MousePos.x >= CX[i] && MousePos.x < CX[i] + HANDLE_SZ &&
					MousePos.y >= CY[i] && MousePos.y < CY[i] + HANDLE_SZ)
				{
					mActiveCorner = i; break;
				}
			}
		}
		if (mActiveCorner >= 0 && Release) mActiveCorner = -1;

		if (mActiveCorner >= 0 && Held && (Delta.x != 0.f || Delta.y != 0.f))
		{
			float nx = PanelPos.x, ny = PanelPos.y, nw = W, nh = H;
			switch (mActiveCorner)
			{
			case 0: nx += Delta.x; ny += Delta.y; nw -= Delta.x; nh -= Delta.y; break;
			case 1:                ny += Delta.y; nw += Delta.x; nh -= Delta.y; break;
			case 2: nx += Delta.x;                nw -= Delta.x; nh += Delta.y; break;
			case 3:                                nw += Delta.x; nh += Delta.y; break;
			}
			nw = max(nw, 120.f); nh = max(nh, 80.f);
			SetPos(nx, ny); SetSize(nw, nh);
			if (auto Bg = mBackground.lock())    Bg->SetSize(nw, nh);
			if (auto TB = mTitleBarWidget.lock()) TB->SetSize(nw, TITLE_H);
			if (auto H2 = mHandleTL.lock()) H2->SetPos(0.f,           0.f);
			if (auto H2 = mHandleTR.lock()) H2->SetPos(nw - HANDLE_SZ, 0.f);
			if (auto H2 = mHandleBL.lock()) H2->SetPos(0.f,           nh - HANDLE_SZ);
			if (auto H2 = mHandleBR.lock()) H2->SetPos(nw - HANDLE_SZ, nh - HANDLE_SZ);
			UpdateAllRowWidths(nw);
			// 리사이즈는 Rebuild를 거치지 않으므로 스크롤 영역을 직접 갱신한다.
			SetScrollArea(mComponentsStartY - 2.f, nh);
		}
	}

	// ── 루트 트랜스폼 실시간 표시 ────────────────────────────────────────────
	auto Actor = mTarget.lock();
	if (Actor)
	{
		if (auto Root = Actor->GetRootComponent().lock())
		{
			TCHAR Buf[128] = {};
			auto Pos   = Root->GetWorldPos();
			auto Rot   = Root->GetWorldRot();
			auto Scale = Root->GetWorldScale();
			if (auto T = mPosText.lock())   { swprintf_s(Buf, 128, L"Pos: %.1f %.1f", Pos.x, Pos.y);    T->SetText(Buf); }
			if (auto T = mRotText.lock())   { swprintf_s(Buf, 128, L"Rot: %.1f",       Rot.z);            T->SetText(Buf); }
			if (auto T = mScaleText.lock()) { swprintf_s(Buf, 128, L"Scl: %.2f %.2f",  Scale.x, Scale.y); T->SetText(Buf); }
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
			if (auto Btn = Action.ActionBtn.lock())
			{
				if (Btn->GetWidgetState() == EWidgetState::Release)
				{
					if (Action.OnClick) Action.OnClick();
					// 표시 레이블 업데이트
					if (Action.GetDisplay)
					{
						if (auto Disp = Action.DisplayLbl.lock())
						{
							std::string S = Action.GetDisplay();
							std::wstring W(S.begin(), S.end());
							Disp->SetText(W.c_str());
						}
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
			for (auto& DD : Entry.Dropdowns)
			{
				// 열기/닫기 토글
				if (auto Tog = DD.ToggleBtn.lock())
				{
					if (Tog->GetWidgetState() == EWidgetState::Release)
					{
						bool bCur = mDropdownOpen.count(DD.DropKey) && mDropdownOpen[DD.DropKey];
						mDropdownOpen[DD.DropKey] = !bCur;
						bDDRebuild = true;
						break;
					}
				}

				// 항목 선택
				for (int k = 0; k < (int)DD.ItemBtns.size(); ++k)
				{
					if (auto ItemBtn = DD.ItemBtns[k].lock())
					{
						if (ItemBtn->GetWidgetState() == EWidgetState::Release)
						{
							if (DD.Setter) DD.Setter(DD.Items[k]);
							// 값 레이블 즉시 업데이트
							if (auto VL = DD.ValueLbl.lock())
							{
								std::wstring W(DD.Items[k].begin(), DD.Items[k].end());
								VL->SetText(W.c_str());
							}
							mDropdownOpen[DD.DropKey] = false;
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
		if (auto Btn = Entry.HeaderBtn.lock())
		{
			if (Btn->GetWidgetState() == EWidgetState::Release)
			{
				mExpandState[Entry.CompKey] = !Entry.bExpanded;
				// 헤더 토글 시 해당 컴포넌트의 드롭다운은 모두 닫음
				for (auto& DD : Entry.Dropdowns)
					mDropdownOpen[DD.DropKey] = false;
				bNeedRebuild = true;
				break;
			}
		}
	}
	if (bNeedRebuild) { Rebuild(); return; }

	// ── 속성 +/- 버튼 인터랙션 ──────────────────────────────────────────────
	for (int ci = 0; ci < (int)mCompEntries.size(); ++ci)
	{
		auto& Entry = mCompEntries[ci];
		if (!Entry.bExpanded) continue;

		for (int pi = 0; pi < (int)Entry.Props.size(); ++pi)
		{
			auto& Prop = Entry.Props[pi];

			// 편집 중인 행은 +/- 와 실시간 표시를 건너뛰고 입력 버퍼를 보여준다.
			if (mEditActive && ci == mEditCompIdx && pi == mEditPropIdx)
			{
				if (auto V = Prop.ValueLabel.lock())
				{
					std::wstring WBuf(mEditBuffer.begin(), mEditBuffer.end());
					WBuf += L"_";   // 캐럿
					V->SetText(WBuf.c_str());
					V->SetTextColor(FVector4(0.55f, 1.f, 0.65f, 1.f));
				}
				continue;
			}

			if (!Prop.Getter || !Prop.Setter) continue;

			// 감소
			if (auto MinBtn = Prop.MinusBtn.lock())
			{
				EWidgetState::Type S = MinBtn->GetWidgetState();
				if (S == EWidgetState::Clicked)
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
					if (S == EWidgetState::Release && Prop.MinHoldTime < 0.35f)
						Prop.Setter(Prop.Getter() - Prop.Step);
					Prop.MinHoldTime = 0.f;
					Prop.MinRepeat   = 0;
				}
			}

			// 증가
			if (auto PlusBtn = Prop.PlusBtn.lock())
			{
				EWidgetState::Type S = PlusBtn->GetWidgetState();
				if (S == EWidgetState::Clicked)
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
					if (S == EWidgetState::Release && Prop.PlusHoldTime < 0.35f)
						Prop.Setter(Prop.Getter() + Prop.Step);
					Prop.PlusHoldTime = 0.f;
					Prop.PlusRepeat   = 0;
				}
			}

			// 실시간 값 표시 업데이트
			if (auto V = Prop.ValueLabel.lock())
			{
				TCHAR Buf[32] = {};
				swprintf_s(Buf, 32, L"%.2f", Prop.Getter());
				V->SetText(Buf);
				V->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
			}
		}
	}
}

// ── 모든 행 너비 갱신 ────────────────────────────────────────────────────────
void CInspectorUI::UpdateAllRowWidths(float nw)
{
	auto SetW = [&](std::weak_ptr<CTextBlock> Weak)
	{
		if (auto T = Weak.lock()) T->SetSize(nw - 8.f, ROW_H);
	};
	if (auto T = mTitleText.lock()) T->SetSize(nw, TITLE_H);
	SetW(mActorNameText);
	SetW(mActorTagText);
	SetW(mTransformHeader);
	SetW(mRootNameText);
	SetW(mPosText);
	SetW(mRotText);
	SetW(mScaleText);
	SetW(mCompHeader);
	Rebuild();
}

CInspectorUI* CInspectorUI::Clone()
{
	return new CInspectorUI(*this);
}
