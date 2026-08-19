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
                                     FMatDropdown& Dropdown,
                                     const std::vector<std::string>& Items,
                                     int InitIdx, const std::string& NamePrefix)
{
	Dropdown.Items        = Items;
	Dropdown.SelectedIdx  = InitIdx;
	Dropdown.bOpen        = false;
	Dropdown.ItemListY    = Y + ROW_H + 2.f;  // 항목 목록은 헤더 행 바로 아래에 표시

	// 레이블
	MakeLabel(NamePrefix + "_L", 6.f, Y, 52.f, ROW_H - 2.f, Label, 11.f);

	// 현재 선택값 표시
	std::wstring WVal(Items[InitIdx].begin(), Items[InitIdx].end());
	auto ValueLabel = CreateWidget<CTextBlock>(NamePrefix + "_V", 3).lock();
	if (ValueLabel)
	{
		ValueLabel->SetPos(60.f, Y);
		ValueLabel->SetSize(106.f, ROW_H - 2.f);
		ValueLabel->SetText(WVal.c_str());
		ValueLabel->SetFontSize(11.f);
		ValueLabel->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		ValueLabel->SetAlignH(ETextAlignH::Left);
		ValueLabel->SetAlignV(ETextAlignV::Middle);
		Dropdown.ValueLabel = ValueLabel;
	}

	// 토글 버튼 [▼]
	auto ToggleButton = MakeButton(NamePrefix + "_TB", 168.f, Y, 44.f, ROW_H - 2.f, 0.20f, 0.30f, 0.46f);
	auto ToggleLabel = MakeLabel(NamePrefix + "_TL", 168.f, Y, 44.f, ROW_H - 2.f, TEXT("▼"), 11.f, 4);
	Dropdown.ToggleButton = ToggleButton;
	Dropdown.ToggleLabel = ToggleLabel;

	// 목록 배경 (높은 Z순서, 초기에는 화면 밖에 숨김)
	float ListH = (float)Items.size() * (ITEM_H + 1.f);
	auto ListBg = CreateWidget<CButton>(NamePrefix + "_BG", 9).lock();
	if (ListBg)
	{
		ListBg->SetPos(-2000.f, Dropdown.ItemListY);
		ListBg->SetSize(PANEL_W - 12.f, ListH);
		ListBg->SetTint(EWidgetState::Normal,  0.08f, 0.08f, 0.10f, 0.97f);
		ListBg->SetTint(EWidgetState::Hovered, 0.08f, 0.08f, 0.10f, 0.97f);
		ListBg->SetTint(EWidgetState::Clicked, 0.08f, 0.08f, 0.10f, 0.97f);
		ListBg->SetTint(EWidgetState::Release, 0.08f, 0.08f, 0.10f, 0.97f);
		ListBg->SetTint(EWidgetState::Disable, 0.08f, 0.08f, 0.10f, 0.97f);
		Dropdown.ListBg = ListBg;
	}

	// 항목 버튼 (화면 밖에 숨김, Z=10/11)
	float ItemY = Dropdown.ItemListY;
	for (int i = 0; i < (int)Items.size(); ++i)
	{
		bool bSel = (i == InitIdx);
		auto ItemButton = CreateWidget<CButton>(NamePrefix + "_IB" + std::to_string(i), 10).lock();
		if (ItemButton)
		{
			ItemButton->SetPos(-2000.f, ItemY);
			ItemButton->SetSize(PANEL_W - 12.f, ITEM_H);
			ItemButton->SetTint(EWidgetState::Normal,  bSel ? 0.14f : 0.08f, bSel ? 0.32f : 0.12f, bSel ? 0.22f : 0.16f, 1.f);
			ItemButton->SetTint(EWidgetState::Hovered, 0.22f, 0.38f, 0.48f, 1.f);
			ItemButton->SetTint(EWidgetState::Clicked, 0.30f, 0.50f, 0.62f, 1.f);
			ItemButton->SetTint(EWidgetState::Release, 0.22f, 0.38f, 0.48f, 1.f);
			ItemButton->SetTint(EWidgetState::Disable, 0.08f, 0.10f, 0.12f, 1.f);
			Dropdown.ItemButtons.push_back(ItemButton);
		}

		auto ItemLabel = CreateWidget<CTextBlock>(NamePrefix + "_IL" + std::to_string(i), 11).lock();
		if (ItemLabel)
		{
			ItemLabel->SetPos(-2000.f, ItemY);
			ItemLabel->SetSize(PANEL_W - 18.f, ITEM_H);
			std::wstring WItem(Items[i].begin(), Items[i].end());
			ItemLabel->SetText(WItem.c_str());
			ItemLabel->SetFontSize(11.f);
			ItemLabel->SetTextColor(bSel ? FVector4(0.5f, 1.f, 0.65f, 1.f) : FVector4(0.85f, 0.85f, 0.85f, 1.f));
			ItemLabel->SetAlignH(ETextAlignH::Left);
			ItemLabel->SetAlignV(ETextAlignV::Middle);
			Dropdown.ItemLabels.push_back(ItemLabel);
		}
		ItemY += ITEM_H + 1.f;
	}
}

