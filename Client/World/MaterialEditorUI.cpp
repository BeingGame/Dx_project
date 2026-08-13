#include "MaterialEditorUI.h"

#include "World/Button.h"
#include "World/TextBlock.h"
#include "World/TitleBar.h"
#include "World/Actor.h"
#include "World/SceneComponent.h"
#include "World/MeshComponent.h"
#include "Asset/Material.h"
#include "Asset/AssetManager.h"
#include "Asset/MaterialManager.h"
#include "World/Input.h"
#include "World/World.h"
#include "DialogUtil.h"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <windows.h>

const std::vector<std::string> CMaterialEditorUI::sShaderNames  = { "Material", "Animation2D", "Color2D", "Texture" };
const std::vector<std::string> CMaterialEditorUI::sSamplerNames = { "Linear", "Point" };
const std::vector<std::string> CMaterialEditorUI::sBlendNames   = { "(none)", "AlphaBlend" };

CMaterialEditorUI::CMaterialEditorUI()
{}

CMaterialEditorUI::CMaterialEditorUI(const CMaterialEditorUI& src)
	: CWidgetContainer(src)
{}

CMaterialEditorUI::~CMaterialEditorUI()
{}

// ── 헬퍼: 드롭다운 콤보 행 ───────────────────────────────────────────────────
void CMaterialEditorUI::MakeDropdown(float Y, const wchar_t* Label,
                                     FMatDropdown& DD,
                                     const std::vector<std::string>& Items,
                                     int InitIdx, const std::string& Pfx)
{
	DD.Items        = Items;
	DD.SelectedIdx  = InitIdx;
	DD.bOpen        = false;
	DD.ItemListY    = Y + ROW_H + 2.f;  // 항목 목록은 헤더 행 바로 아래에 표시

	// 레이블
	MakeLabel(Pfx + "_L", 6.f, Y, 52.f, ROW_H - 2.f, Label, 11.f);

	// 현재 선택값 표시
	std::wstring WVal(Items[InitIdx].begin(), Items[InitIdx].end());
	auto ValLbl = CreateWidget<CTextBlock>(Pfx + "_V", 3).lock();
	if (ValLbl)
	{
		ValLbl->SetPos(60.f, Y);
		ValLbl->SetSize(106.f, ROW_H - 2.f);
		ValLbl->SetText(WVal.c_str());
		ValLbl->SetFontSize(11.f);
		ValLbl->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		ValLbl->SetAlignH(ETextAlignH::Left);
		ValLbl->SetAlignV(ETextAlignV::Middle);
		DD.ValueLbl = ValLbl;
	}

	// 토글 버튼 [▼]
	auto TBtn = MakeButton(Pfx + "_TB", 168.f, Y, 44.f, ROW_H - 2.f, 0.20f, 0.30f, 0.46f);
	auto TLbl = MakeLabel(Pfx + "_TL", 168.f, Y, 44.f, ROW_H - 2.f, TEXT("▼"), 11.f, 4);
	DD.ToggleBtn = TBtn;
	DD.ToggleLbl = TLbl;

	// 목록 배경 (높은 Z순서, 초기에는 화면 밖에 숨김)
	float ListH = (float)Items.size() * (ITEM_H + 1.f);
	auto ListBg = CreateWidget<CButton>(Pfx + "_BG", 9).lock();
	if (ListBg)
	{
		ListBg->SetPos(-2000.f, DD.ItemListY);
		ListBg->SetSize(PANEL_W - 12.f, ListH);
		ListBg->SetTint(EWidgetState::Normal,  0.08f, 0.08f, 0.10f, 0.97f);
		ListBg->SetTint(EWidgetState::Hovered, 0.08f, 0.08f, 0.10f, 0.97f);
		ListBg->SetTint(EWidgetState::Clicked, 0.08f, 0.08f, 0.10f, 0.97f);
		ListBg->SetTint(EWidgetState::Release, 0.08f, 0.08f, 0.10f, 0.97f);
		ListBg->SetTint(EWidgetState::Disable, 0.08f, 0.08f, 0.10f, 0.97f);
		DD.ListBg = ListBg;
	}

	// 항목 버튼 (화면 밖에 숨김, Z=10/11)
	float IY = DD.ItemListY;
	for (int i = 0; i < (int)Items.size(); ++i)
	{
		bool bSel = (i == InitIdx);
		auto IBtn = CreateWidget<CButton>(Pfx + "_IB" + std::to_string(i), 10).lock();
		if (IBtn)
		{
			IBtn->SetPos(-2000.f, IY);
			IBtn->SetSize(PANEL_W - 12.f, ITEM_H);
			IBtn->SetTint(EWidgetState::Normal,  bSel ? 0.14f : 0.08f, bSel ? 0.32f : 0.12f, bSel ? 0.22f : 0.16f, 1.f);
			IBtn->SetTint(EWidgetState::Hovered, 0.22f, 0.38f, 0.48f, 1.f);
			IBtn->SetTint(EWidgetState::Clicked, 0.30f, 0.50f, 0.62f, 1.f);
			IBtn->SetTint(EWidgetState::Release, 0.22f, 0.38f, 0.48f, 1.f);
			IBtn->SetTint(EWidgetState::Disable, 0.08f, 0.10f, 0.12f, 1.f);
			DD.ItemBtns.push_back(IBtn);
		}

		auto ILbl = CreateWidget<CTextBlock>(Pfx + "_IL" + std::to_string(i), 11).lock();
		if (ILbl)
		{
			ILbl->SetPos(-2000.f, IY);
			ILbl->SetSize(PANEL_W - 18.f, ITEM_H);
			std::wstring WItem(Items[i].begin(), Items[i].end());
			ILbl->SetText(WItem.c_str());
			ILbl->SetFontSize(11.f);
			ILbl->SetTextColor(bSel ? FVector4(0.5f, 1.f, 0.65f, 1.f) : FVector4(0.85f, 0.85f, 0.85f, 1.f));
			ILbl->SetAlignH(ETextAlignH::Left);
			ILbl->SetAlignV(ETextAlignV::Middle);
			DD.ItemLbls.push_back(ILbl);
		}
		IY += ITEM_H + 1.f;
	}
}

