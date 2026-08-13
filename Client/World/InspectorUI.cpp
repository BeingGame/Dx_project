#include "InspectorUI.h"

#include "World/Button.h"
#include "World/TextBlock.h"
#include "World/TitleBar.h"
#include "World/Actor.h"
#include "World/SceneComponent.h"
#include "World/ActorComponent.h"
#include "World/MeshComponent.h"
#include "Asset/Material.h"
#include "Asset/AssetManager.h"
#include "Asset/MeshManager.h"
#include "Asset/MaterialManager.h"
#include "RenderState.h"
#include "World/Input.h"
#include "World/World.h"
#include "DialogUtil.h"

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

		// 헤더 버튼 (클릭 가능한 배경)
		auto HBtn = CreateWidget<CButton>("ICH_" + std::to_string(I), 3).lock();
		if (HBtn)
		{
			HBtn->SetPos(2.f, Y);
			HBtn->SetSize(PanelW - 4.f, ROW_H);
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
			HLbl->SetSize(PanelW - 12.f, ROW_H);
			HLbl->SetText(HBuf);
			HLbl->SetFontSize(12.f);
			HLbl->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
			HLbl->SetAlignH(ETextAlignH::Left);
			HLbl->SetAlignV(ETextAlignV::Middle);
			mDynamicRows.push_back(HLbl);
		}
		Y += ROW_H + 2.f;

		FInspCompEntry Entry;
		Entry.CompKey   = Comp.get();
		Entry.HeaderBtn = HBtn;
		Entry.HeaderLbl = HLbl;
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
						[MCW]() -> std::string { auto MC = MCW.lock(); return MC ? MC->GetMeshName() : ""; },
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
								MC->SetMaterialSlot(0, NewMat);
								const std::string& ShaderName = NewMat->GetShaderName();
								if (!ShaderName.empty())
									MC->SetShader(ShaderName);
							}
						},
						Entry);
				}

				// 머티리얼 속성 편집 (슬롯 0이 존재할 때만)
				if (MeshComp->GetMaterial(0))
				{
					AddSectionHeader(Y, TEXT("Mat Props"));

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

					AddPropRow(Y, TEXT("ColA"), 0.05f,
						[MCW]() -> float {
							auto MC = MCW.lock(); if (!MC) return 1.f;
							auto Mat = MC->GetMaterial(0); return Mat ? Mat->GetBaseColor().w : 1.f;
						},
						[MCW](float V) {
							auto MC = MCW.lock(); if (!MC) return;
							auto Mat = MC->GetMaterial(0); if (!Mat) return;
							float Cl = V < 0.f ? 0.f : (V > 1.f ? 1.f : V);
							const FVector4& C = Mat->GetBaseColor();
							MC->SetBaseColor(0, C.x, C.y, C.z, Cl);
						},
						Entry);
				}
			}
		}

		mCompEntries.push_back(std::move(Entry));
	}

	// ── 액터 컴포넌트 (이름만 표시, 확장 불가) ───────────────────────────────
	for (const auto& Comp : Actor->GetActorCompList())
	{
		if (!Comp) continue;

		int I = mDynIdx++;
		std::string CompName = Comp->GetName();
		std::string CompType = Comp->GetTypeName();
		std::wstring WName(CompName.begin(), CompName.end());
		std::wstring WType(CompType.begin(), CompType.end());
		TCHAR ABuf[128] = {};
		wsprintf(ABuf, TEXT("[A] %s (%s)"), WName.c_str(), WType.c_str());

		if (auto Row = CreateWidget<CTextBlock>("ICA_" + std::to_string(I), 3).lock())
		{
			Row->SetPos(8.f, Y);
			Row->SetSize(PanelW - 12.f, ROW_H);
			Row->SetText(ABuf);
			Row->SetFontSize(12.f);
			Row->SetTextColor(FVector4(0.72f, 0.72f, 0.72f, 1.f));
			Row->SetAlignH(ETextAlignH::Left);
			Row->SetAlignV(ETextAlignV::Middle);
			mDynamicRows.push_back(Row);
		}
		Y += ROW_H + 2.f;
	}
}

// ── 업데이트 ─────────────────────────────────────────────────────────────────
void CInspectorUI::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);

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
	for (auto& Entry : mCompEntries)
	{
		if (!Entry.bExpanded) continue;
		for (auto& Prop : Entry.Props)
		{
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
