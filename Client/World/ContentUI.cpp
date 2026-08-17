#include "ContentUI.h"

#include "World/Button.h"
#include "World/TextBlock.h"
#include "World/TitleBar.h"
#include "World/World.h"
#include "World/Actor.h"
#include "World/Input.h"

CContentUI::CContentUI()
{}

CContentUI::CContentUI(const CContentUI& src)
	: CWidgetContainer(src)
{}

CContentUI::~CContentUI()
{}

bool CContentUI::Init()
{
	CWidgetContainer::Init();

	SetPos(PANEL_X, PANEL_Y);
	SetSize(PANEL_W, PANEL_H);

	// 배경
	auto Bg = CreateWidget<CButton>("ContentBg", 0).lock();
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
	auto TitleBar = CreateWidget<CTitleBar>("ContentTitleBar", 1).lock();
	if (TitleBar)
	{
		TitleBar->SetPos(0.f, 0.f);
		TitleBar->SetSize(PANEL_W, TITLE_H);
		TitleBar->SetTint(0.22f, 0.22f, 0.28f, 1.f);
		TitleBar->SetUpdateWidget(GetThisPtr<CWidget>());
		mTitleBarWidget = TitleBar;
	}

	auto TitleText = CreateWidget<CTextBlock>("ContentTitle", 2).lock();
	if (TitleText)
	{
		TitleText->SetPos(0.f, 0.f);
		TitleText->SetSize(PANEL_W, TITLE_H);
		TitleText->SetText(TEXT("Content"));
		TitleText->SetFontSize(15.f);
		TitleText->SetTextColor(FVector4::White);
		TitleText->SetAlignH(ETextAlignH::Center);
		TitleText->SetAlignV(ETextAlignV::Middle);
		mTitleText = TitleText;
	}

	// 코너 리사이즈 핸들 (ZOrder 10, 시각 전용 — 히트 테스트는 수동으로 처리)
	auto MakeHandle = [&](const std::string& Name, float X, float Y) -> std::weak_ptr<CButton>
	{
		auto H = CreateWidget<CButton>(Name, 10).lock();
		if (H)
		{
			H->SetPos(X, Y);
			H->SetSize(HANDLE_SZ, HANDLE_SZ);
			H->SetTint(EWidgetState::Normal,  0.45f, 0.45f, 0.55f, 0.85f);
			H->SetTint(EWidgetState::Hovered, 0.70f, 0.90f, 1.00f, 1.f);
			H->SetTint(EWidgetState::Clicked, 1.00f, 1.00f, 1.00f, 1.f);
			H->SetTint(EWidgetState::Release, 1.00f, 1.00f, 1.00f, 1.f);
			H->SetTint(EWidgetState::Disable, 0.30f, 0.30f, 0.30f, 0.5f);
		}
		return H;
	};

	mHandleTL = MakeHandle("HandleTL", 0.f,              0.f);
	mHandleTR = MakeHandle("HandleTR", PANEL_W - HANDLE_SZ, 0.f);
	mHandleBL = MakeHandle("HandleBL", 0.f,              PANEL_H - HANDLE_SZ);
	mHandleBR = MakeHandle("HandleBR", PANEL_W - HANDLE_SZ, PANEL_H - HANDLE_SZ);

	mStaticChildCount = (int)mChildList.size();
	mStaticWidgets = mChildList;

	// 액터 목록이 패널보다 길어지면 휠로 스크롤할 수 있게 한다.
	EnableScroll(true);
	SetScrollArea(TITLE_H, PANEL_H);
	SetScrollStep(ENTRY_H);

	return true;
}

void CContentUI::Update(float DeltaTime)
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

		// 코너 화면 좌표 (좌상 우상 좌하 우하)
		float CX[4] = {
			PanelPos.x,
			PanelPos.x + W - HANDLE_SZ,
			PanelPos.x,
			PanelPos.x + W - HANDLE_SZ
		};
		float CY[4] = {
			PanelPos.y,
			PanelPos.y,
			PanelPos.y + H - HANDLE_SZ,
			PanelPos.y + H - HANDLE_SZ
		};

		if (mActiveCorner == -1 && Press)
		{
			for (int i = 0; i < 4; ++i)
			{
				if (MousePos.x >= CX[i] && MousePos.x < CX[i] + HANDLE_SZ &&
					MousePos.y >= CY[i] && MousePos.y < CY[i] + HANDLE_SZ)
				{
					mActiveCorner = i;
					break;
				}
			}
		}

		if (mActiveCorner >= 0 && Release)
			mActiveCorner = -1;

		if (mActiveCorner >= 0 && Held && (Delta.x != 0.f || Delta.y != 0.f))
		{
			float nx = PanelPos.x, ny = PanelPos.y, nw = W, nh = H;

			switch (mActiveCorner)
			{
			case 0: nx += Delta.x; ny += Delta.y; nw -= Delta.x; nh -= Delta.y; break; // 좌상
			case 1:                ny += Delta.y; nw += Delta.x; nh -= Delta.y; break; // 우상
			case 2: nx += Delta.x;                nw -= Delta.x; nh += Delta.y; break; // 좌하
			case 3:                                nw += Delta.x; nh += Delta.y; break; // 우하
			}

			nw = max(nw, 120.f);
			nh = max(nh, 80.f);

			SetPos(nx, ny);
			SetSize(nw, nh);

			if (auto Bg = mBackground.lock())    Bg->SetSize(nw, nh);
			if (auto TB = mTitleBarWidget.lock()) TB->SetSize(nw, TITLE_H);
			if (auto TT = mTitleText.lock())      TT->SetSize(nw, TITLE_H);

			if (auto H = mHandleTL.lock()) H->SetPos(0.f,            0.f);
			if (auto H = mHandleTR.lock()) H->SetPos(nw - HANDLE_SZ, 0.f);
			if (auto H = mHandleBL.lock()) H->SetPos(0.f,            nh - HANDLE_SZ);
			if (auto H = mHandleBR.lock()) H->SetPos(nw - HANDLE_SZ, nh - HANDLE_SZ);

			mLastActorCount = -1; // 새 너비로 강제 재구성
		}
	}
	// ─────────────────────────────────────────────────────────────────────────

	// 월드 로드 시 기존 액터 포인터가 만료되면 강제 재구성
	if (mSelectedActor.expired())
		mSelectedActor.reset();

	bool bStale = false;
	for (auto& E : mEntries)
	{
		if (E.Actor.expired()) { bStale = true; break; }
	}

	int CurrentCount = (int)World->GetActorList().size();
	if (CurrentCount != mLastActorCount || bStale)
	{
		mLastActorCount = CurrentCount;
		Rebuild();
		return;
	}

	// 버튼 Release 상태를 폴링하여 클릭 감지
	for (int i = 0; i < (int)mEntries.size(); ++i)
	{
		auto Btn = mEntries[i].Button.lock();
		if (Btn && Btn->GetWidgetState() == EWidgetState::Release)
		{
			OnEntryClicked(i);
		}
	}
}