void CMaterialEditorUI::OpenDropdown(FMatDropdown& DD)
{
	DD.bOpen = true;
	if (auto L = DD.ToggleLbl.lock()) L->SetText(TEXT("▲"));
	if (auto B = DD.ListBg.lock()) B->SetPos(6.f, DD.ItemListY);
	float IY = DD.ItemListY;
	for (int i = 0; i < (int)DD.ItemBtns.size(); ++i)
	{
		if (auto B = DD.ItemBtns[i].lock()) B->SetPos(6.f, IY);
		if (auto L = DD.ItemLbls[i].lock()) L->SetPos(12.f, IY);
		IY += ITEM_H + 1.f;
	}
}

void CMaterialEditorUI::CloseDropdown(FMatDropdown& DD)
{
	DD.bOpen = false;
	if (auto L = DD.ToggleLbl.lock()) L->SetText(TEXT("▼"));
	if (auto B = DD.ListBg.lock()) B->SetPos(-2000.f, DD.ItemListY);
	for (int i = 0; i < (int)DD.ItemBtns.size(); ++i)
	{
		if (auto B = DD.ItemBtns[i].lock()) B->SetPos(-2000.f, DD.ItemListY);
		if (auto L = DD.ItemLbls[i].lock()) L->SetPos(-2000.f, DD.ItemListY);
	}
}

void CMaterialEditorUI::CloseAllDropdowns()
{
	CloseDropdown(mShaderDD);
	CloseDropdown(mSamplerDD);
	CloseDropdown(mBlendDD);
}

// ── 헬퍼: 수치 +/- 행 ────────────────────────────────────────────────────
void CMaterialEditorUI::MakeNumRow(float Y, const wchar_t* Label, FColorRow& Row, float InitVal, float Step)
{
	Row.Value = InitVal;
	Row.Step  = Step;

	MakeLabel("MENL_" + std::to_string((int)Y), 6.f, Y, 38.f, ROW_H - 2.f, Label, 11.f);

	Row.MinusBtn = MakeButton("MENM_" + std::to_string((int)Y), 46.f, Y, 18.f, ROW_H - 2.f, 0.26f, 0.26f, 0.32f);
	MakeLabel("MENML_" + std::to_string((int)Y), 46.f, Y, 18.f, ROW_H - 2.f, TEXT("-"), 12.f, 4);

	TCHAR ValBuf[16] = {};
	swprintf_s(ValBuf, 16, L"%.2f", InitVal);
	auto ValLbl = CreateWidget<CTextBlock>("MENV_" + std::to_string((int)Y), 3).lock();
	if (ValLbl)
	{
		ValLbl->SetPos(66.f, Y);
		ValLbl->SetSize(80.f, ROW_H - 2.f);
		ValLbl->SetText(ValBuf);
		ValLbl->SetFontSize(11.f);
		ValLbl->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		ValLbl->SetAlignH(ETextAlignH::Center);
		ValLbl->SetAlignV(ETextAlignV::Middle);
		Row.ValueLbl = ValLbl;
	}

	Row.PlusBtn = MakeButton("MENP_" + std::to_string((int)Y), 148.f, Y, 18.f, ROW_H - 2.f, 0.26f, 0.26f, 0.32f);
	MakeLabel("MENPL_" + std::to_string((int)Y), 148.f, Y, 18.f, ROW_H - 2.f, TEXT("+"), 12.f, 4);
}