void CMaterialEditorUI::OpenDropdown(FMatDropdown& Dropdown)
{
	Dropdown.bOpen = true;
	if (auto ToggleLabel = Dropdown.ToggleLabel.lock()) ToggleLabel->SetText(TEXT("▲"));
	if (auto ListBackground = Dropdown.ListBg.lock()) ListBackground->SetPos(6.f, Dropdown.ItemListY);
	float ItemY = Dropdown.ItemListY;
	for (int i = 0; i < (int)Dropdown.ItemButtons.size(); ++i)
	{
		if (auto ItemButton = Dropdown.ItemButtons[i].lock()) ItemButton->SetPos(6.f, ItemY);
		if (auto ItemLabel = Dropdown.ItemLabels[i].lock()) ItemLabel->SetPos(12.f, ItemY);
		ItemY += ITEM_H + 1.f;
	}
}

void CMaterialEditorUI::CloseDropdown(FMatDropdown& Dropdown)
{
	Dropdown.bOpen = false;
	if (auto ToggleLabel = Dropdown.ToggleLabel.lock()) ToggleLabel->SetText(TEXT("▼"));
	if (auto ListBackground = Dropdown.ListBg.lock()) ListBackground->SetPos(-2000.f, Dropdown.ItemListY);
	for (int i = 0; i < (int)Dropdown.ItemButtons.size(); ++i)
	{
		if (auto ItemButton = Dropdown.ItemButtons[i].lock()) ItemButton->SetPos(-2000.f, Dropdown.ItemListY);
		if (auto ItemLabel = Dropdown.ItemLabels[i].lock()) ItemLabel->SetPos(-2000.f, Dropdown.ItemListY);
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

	Row.MinusButton = MakeButton("MENM_" + std::to_string((int)Y), 46.f, Y, 18.f, ROW_H - 2.f, 0.26f, 0.26f, 0.32f);
	MakeLabel("MENML_" + std::to_string((int)Y), 46.f, Y, 18.f, ROW_H - 2.f, TEXT("-"), 12.f, 4);

	TCHAR ValBuf[16] = {};
	swprintf_s(ValBuf, 16, L"%.2f", InitVal);
	auto ValueLabel = CreateWidget<CTextBlock>("MENV_" + std::to_string((int)Y), 3).lock();
	if (ValueLabel)
	{
		ValueLabel->SetPos(66.f, Y);
		ValueLabel->SetSize(80.f, ROW_H - 2.f);
		ValueLabel->SetText(ValBuf);
		ValueLabel->SetFontSize(11.f);
		ValueLabel->SetTextColor(FVector4(0.88f, 0.92f, 1.f, 1.f));
		ValueLabel->SetAlignH(ETextAlignH::Center);
		ValueLabel->SetAlignV(ETextAlignV::Middle);
		Row.ValueLabel = ValueLabel;
	}

	Row.PlusButton = MakeButton("MENP_" + std::to_string((int)Y), 148.f, Y, 18.f, ROW_H - 2.f, 0.26f, 0.26f, 0.32f);
	MakeLabel("MENPL_" + std::to_string((int)Y), 148.f, Y, 18.f, ROW_H - 2.f, TEXT("+"), 12.f, 4);
}

std::weak_ptr<CButton> CMaterialEditorUI::MakeButton(const std::string& Name,
                                                     float X, float Y, float Width, float Height,
                                                     float r, float g, float b)
{
	auto Button = CreateWidget<CButton>(Name, 3).lock();
	if (Button)
	{
		Button->SetPos(X, Y);
		Button->SetSize(Width, Height);
		Button->SetTint(EWidgetState::Normal,  r,       g,       b,       1.f);
		Button->SetTint(EWidgetState::Hovered, r+0.12f, g+0.12f, b+0.16f, 1.f);
		Button->SetTint(EWidgetState::Clicked, r+0.22f, g+0.22f, b+0.28f, 1.f);
		Button->SetTint(EWidgetState::Release, r+0.12f, g+0.12f, b+0.16f, 1.f);
		Button->SetTint(EWidgetState::Disable, 0.20f,   0.20f,   0.24f,   1.f);
	}
	return Button;
}

std::weak_ptr<CTextBlock> CMaterialEditorUI::MakeLabel(const std::string& Name,
                                                       float X, float Y, float Width, float Height,
                                                       const wchar_t* Text, float FontSize, int ZOrder)
{
	auto Label = CreateWidget<CTextBlock>(Name, ZOrder).lock();
	if (Label)
	{
		Label->SetPos(X, Y);
		Label->SetSize(Width, Height);
		Label->SetText(Text);
		Label->SetFontSize(FontSize);
		Label->SetTextColor(FVector4(0.80f, 0.80f, 0.80f, 1.f));
		Label->SetAlignH(ETextAlignH::Left);
		Label->SetAlignV(ETextAlignV::Middle);
	}
	return Label;
}

// ── 초기화 ──────────────────────────────────────────────────────────────
bool CMaterialEditorUI::Init()
{
	CWidgetContainer::Init();

	SetPos(PANEL_X, PANEL_Y);
	SetSize(PANEL_W, PANEL_H);
	SetEnable(false); // 기본적으로 숨김

	// 배경
	auto Background = CreateWidget<CButton>("MatEdBg", 0).lock();
	if (Background)
	{
		Background->SetPos(0.f, 0.f);
		Background->SetSize(PANEL_W, PANEL_H);
		Background->SetTint(EWidgetState::Normal,  0.12f, 0.12f, 0.14f, 0.95f);
		Background->SetTint(EWidgetState::Hovered, 0.12f, 0.12f, 0.14f, 0.95f);
		Background->SetTint(EWidgetState::Clicked, 0.12f, 0.12f, 0.14f, 0.95f);
		Background->SetTint(EWidgetState::Release, 0.12f, 0.12f, 0.14f, 0.95f);
		Background->SetTint(EWidgetState::Disable, 0.12f, 0.12f, 0.14f, 0.95f);
		mBackground = Background;
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
	if (auto Title = FindWidget<CTextBlock>("MatEdTitle").lock())
	{
		Title->SetTextColor(FVector4::White);
		Title->SetAlignH(ETextAlignH::Center);
	}

	// 타이틀바 우측 상단 닫기 [X] 버튼
	mCloseButton = MakeButton("MatEdClose", PANEL_W - TITLE_H + 2.f, 2.f, TITLE_H - 4.f, TITLE_H - 4.f, 0.55f, 0.18f, 0.18f);
	MakeLabel("MatEdCloseLbl", PANEL_W - TITLE_H + 2.f, 2.f, TITLE_H - 4.f, TITLE_H - 4.f, TEXT("X"), 13.f, 4);
	if (auto Label = FindWidget<CTextBlock>("MatEdCloseLbl").lock())
	{
		Label->SetTextColor(FVector4::White);
		Label->SetAlignH(ETextAlignH::Center);
		Label->SetAlignV(ETextAlignV::Middle);
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
	mTexButton = MakeButton("MatTexBtn", 172.f, Y, 40.f, ROW_H - 2.f, 0.20f, 0.30f, 0.46f);
	MakeLabel("MatTexBtnL", 172.f, Y, 40.f, ROW_H - 2.f, TEXT("..."), 11.f, 4);
	Y += ROW_H + 4.f;

	// ── 구분선 레이블 ─────────────────────────────────────────────────
	auto DivLabel = CreateWidget<CTextBlock>("MatDivLbl", 3).lock();
	if (DivLabel)
	{
		DivLabel->SetPos(6.f, Y);
		DivLabel->SetSize(PANEL_W - 12.f, 14.f);
		DivLabel->SetText(TEXT("BaseColor / Opacity"));
		DivLabel->SetFontSize(11.f);
		DivLabel->SetTextColor(FVector4(0.55f, 0.75f, 1.f, 1.f));
		DivLabel->SetAlignH(ETextAlignH::Left);
		DivLabel->SetAlignV(ETextAlignV::Middle);
	}
	Y += 18.f;

	// ── 색상 행: R G Button A Opacity ──────────────────────────────────────
	MakeNumRow(Y, TEXT("R"),    mColorRows[0], 1.f, 0.05f); Y += ROW_H + 3.f;
	MakeNumRow(Y, TEXT("G"),    mColorRows[1], 1.f, 0.05f); Y += ROW_H + 3.f;
	MakeNumRow(Y, TEXT("B"),    mColorRows[2], 1.f, 0.05f); Y += ROW_H + 3.f;
	MakeNumRow(Y, TEXT("A"),    mColorRows[3], 1.f, 0.05f); Y += ROW_H + 3.f;
	MakeNumRow(Y, TEXT("Opac"), mColorRows[4], 1.f, 0.05f); Y += ROW_H + 10.f;

	// ── 생성 / 할당 버튼 ───────────────────────────────────────────────
	auto MakeWideButton = [&](const std::string& Name, float ButtonY, const wchar_t* LabelText,
	                       float r, float g, float b) -> std::weak_ptr<CButton>
	{
		auto Button = MakeButton(Name, 8.f, ButtonY, PANEL_W - 16.f, 24.f, r, g, b);
		auto Label = CreateWidget<CTextBlock>(Name + "_L", 4).lock();
		if (Label)
		{
			Label->SetPos(8.f, ButtonY);
			Label->SetSize(PANEL_W - 16.f, 24.f);
			Label->SetText(LabelText);
			Label->SetFontSize(12.f);
			Label->SetTextColor(FVector4::White);
			Label->SetAlignH(ETextAlignH::Center);
			Label->SetAlignV(ETextAlignV::Middle);
		}
		return Button;
	};

	mCreateButton = MakeWideButton("MatCreate", Y, TEXT("Create Material"), 0.16f, 0.42f, 0.22f);
	Y += 28.f + 4.f;
	mAssignButton = MakeWideButton("MatAssign", Y, TEXT("Assign to Selected"), 0.16f, 0.28f, 0.46f);
	Y += 28.f + 4.f;
	mSaveButton   = MakeWideButton("MatSave",   Y, TEXT("Save .mat"),          0.38f, 0.25f, 0.12f);
	Y += 28.f + 4.f;
	mLoadButton   = MakeWideButton("MatLoad",   Y, TEXT("Load .mat"),          0.20f, 0.20f, 0.36f);
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
		mStatusLabel = Stat;
	}

	return true;
}

// ── 헬퍼 함수 ──────────────────────────────────────────────────────────────
void CMaterialEditorUI::RefreshDropdownDisplays()
{
	auto SyncDropdown = [](FMatDropdown& Dropdown, int Idx) {
		if (Idx < 0 || Idx >= (int)Dropdown.Items.size()) return;
		Dropdown.SelectedIdx = Idx;
		if (auto ValueLabel = Dropdown.ValueLabel.lock())
		{
			std::wstring WideText(Dropdown.Items[Idx].begin(), Dropdown.Items[Idx].end());
			ValueLabel->SetText(WideText.c_str());
		}
		// 항목 강조 색상 업데이트
		for (int i = 0; i < (int)Dropdown.ItemLabels.size(); ++i)
		{
			if (auto Label = Dropdown.ItemLabels[i].lock())
				Label->SetTextColor(i == Idx ? FVector4(0.5f, 1.f, 0.65f, 1.f) : FVector4(0.85f, 0.85f, 0.85f, 1.f));
		}
	};
	SyncDropdown(mShaderDD,  mShaderIdx);
	SyncDropdown(mSamplerDD, mSamplerIdx);
	SyncDropdown(mBlendDD,   mBlendIdx);
}

void CMaterialEditorUI::ApplyNumRow(FColorRow& Row, float Dir, float DeltaTime)
{
	Row.Value = max(0.f, min(1.f, Row.Value + Dir * Row.Step));
	if (auto ValueLabel = Row.ValueLabel.lock())
	{
		TCHAR TextBuffer[16] = {};
		swprintf_s(TextBuffer, 16, L"%.2f", Row.Value);
		ValueLabel->SetText(TextBuffer);
	}
}

void CMaterialEditorUI::RefreshNumDisplays()
{
	for (int i = 0; i < 5; ++i)
	{
		if (auto ValueLabel = mColorRows[i].ValueLabel.lock())
		{
			TCHAR TextBuffer[16] = {};
			swprintf_s(TextBuffer, 16, L"%.2f", mColorRows[i].Value);
			ValueLabel->SetText(TextBuffer);
		}
	}
}

// 현재 UI 설정으로 머티리얼 매니저에 머티리얼을 생성/갱신
void CMaterialEditorUI::ApplyToManager(const std::string& Name)
{
	auto AssetManager = CAssetManager::GetInst();
	if (!AssetManager) return;
	auto MaterialManager = AssetManager->GetSubManager<CMaterialManager>(EAssetType::Material);
	if (!MaterialManager) return;

	// 이미 존재하면 CreateMaterial은 조용히 실패함 — 무시
	MaterialManager->CreateMaterial(Name, sShaderNames[mShaderIdx], sSamplerNames[mSamplerIdx]);

	auto Material = MaterialManager->FindMaterial(Name).lock();
	if (!Material) return;

	Material->SetBaseColor(mColorRows[0].Value, mColorRows[1].Value, mColorRows[2].Value, mColorRows[3].Value);
	Material->SetOpacity(mColorRows[4].Value);
	if (mBlendIdx > 0) Material->SetRenderState(sBlendNames[mBlendIdx]);
	if (!mTexFullPath.empty())
	{
		int Length = MultiByteToWideChar(CP_ACP, 0, mTexFullPath.c_str(), -1, nullptr, 0);
		std::wstring WPath(Length > 0 ? Length - 1 : 0, 0);
		if (Length > 0) MultiByteToWideChar(CP_ACP, 0, mTexFullPath.c_str(), -1, &WPath[0], Length);
		Material->AddTextureFullPath(mTexName, WPath.c_str());
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
		if (auto StatusLabel = mStatusLabel.lock()) StatusLabel->SetText(TEXT("Save failed: can't open file"));
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

	if (auto StatusLabel = mStatusLabel.lock())
	{
		std::wstring WideName(mMatName.begin(), mMatName.end());
		TCHAR TextBuffer[128] = {};
		wsprintf(TextBuffer, TEXT("Saved: %s"), WideName.c_str());
		StatusLabel->SetText(TextBuffer);
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
		if (auto StatusLabel = mStatusLabel.lock()) StatusLabel->SetText(TEXT("Load failed: can't open file"));
		return;
	}

	std::unordered_map<std::string, std::string> Props;
	std::string Line;
	while (std::getline(File, Line))
	{
		auto EqualPos = Line.find('=');
		if (EqualPos == std::string::npos) continue;
		Props[Line.substr(0, EqualPos)] = Line.substr(EqualPos + 1);
	}
	File.close();

	auto GetString = [&](const std::string& Key, const std::string& DefaultValue) -> std::string {
		auto Found = Props.find(Key);
		return (Found != Props.end()) ? Found->second : DefaultValue;
	};
	auto GetF = [&](const std::string& Key, float DefaultValue) -> float {
		auto Found = Props.find(Key);
		if (Found == Props.end() || Found->second.empty()) return DefaultValue;
		try { return std::stof(Found->second); } catch (...) { return DefaultValue; }
	};

	mMatName = GetString("Name", DialogUtil::ExtractBaseName(Path));

	std::string Shader  = GetString("Shader",  sShaderNames[0]);
	std::string Sampler = GetString("Sampler", sSamplerNames[0]);
	std::string Blend   = GetString("Blend",   sBlendNames[0]);

	mShaderIdx  = 0;
	mSamplerIdx = 0;
	mBlendIdx   = 0;
	for (int i = 0; i < (int)sShaderNames.size();  ++i) if (sShaderNames[i]  == Shader)  mShaderIdx  = i;
	for (int i = 0; i < (int)sSamplerNames.size(); ++i) if (sSamplerNames[i] == Sampler) mSamplerIdx = i;
	for (int i = 0; i < (int)sBlendNames.size();   ++i) if (sBlendNames[i]   == Blend)   mBlendIdx   = i;

	mTexName     = GetString("TexName", "");
	mTexFullPath = GetString("TexPath", "");

	mColorRows[0].Value = GetF("R",       1.f);
	mColorRows[1].Value = GetF("G",       1.f);
	mColorRows[2].Value = GetF("B",       1.f);
	mColorRows[3].Value = GetF("A",       1.f);
	mColorRows[4].Value = GetF("Opacity", 1.f);

	// 모든 표시 갱신
	CloseAllDropdowns();
	RefreshDropdownDisplays();
	RefreshNumDisplays();

	if (auto TextureDisplay = mTexDisp.lock())
	{
		if (!mTexName.empty())
		{
			std::wstring WideText(mTexName.begin(), mTexName.end());
			TextureDisplay->SetText(WideText.c_str());
		}
		else TextureDisplay->SetText(TEXT("(none)"));
	}

	// MaterialManager에 등록하여 에셋으로 즉시 사용 가능하게 함
	ApplyToManager(mMatName);

	if (auto StatusLabel = mStatusLabel.lock())
	{
		std::wstring WideName(mMatName.begin(), mMatName.end());
		TCHAR TextBuffer[128] = {};
		wsprintf(TextBuffer, TEXT("Loaded: %s"), WideName.c_str());
		StatusLabel->SetText(TextBuffer);
	}

	if (mOnMaterialUpdated) mOnMaterialUpdated();
}

void CMaterialEditorUI::CreateMaterial()
{
	auto World = mWorld.lock();
	if (!World) return;
	auto AssetManager = CAssetManager::GetInst();
	if (!AssetManager) return;

	std::string MatName = "MatEditor_" + std::to_string(mMatCount++);
	std::string Shader  = sShaderNames[mShaderIdx];
	std::string Sampler = sSamplerNames[mSamplerIdx];

	auto MaterialManager = AssetManager->GetSubManager<CMaterialManager>(EAssetType::Material);
	if (!MaterialManager) return;

	if (!MaterialManager->CreateMaterial(MatName, Shader, Sampler)) return;

	auto MaterialWeak = MaterialManager->FindMaterial(MatName);
	auto Material = MaterialWeak.lock();
	if (!Material) return;

	// 기본 색상
	Material->SetBaseColor(mColorRows[0].Value, mColorRows[1].Value, mColorRows[2].Value, mColorRows[3].Value);
	Material->SetOpacity(mColorRows[4].Value);

	// 블렌드
	if (mBlendIdx > 0)
		Material->SetRenderState(sBlendNames[mBlendIdx]);

	// 텍스처
	if (!mTexFullPath.empty())
	{
		std::wstring WPath(mTexFullPath.begin(), mTexFullPath.end());
		Material->AddTextureFullPath(mTexName, WPath.c_str());
	}

	if (auto StatusLabel = mStatusLabel.lock())
	{
		std::wstring WName(MatName.begin(), MatName.end());
		TCHAR TextBuffer[64] = {};
		wsprintf(TextBuffer, TEXT("Created: %s"), WName.c_str());
		StatusLabel->SetText(TextBuffer);
	}

	if (mOnMaterialUpdated) mOnMaterialUpdated();
}

void CMaterialEditorUI::AssignToSelected()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor)
	{
		if (auto StatusLabel = mStatusLabel.lock()) StatusLabel->SetText(TEXT("No actor selected"));
		return;
	}

	for (const auto& Comp : Actor->GetSceneCompList())
	{
		auto MeshComponent = std::dynamic_pointer_cast<CMeshComponent>(Comp);
		if (!MeshComponent) continue;

		MeshComponent->SetShader(sShaderNames[mShaderIdx]);

		if (!mTexFullPath.empty())
		{
			int Length = MultiByteToWideChar(CP_ACP, 0, mTexFullPath.c_str(), -1, nullptr, 0);
			std::wstring WPath(Length > 0 ? Length - 1 : 0, 0);
			if (Length > 0) MultiByteToWideChar(CP_ACP, 0, mTexFullPath.c_str(), -1, &WPath[0], Length);
			MeshComponent->AddTextureFullPath(0, mTexName, WPath.c_str());
		}

		if (mBlendIdx > 0)
			MeshComponent->SetBlendState(0, sBlendNames[mBlendIdx]);

		MeshComponent->SetBaseColor(0, mColorRows[0].Value, mColorRows[1].Value, mColorRows[2].Value, mColorRows[3].Value);
		MeshComponent->SetOpacity(0, mColorRows[4].Value);

		if (auto StatusLabel = mStatusLabel.lock()) StatusLabel->SetText(TEXT("Assigned!"));
		if (mOnMaterialUpdated) mOnMaterialUpdated();
		return;
	}

	if (auto StatusLabel = mStatusLabel.lock()) StatusLabel->SetText(TEXT("No MeshComponent found"));
}

// ── 업데이트 ───────────────────────────────────────────────────────────────
void CMaterialEditorUI::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);

	// ── 드롭다운 처리 ────────────────────────────────────────────────────────
	auto HandleDD = [&](FMatDropdown& Dropdown, int& Idx, FMatDropdown* Other1, FMatDropdown* Other2)
	{
		// 토글 버튼
		if (auto ToggleButton = Dropdown.ToggleButton.lock())
		{
			if (ToggleButton->GetWidgetState() == EWidgetState::Release)
			{
				if (Dropdown.bOpen)
					CloseDropdown(Dropdown);
				else
				{
					// 다른 드롭다운 먼저 닫기
					if (Other1) CloseDropdown(*Other1);
					if (Other2) CloseDropdown(*Other2);
					OpenDropdown(Dropdown);
				}
			}
		}

		// 항목 버튼
		for (int i = 0; i < (int)Dropdown.ItemButtons.size(); ++i)
		{
			if (auto ItemButton = Dropdown.ItemButtons[i].lock())
			{
				if (ItemButton->GetWidgetState() == EWidgetState::Release)
				{
					Idx = i;
					Dropdown.SelectedIdx = i;
					// 값 레이블 업데이트
					if (auto ValueLabel = Dropdown.ValueLabel.lock())
					{
						std::wstring WideText(Dropdown.Items[i].begin(), Dropdown.Items[i].end());
						ValueLabel->SetText(WideText.c_str());
					}
					// 항목 강조 색상 업데이트
					for (int j = 0; j < (int)Dropdown.ItemLabels.size(); ++j)
						if (auto Label = Dropdown.ItemLabels[j].lock())
							Label->SetTextColor(j == i ? FVector4(0.5f, 1.f, 0.65f, 1.f) : FVector4(0.85f, 0.85f, 0.85f, 1.f));
					CloseDropdown(Dropdown);
					break;
				}
			}
		}
	};

	HandleDD(mShaderDD,  mShaderIdx,  &mSamplerDD, &mBlendDD);
	HandleDD(mSamplerDD, mSamplerIdx, &mShaderDD,  &mBlendDD);
	HandleDD(mBlendDD,   mBlendIdx,   &mShaderDD,  &mSamplerDD);

	// 텍스처 탐색
	if (auto Button = mTexButton.lock())
	{
		if (Button->GetWidgetState() == EWidgetState::Release)
		{
			std::string TexDir = DialogUtil::GetExeDir() + "Asset\\Texture\\";
			std::string Path   = DialogUtil::OpenFile(
				"Texture Files\0*.png;*.dds;*.jpg;*.bmp;*.tga\0All\0*.*\0",
				TexDir.c_str());
			if (!Path.empty())
			{
				mTexFullPath = Path;
				mTexName     = DialogUtil::ExtractBaseName(Path);
				if (auto TextureDisplay = mTexDisp.lock())
				{
					std::wstring WideText(mTexName.begin(), mTexName.end());
					TextureDisplay->SetText(WideText.c_str());
				}
			}
		}
	}

	// 색상 +/- 행
	auto HandleNumRow = [&](FColorRow& Row, float DeltaSeconds)
	{
		if (auto MinButton = Row.MinusButton.lock())
		{
			EWidgetState::Type State = MinButton->GetWidgetState();
			if (State == EWidgetState::Clicked)
			{
				Row.MinHoldTime += DeltaSeconds;
				int Expected = (Row.MinHoldTime > 0.35f) ? (int)((Row.MinHoldTime - 0.35f) / 0.08f) + 1 : 0;
				while (Row.MinRepeat < Expected) { ApplyNumRow(Row, -1.f, DeltaSeconds); ++Row.MinRepeat; }
			}
			else
			{
				if (State == EWidgetState::Release && Row.MinHoldTime < 0.35f) ApplyNumRow(Row, -1.f, DeltaSeconds);
				Row.MinHoldTime = 0.f; Row.MinRepeat = 0;
			}
		}
		if (auto PlusButton = Row.PlusButton.lock())
		{
			EWidgetState::Type State = PlusButton->GetWidgetState();
			if (State == EWidgetState::Clicked)
			{
				Row.PlusHoldTime += DeltaSeconds;
				int Expected = (Row.PlusHoldTime > 0.35f) ? (int)((Row.PlusHoldTime - 0.35f) / 0.08f) + 1 : 0;
				while (Row.PlusRepeat < Expected) { ApplyNumRow(Row, +1.f, DeltaSeconds); ++Row.PlusRepeat; }
			}
			else
			{
				if (State == EWidgetState::Release && Row.PlusHoldTime < 0.35f) ApplyNumRow(Row, +1.f, DeltaSeconds);
				Row.PlusHoldTime = 0.f; Row.PlusRepeat = 0;
			}
		}
	};

	for (int i = 0; i < 5; ++i) HandleNumRow(mColorRows[i], DeltaTime);

	// 닫기 버튼
	if (auto Button = mCloseButton.lock())
		if (Button->GetWidgetState() == EWidgetState::Release) SetEnable(false);

	// 생성 / 할당 / 저장 / 불러오기 버튼
	if (auto Button = mCreateButton.lock())
		if (Button->GetWidgetState() == EWidgetState::Release) CreateMaterial();

	if (auto Button = mAssignButton.lock())
		if (Button->GetWidgetState() == EWidgetState::Release) AssignToSelected();

	if (auto Button = mSaveButton.lock())
		if (Button->GetWidgetState() == EWidgetState::Release) SaveToFile();

	if (auto Button = mLoadButton.lock())
		if (Button->GetWidgetState() == EWidgetState::Release) LoadFromFile();
}

CMaterialEditorUI* CMaterialEditorUI::Clone()
{
	return new CMaterialEditorUI(*this);
}