void CContentUI::Rebuild()
{
	// mChildList는 렌더/충돌에서 정렬되므로 개수로 자르면 정적 위젯이 날아갈 수 있다.
	// Init 시점 스냅샷으로 되돌린 뒤 다시 채운다.
	if (!mStaticWidgets.empty())
		mChildList = mStaticWidgets;
	else if ((int)mChildList.size() > mStaticChildCount)
		mChildList.resize(mStaticChildCount);
	mEntries.clear();

	auto World = mWorld.lock();
	if (!World) { FinishLayout(TITLE_H + 2.f); return; }

	const auto& ActorMap = World->GetActorList();

	float CurW  = GetSize().x;
	float Y     = TITLE_H + 2.f;
	int   Index = 0;

	for (auto& [Name, ActorPtr] : ActorMap)
	{
		std::string BtnName = "ContentEntry_" + std::to_string(Index);
		std::string LblName = "ContentLbl_"   + std::to_string(Index);

		auto Btn = CreateWidget<CButton>(BtnName, 3).lock();
		if (Btn)
		{
			Btn->SetPos(4.f, Y);
			Btn->SetSize(CurW - 8.f, ENTRY_H - 2.f);
			Btn->SetTint(EWidgetState::Normal,  0.20f, 0.20f, 0.24f, 1.f);
			Btn->SetTint(EWidgetState::Hovered, 0.30f, 0.50f, 0.90f, 1.f);
			Btn->SetTint(EWidgetState::Clicked, 0.40f, 0.65f, 1.00f, 1.f);
			Btn->SetTint(EWidgetState::Release, 0.40f, 0.65f, 1.00f, 1.f);
			Btn->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.18f, 1.f);
		}

		std::wstring WName(Name.begin(), Name.end());
		auto Lbl = CreateWidget<CTextBlock>(LblName, 4).lock();
		if (Lbl)
		{
			Lbl->SetPos(8.f, Y);
			Lbl->SetSize(CurW - 12.f, ENTRY_H - 2.f);
			Lbl->SetText(WName.c_str());
			Lbl->SetFontSize(13.f);
			Lbl->SetTextColor(FVector4::White);
			Lbl->SetAlignH(ETextAlignH::Left);
			Lbl->SetAlignV(ETextAlignV::Middle);
		}

		FEntry Entry;
		Entry.Actor  = ActorPtr;
		Entry.Button = Btn;
		Entry.Index  = Index;
		mEntries.push_back(Entry);

		Y += ENTRY_H;
		++Index;
	}

	FinishLayout(Y);

	RefreshSelectionTints();
}

// Rebuild 직후에만 호출된다. (아직 정렬되지 않아 인덱스를 믿을 수 있다)
void CContentUI::FinishLayout(float ContentEndY)
{
	size_t StaticCount = mStaticWidgets.empty()
		? (size_t)mStaticChildCount
		: mStaticWidgets.size();

	for (size_t i = StaticCount; i < mChildList.size(); ++i)
		mChildList[i]->SetScrollTarget(true);

	SetScrollArea(TITLE_H, GetSize().y);
	SetScrollContentEnd(ContentEndY + 4.f);
}

void CContentUI::OnEntryClicked(int Index)
{
	if (Index < 0 || Index >= (int)mEntries.size()) return;
	mSelectedActor = mEntries[Index].Actor;
	RefreshSelectionTints();
	if (mOnActorSelected)
		mOnActorSelected(mEntries[Index].Actor);
}

void CContentUI::RefreshSelectionTints()
{
	auto SelActor = mSelectedActor.lock();
	for (auto& Entry : mEntries)
	{
		auto Btn = Entry.Button.lock();
		if (!Btn) continue;
		bool bSelected = SelActor && (Entry.Actor.lock() == SelActor);
		if (bSelected)
		{
			Btn->SetTint(EWidgetState::Normal, 0.15f, 0.45f, 0.90f, 1.f);
		}
		else
		{
			Btn->SetTint(EWidgetState::Normal, 0.20f, 0.20f, 0.24f, 1.f);
		}
	}
}

CContentUI* CContentUI::Clone()
{
	return new CContentUI(*this);
}