std::weak_ptr<CButton> CMaterialEditorUI::MakeButton(const std::string& Name,
                                                     float X, float Y, float W, float H,
                                                     float r, float g, float b)
{
	auto Btn = CreateWidget<CButton>(Name, 3).lock();
	if (Btn)
	{
		Btn->SetPos(X, Y);
		Btn->SetSize(W, H);
		Btn->SetTint(EWidgetState::Normal,  r,       g,       b,       1.f);
		Btn->SetTint(EWidgetState::Hovered, r+0.12f, g+0.12f, b+0.16f, 1.f);
		Btn->SetTint(EWidgetState::Clicked, r+0.22f, g+0.22f, b+0.28f, 1.f);
		Btn->SetTint(EWidgetState::Release, r+0.12f, g+0.12f, b+0.16f, 1.f);
		Btn->SetTint(EWidgetState::Disable, 0.20f,   0.20f,   0.24f,   1.f);
	}
	return Btn;
}

std::weak_ptr<CTextBlock> CMaterialEditorUI::MakeLabel(const std::string& Name,
                                                       float X, float Y, float W, float H,
                                                       const wchar_t* Text, float FontSize, int ZOrder)
{
	auto Lbl = CreateWidget<CTextBlock>(Name, ZOrder).lock();
	if (Lbl)
	{
		Lbl->SetPos(X, Y);
		Lbl->SetSize(W, H);
		Lbl->SetText(Text);
		Lbl->SetFontSize(FontSize);
		Lbl->SetTextColor(FVector4(0.80f, 0.80f, 0.80f, 1.f));
		Lbl->SetAlignH(ETextAlignH::Left);
		Lbl->SetAlignV(ETextAlignV::Middle);
	}
	return Lbl;
}

// ── 초기화 ──────────────────────────────────────────────────────────────
bool CMaterialEditorUI::Init()
{
	CWidgetContainer::Init();

	SetPos(PANEL_X, PANEL_Y);
	SetSize(PANEL_W, PANEL_H);
	SetEnable(false); // 기본적으로 숨김

	// 배경
	auto Bg = CreateWidget<CButton>("MatEdBg", 0).lock();
	if (Bg)
	{
		Bg->SetPos(0.f, 0.f);
		Bg->SetSize(PANEL_W, PANEL_H);
		Bg->SetTint(EWidgetState::Normal,  0.12f, 0.12f, 0.14f, 0.95f);
		Bg->SetTint(EWidgetState::Hovered, 0.12f, 0.12f, 0.14f, 0.95f);
		Bg->SetTint(EWidgetState::Clicked, 0.12f, 0.12f, 0.14f, 0.95f);
		Bg->SetTint(EWidgetState::Release, 0.12f, 0.12f, 0.14f, 0.95f);
		Bg->SetTint(EWidgetState::Disable, 0.12f, 0.12f, 0.14f, 0.95f);
		mBackground = Bg;
	}

	// 타이틀바
	auto TitleBar = CreateWidget<CTitleBar>("MatEdTitleBar", 1).lock();
	if (TitleBar)
	{
		TitleBar->SetPos(0.f, 0.f);
		TitleBar->SetSize(PANEL_W, TITLE_H);
		TitleBar->SetTint(0.22f, 0.22f, 0.28f, 1.f);
		TitleBar->SetUpdateWidget(GetThisPtr<CWidget>());
		mTitleBarWidget = TitleBar;
	}

	MakeLabel("MatEdTitle", 0.f, 0.f, PANEL_W - TITLE_H, TITLE_H, TEXT("Material Editor"), 14.f, 2);
	if (auto T = FindWidget<CTextBlock>("MatEdTitle").lock())
	{
		T->SetTextColor(FVector4::White);
		T->SetAlignH(ETextAlignH::Center);
	}

	// 타이틀바 우측 상단 닫기 [X] 버튼
	mCloseBtn = MakeButton("MatEdClose", PANEL_W - TITLE_H + 2.f, 2.f, TITLE_H - 4.f, TITLE_H - 4.f, 0.55f, 0.18f, 0.18f);
	MakeLabel("MatEdCloseLbl", PANEL_W - TITLE_H + 2.f, 2.f, TITLE_H - 4.f, TITLE_H - 4.f, TEXT("X"), 13.f, 4);
	if (auto L = FindWidget<CTextBlock>("MatEdCloseLbl").lock())
	{
		L->SetTextColor(FVector4::White);
		L->SetAlignH(ETextAlignH::Center);
		L->SetAlignV(ETextAlignV::Middle);
	}

	float Y = TITLE_H + 8.f;

	// ── Shader / Sampler / Blend 드롭다운 ───────────────────────────────
	MakeDropdown(Y, TEXT("Shader"),  mShaderDD,  sShaderNames,  0, "MDS"); Y += ROW_H + 4.f;
	MakeDropdown(Y, TEXT("Sampler"), mSamplerDD, sSamplerNames, 0, "MDP"); Y += ROW_H + 4.f;
	MakeDropdown(Y, TEXT("Blend"),   mBlendDD,   sBlendNames,   0, "MDB"); Y += ROW_H + 4.f;

	// ── 텍스처 탐색 ────────────────────────────────────────────────────
	MakeLabel("MatTexLbl", 6.f, Y, 52.f, ROW_H - 2.f, TEXT("Texture"), 11.f);
	auto TexDisp = CreateWidget<CTextBlock>("MatTexDisp", 3).lock();
	if (TexDisp)
	{
		TexDisp->SetPos(60.f, Y);
		TexDisp->SetSize(110.f, ROW_H - 2.f);
		TexDisp->SetText(TEXT("(none)"));
		TexDisp->SetFontSize(10.f);
		TexDisp->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		TexDisp->SetAlignH(ETextAlignH::Left);
		TexDisp->SetAlignV(ETextAlignV::Middle);
		mTexDisp = TexDisp;
	}
	mTexBtn = MakeButton("MatTexBtn", 172.f, Y, 40.f, ROW_H - 2.f, 0.20f, 0.30f, 0.46f);
	MakeLabel("MatTexBtnL", 172.f, Y, 40.f, ROW_H - 2.f, TEXT("..."), 11.f, 4);
	Y += ROW_H + 4.f;

	// ── 구분선 레이블 ─────────────────────────────────────────────────
	auto DivLbl = CreateWidget<CTextBlock>("MatDivLbl", 3).lock();
	if (DivLbl)
	{
		DivLbl->SetPos(6.f, Y);
		DivLbl->SetSize(PANEL_W - 12.f, 14.f);
		DivLbl->SetText(TEXT("BaseColor / Opacity"));
		DivLbl->SetFontSize(11.f);
		DivLbl->SetTextColor(FVector4(0.55f, 0.75f, 1.f, 1.f));
		DivLbl->SetAlignH(ETextAlignH::Left);
		DivLbl->SetAlignV(ETextAlignV::Middle);
	}
	Y += 18.f;

	// ── 색상 행: R G B A Opacity ──────────────────────────────────────
	MakeNumRow(Y, TEXT("R"),    mColorRows[0], 1.f, 0.05f); Y += ROW_H + 3.f;
	MakeNumRow(Y, TEXT("G"),    mColorRows[1], 1.f, 0.05f); Y += ROW_H + 3.f;
	MakeNumRow(Y, TEXT("B"),    mColorRows[2], 1.f, 0.05f); Y += ROW_H + 3.f;
	MakeNumRow(Y, TEXT("A"),    mColorRows[3], 1.f, 0.05f); Y += ROW_H + 3.f;
	MakeNumRow(Y, TEXT("Opac"), mColorRows[4], 1.f, 0.05f); Y += ROW_H + 10.f;

	// ── 생성 / 할당 버튼 ───────────────────────────────────────────────
	auto MakeWideBtn = [&](const std::string& Name, float BtnY, const wchar_t* Lbl,
	                       float r, float g, float b) -> std::weak_ptr<CButton>
	{
		auto B = MakeButton(Name, 8.f, BtnY, PANEL_W - 16.f, 24.f, r, g, b);
		auto L = CreateWidget<CTextBlock>(Name + "_L", 4).lock();
		if (L)
		{
			L->SetPos(8.f, BtnY);
			L->SetSize(PANEL_W - 16.f, 24.f);
			L->SetText(Lbl);
			L->SetFontSize(12.f);
			L->SetTextColor(FVector4::White);
			L->SetAlignH(ETextAlignH::Center);
			L->SetAlignV(ETextAlignV::Middle);
		}
		return B;
	};

	mCreateBtn = MakeWideBtn("MatCreate", Y, TEXT("Create Material"), 0.16f, 0.42f, 0.22f);
	Y += 28.f + 4.f;
	mAssignBtn = MakeWideBtn("MatAssign", Y, TEXT("Assign to Selected"), 0.16f, 0.28f, 0.46f);
	Y += 28.f + 4.f;
	mSaveBtn   = MakeWideBtn("MatSave",   Y, TEXT("Save .mat"),          0.38f, 0.25f, 0.12f);
	Y += 28.f + 4.f;
	mLoadBtn   = MakeWideBtn("MatLoad",   Y, TEXT("Load .mat"),          0.20f, 0.20f, 0.36f);
	Y += 28.f + 6.f;

	// 상태 레이블
	auto Stat = CreateWidget<CTextBlock>("MatStatus", 3).lock();
	if (Stat)
	{
		Stat->SetPos(6.f, Y);
		Stat->SetSize(PANEL_W - 12.f, ROW_H);
		Stat->SetText(TEXT(""));
		Stat->SetFontSize(10.f);
		Stat->SetTextColor(FVector4(0.70f, 1.f, 0.70f, 1.f));
		Stat->SetAlignH(ETextAlignH::Left);
		Stat->SetAlignV(ETextAlignV::Middle);
		mStatusLbl = Stat;
	}

	return true;
}

// ── 헬퍼 함수 ──────────────────────────────────────────────────────────────
void CMaterialEditorUI::RefreshDropdownDisplays()
{
	auto SyncDD = [](FMatDropdown& DD, int Idx) {
		if (Idx < 0 || Idx >= (int)DD.Items.size()) return;
		DD.SelectedIdx = Idx;
		if (auto V = DD.ValueLbl.lock())
		{
			std::wstring W(DD.Items[Idx].begin(), DD.Items[Idx].end());
			V->SetText(W.c_str());
		}
		// 항목 강조 색상 업데이트
		for (int i = 0; i < (int)DD.ItemLbls.size(); ++i)
		{
			if (auto L = DD.ItemLbls[i].lock())
				L->SetTextColor(i == Idx ? FVector4(0.5f, 1.f, 0.65f, 1.f) : FVector4(0.85f, 0.85f, 0.85f, 1.f));
		}
	};
	SyncDD(mShaderDD,  mShaderIdx);
	SyncDD(mSamplerDD, mSamplerIdx);
	SyncDD(mBlendDD,   mBlendIdx);
}

void CMaterialEditorUI::ApplyNumRow(FColorRow& Row, float Dir, float DeltaTime)
{
	Row.Value = max(0.f, min(1.f, Row.Value + Dir * Row.Step));
	if (auto V = Row.ValueLbl.lock())
	{
		TCHAR Buf[16] = {};
		swprintf_s(Buf, 16, L"%.2f", Row.Value);
		V->SetText(Buf);
	}
}

void CMaterialEditorUI::RefreshNumDisplays()
{
	for (int i = 0; i < 5; ++i)
	{
		if (auto V = mColorRows[i].ValueLbl.lock())
		{
			TCHAR Buf[16] = {};
			swprintf_s(Buf, 16, L"%.2f", mColorRows[i].Value);
			V->SetText(Buf);
		}
	}
}

// 현재 UI 설정으로 머티리얼 매니저에 머티리얼을 생성/갱신
void CMaterialEditorUI::ApplyToManager(const std::string& Name)
{
	auto AssetMgr = CAssetManager::GetInst();
	if (!AssetMgr) return;
	auto MatMgr = AssetMgr->GetSubManager<CMaterialManager>(EAssetType::Material);
	if (!MatMgr) return;

	// 이미 존재하면 CreateMaterial은 조용히 실패함 — 무시
	MatMgr->CreateMaterial(Name, sShaderNames[mShaderIdx], sSamplerNames[mSamplerIdx]);

	auto Mat = MatMgr->FindMaterial(Name).lock();
	if (!Mat) return;

	Mat->SetBaseColor(mColorRows[0].Value, mColorRows[1].Value, mColorRows[2].Value, mColorRows[3].Value);
	Mat->SetOpacity(mColorRows[4].Value);
	if (mBlendIdx > 0) Mat->SetRenderState(sBlendNames[mBlendIdx]);
	if (!mTexFullPath.empty())
	{
		int Len = MultiByteToWideChar(CP_ACP, 0, mTexFullPath.c_str(), -1, nullptr, 0);
		std::wstring WPath(Len > 0 ? Len - 1 : 0, 0);
		if (Len > 0) MultiByteToWideChar(CP_ACP, 0, mTexFullPath.c_str(), -1, &WPath[0], Len);
		Mat->AddTextureFullPath(mTexName, WPath.c_str());
	}
}

void CMaterialEditorUI::SaveToFile()
{
	std::string MatDir = DialogUtil::GetExeDir() + "Asset\\Material\\";
	CreateDirectoryA(MatDir.c_str(), nullptr);

	std::string Path = DialogUtil::SaveFile(
		"Material Files\0*.mat\0All Files\0*.*\0",
		MatDir.c_str(), "mat");
	if (Path.empty()) return;

	mMatName = DialogUtil::ExtractBaseName(Path);

	std::ofstream File(Path);
	if (!File.is_open())
	{
		if (auto S = mStatusLbl.lock()) S->SetText(TEXT("Save failed: can't open file"));
		return;
	}

	File << "Name="    << mMatName                   << "\n";
	File << "Shader="  << sShaderNames[mShaderIdx]   << "\n";
	File << "Sampler=" << sSamplerNames[mSamplerIdx] << "\n";
	File << "Blend="   << sBlendNames[mBlendIdx]     << "\n";
	File << "TexName=" << mTexName                   << "\n";
	File << "TexPath=" << mTexFullPath               << "\n";
	File << "R="       << mColorRows[0].Value        << "\n";
	File << "G="       << mColorRows[1].Value        << "\n";
	File << "B="       << mColorRows[2].Value        << "\n";
	File << "A="       << mColorRows[3].Value        << "\n";
	File << "Opacity=" << mColorRows[4].Value        << "\n";
	File.close();

	// MaterialManager에 등록하여 즉시 사용 가능하게 함
	ApplyToManager(mMatName);

	if (auto S = mStatusLbl.lock())
	{
		std::wstring WN(mMatName.begin(), mMatName.end());
		TCHAR Buf[128] = {};
		wsprintf(Buf, TEXT("Saved: %s"), WN.c_str());
		S->SetText(Buf);
	}

	if (mOnMaterialUpdated) mOnMaterialUpdated();
}

void CMaterialEditorUI::LoadFromFile()
{
	std::string MatDir = DialogUtil::GetExeDir() + "Asset\\Material\\";
	CreateDirectoryA(MatDir.c_str(), nullptr);

	std::string Path = DialogUtil::OpenFile(
		"Material Files\0*.mat\0All Files\0*.*\0",
		MatDir.c_str());
	if (Path.empty()) return;

	std::ifstream File(Path);
	if (!File.is_open())
	{
		if (auto S = mStatusLbl.lock()) S->SetText(TEXT("Load failed: can't open file"));
		return;
	}

	std::unordered_map<std::string, std::string> Props;
	std::string Line;
	while (std::getline(File, Line))
	{
		auto eq = Line.find('=');
		if (eq == std::string::npos) continue;
		Props[Line.substr(0, eq)] = Line.substr(eq + 1);
	}
	File.close();

	auto Get = [&](const std::string& Key, const std::string& Def) -> std::string {
		auto it = Props.find(Key);
		return (it != Props.end()) ? it->second : Def;
	};
	auto GetF = [&](const std::string& Key, float Def) -> float {
		auto it = Props.find(Key);
		if (it == Props.end() || it->second.empty()) return Def;
		try { return std::stof(it->second); } catch (...) { return Def; }
	};

	mMatName = Get("Name", DialogUtil::ExtractBaseName(Path));

	std::string Shader  = Get("Shader",  sShaderNames[0]);
	std::string Sampler = Get("Sampler", sSamplerNames[0]);
	std::string Blend   = Get("Blend",   sBlendNames[0]);

	mShaderIdx  = 0;
	mSamplerIdx = 0;
	mBlendIdx   = 0;
	for (int i = 0; i < (int)sShaderNames.size();  ++i) if (sShaderNames[i]  == Shader)  mShaderIdx  = i;
	for (int i = 0; i < (int)sSamplerNames.size(); ++i) if (sSamplerNames[i] == Sampler) mSamplerIdx = i;
	for (int i = 0; i < (int)sBlendNames.size();   ++i) if (sBlendNames[i]   == Blend)   mBlendIdx   = i;

	mTexName     = Get("TexName", "");
	mTexFullPath = Get("TexPath", "");

	mColorRows[0].Value = GetF("R",       1.f);
	mColorRows[1].Value = GetF("G",       1.f);
	mColorRows[2].Value = GetF("B",       1.f);
	mColorRows[3].Value = GetF("A",       1.f);
	mColorRows[4].Value = GetF("Opacity", 1.f);

	// 모든 표시 갱신
	CloseAllDropdowns();
	RefreshDropdownDisplays();
	RefreshNumDisplays();

	if (auto D = mTexDisp.lock())
	{
		if (!mTexName.empty())
		{
			std::wstring W(mTexName.begin(), mTexName.end());
			D->SetText(W.c_str());
		}
		else D->SetText(TEXT("(none)"));
	}

	// MaterialManager에 등록하여 에셋으로 즉시 사용 가능하게 함
	ApplyToManager(mMatName);

	if (auto S = mStatusLbl.lock())
	{
		std::wstring WN(mMatName.begin(), mMatName.end());
		TCHAR Buf[128] = {};
		wsprintf(Buf, TEXT("Loaded: %s"), WN.c_str());
		S->SetText(Buf);
	}

	if (mOnMaterialUpdated) mOnMaterialUpdated();
}

void CMaterialEditorUI::CreateMaterial()
{
	auto World = mWorld.lock();
	if (!World) return;
	auto AssetMgr = CAssetManager::GetInst();
	if (!AssetMgr) return;

	std::string MatName = "MatEditor_" + std::to_string(mMatCount++);
	std::string Shader  = sShaderNames[mShaderIdx];
	std::string Sampler = sSamplerNames[mSamplerIdx];

	auto MatMgr = AssetMgr->GetSubManager<CMaterialManager>(EAssetType::Material);
	if (!MatMgr) return;

	if (!MatMgr->CreateMaterial(MatName, Shader, Sampler)) return;

	auto MatWeak = MatMgr->FindMaterial(MatName);
	auto Mat = MatWeak.lock();
	if (!Mat) return;

	// 기본 색상
	Mat->SetBaseColor(mColorRows[0].Value, mColorRows[1].Value, mColorRows[2].Value, mColorRows[3].Value);
	Mat->SetOpacity(mColorRows[4].Value);

	// 블렌드
	if (mBlendIdx > 0)
		Mat->SetRenderState(sBlendNames[mBlendIdx]);

	// 텍스처
	if (!mTexFullPath.empty())
	{
		std::wstring WPath(mTexFullPath.begin(), mTexFullPath.end());
		Mat->AddTextureFullPath(mTexName, WPath.c_str());
	}

	if (auto S = mStatusLbl.lock())
	{
		std::wstring WName(MatName.begin(), MatName.end());
		TCHAR Buf[64] = {};
		wsprintf(Buf, TEXT("Created: %s"), WName.c_str());
		S->SetText(Buf);
	}

	if (mOnMaterialUpdated) mOnMaterialUpdated();
}

void CMaterialEditorUI::AssignToSelected()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor)
	{
		if (auto S = mStatusLbl.lock()) S->SetText(TEXT("No actor selected"));
		return;
	}

	for (const auto& Comp : Actor->GetSceneCompList())
	{
		auto MC = std::dynamic_pointer_cast<CMeshComponent>(Comp);
		if (!MC) continue;

		MC->SetShader(sShaderNames[mShaderIdx]);

		if (!mTexFullPath.empty())
		{
			int Len = MultiByteToWideChar(CP_ACP, 0, mTexFullPath.c_str(), -1, nullptr, 0);
			std::wstring WPath(Len > 0 ? Len - 1 : 0, 0);
			if (Len > 0) MultiByteToWideChar(CP_ACP, 0, mTexFullPath.c_str(), -1, &WPath[0], Len);
			MC->AddTextureFullPath(0, mTexName, WPath.c_str());
		}

		if (mBlendIdx > 0)
			MC->SetBlendState(0, sBlendNames[mBlendIdx]);

		MC->SetBaseColor(0, mColorRows[0].Value, mColorRows[1].Value, mColorRows[2].Value, mColorRows[3].Value);
		MC->SetOpacity(0, mColorRows[4].Value);

		if (auto S = mStatusLbl.lock()) S->SetText(TEXT("Assigned!"));
		if (mOnMaterialUpdated) mOnMaterialUpdated();
		return;
	}

	if (auto S = mStatusLbl.lock()) S->SetText(TEXT("No MeshComponent found"));
}

// ── 업데이트 ───────────────────────────────────────────────────────────────
void CMaterialEditorUI::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);

	// ── 드롭다운 처리 ────────────────────────────────────────────────────────
	auto HandleDD = [&](FMatDropdown& DD, int& Idx, FMatDropdown* Other1, FMatDropdown* Other2)
	{
		// 토글 버튼
		if (auto Tog = DD.ToggleBtn.lock())
		{
			if (Tog->GetWidgetState() == EWidgetState::Release)
			{
				if (DD.bOpen)
					CloseDropdown(DD);
				else
				{
					// 다른 드롭다운 먼저 닫기
					if (Other1) CloseDropdown(*Other1);
					if (Other2) CloseDropdown(*Other2);
					OpenDropdown(DD);
				}
			}
		}

		// 항목 버튼
		for (int i = 0; i < (int)DD.ItemBtns.size(); ++i)
		{
			if (auto IB = DD.ItemBtns[i].lock())
			{
				if (IB->GetWidgetState() == EWidgetState::Release)
				{
					Idx = i;
					DD.SelectedIdx = i;
					// 값 레이블 업데이트
					if (auto VL = DD.ValueLbl.lock())
					{
						std::wstring W(DD.Items[i].begin(), DD.Items[i].end());
						VL->SetText(W.c_str());
					}
					// 항목 강조 색상 업데이트
					for (int j = 0; j < (int)DD.ItemLbls.size(); ++j)
						if (auto L = DD.ItemLbls[j].lock())
							L->SetTextColor(j == i ? FVector4(0.5f, 1.f, 0.65f, 1.f) : FVector4(0.85f, 0.85f, 0.85f, 1.f));
					CloseDropdown(DD);
					break;
				}
			}
		}
	};

	HandleDD(mShaderDD,  mShaderIdx,  &mSamplerDD, &mBlendDD);
	HandleDD(mSamplerDD, mSamplerIdx, &mShaderDD,  &mBlendDD);
	HandleDD(mBlendDD,   mBlendIdx,   &mShaderDD,  &mSamplerDD);

	// 텍스처 탐색
	if (auto Btn = mTexBtn.lock())
	{
		if (Btn->GetWidgetState() == EWidgetState::Release)
		{
			std::string TexDir = DialogUtil::GetExeDir() + "Asset\\Texture\\";
			std::string Path   = DialogUtil::OpenFile(
				"Texture Files\0*.png;*.dds;*.jpg;*.bmp;*.tga\0All\0*.*\0",
				TexDir.c_str());
			if (!Path.empty())
			{
				mTexFullPath = Path;
				mTexName     = DialogUtil::ExtractBaseName(Path);
				if (auto D = mTexDisp.lock())
				{
					std::wstring W(mTexName.begin(), mTexName.end());
					D->SetText(W.c_str());
				}
			}
		}
	}

	// 색상 +/- 행
	auto HandleNumRow = [&](FColorRow& Row, float DT)
	{
		if (auto MinBtn = Row.MinusBtn.lock())
		{
			EWidgetState::Type S = MinBtn->GetWidgetState();
			if (S == EWidgetState::Clicked)
			{
				Row.MinHoldTime += DT;
				int Expected = (Row.MinHoldTime > 0.35f) ? (int)((Row.MinHoldTime - 0.35f) / 0.08f) + 1 : 0;
				while (Row.MinRepeat < Expected) { ApplyNumRow(Row, -1.f, DT); ++Row.MinRepeat; }
			}
			else
			{
				if (S == EWidgetState::Release && Row.MinHoldTime < 0.35f) ApplyNumRow(Row, -1.f, DT);
				Row.MinHoldTime = 0.f; Row.MinRepeat = 0;
			}
		}
		if (auto PlusBtn = Row.PlusBtn.lock())
		{
			EWidgetState::Type S = PlusBtn->GetWidgetState();
			if (S == EWidgetState::Clicked)
			{
				Row.PlusHoldTime += DT;
				int Expected = (Row.PlusHoldTime > 0.35f) ? (int)((Row.PlusHoldTime - 0.35f) / 0.08f) + 1 : 0;
				while (Row.PlusRepeat < Expected) { ApplyNumRow(Row, +1.f, DT); ++Row.PlusRepeat; }
			}
			else
			{
				if (S == EWidgetState::Release && Row.PlusHoldTime < 0.35f) ApplyNumRow(Row, +1.f, DT);
				Row.PlusHoldTime = 0.f; Row.PlusRepeat = 0;
			}
		}
	};

	for (int i = 0; i < 5; ++i) HandleNumRow(mColorRows[i], DeltaTime);

	// 닫기 버튼
	if (auto Btn = mCloseBtn.lock())
		if (Btn->GetWidgetState() == EWidgetState::Release) SetEnable(false);

	// 생성 / 할당 / 저장 / 불러오기 버튼
	if (auto Btn = mCreateBtn.lock())
		if (Btn->GetWidgetState() == EWidgetState::Release) CreateMaterial();

	if (auto Btn = mAssignBtn.lock())
		if (Btn->GetWidgetState() == EWidgetState::Release) AssignToSelected();

	if (auto Btn = mSaveBtn.lock())
		if (Btn->GetWidgetState() == EWidgetState::Release) SaveToFile();

	if (auto Btn = mLoadBtn.lock())
		if (Btn->GetWidgetState() == EWidgetState::Release) LoadFromFile();
}

CMaterialEditorUI* CMaterialEditorUI::Clone()
{
	return new CMaterialEditorUI(*this);
}
