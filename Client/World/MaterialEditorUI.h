#pragma once

#include "World/WidgetContainer.h"
#include <functional>

class CMaterialEditorUI : public CWidgetContainer
{
public:
	CMaterialEditorUI();
	CMaterialEditorUI(const CMaterialEditorUI& src);
	virtual ~CMaterialEditorUI();

private:
	static constexpr float PANEL_W  = 220.f;
	static constexpr float PANEL_H  = 440.f;
	static constexpr float PANEL_X  = 430.f;
	static constexpr float PANEL_Y  = 80.f;
	static constexpr float TITLE_H  = 28.f;
	static constexpr float ROW_H    = 20.f;
	static constexpr float ITEM_H   = 20.f;

	// ── 위젯 참조 ────────────────────────────────────────────────────────────
	std::weak_ptr<class CButton>    mBackground;
	std::weak_ptr<class CTitleBar>  mTitleBarWidget;

	// ── 드롭다운 콤보박스 ────────────────────────────────────────────────────
	struct FMatDropdown
	{
		std::weak_ptr<class CTextBlock> ValueLabel;
		std::weak_ptr<class CButton>    ToggleButton;
		std::weak_ptr<class CTextBlock> ToggleLabel;
		std::weak_ptr<class CButton>    ListBg;
		std::vector<std::weak_ptr<class CButton>>    ItemButtons;
		std::vector<std::weak_ptr<class CTextBlock>> ItemLabels;
		std::vector<std::string> Items;
		int   SelectedIdx = 0;
		bool  bOpen       = false;
		float ItemListY   = 0.f;   // 항목 목록이 시작되는 Y 좌표 (패널 로컬)
	};
	FMatDropdown mShaderDD;
	FMatDropdown mSamplerDD;
	FMatDropdown mBlendDD;

	// 텍스처 탐색
	std::weak_ptr<class CTextBlock> mTexDisp;
	std::weak_ptr<class CButton>    mTexButton;

	// 색상 R/G/B/A + Opacity (수치 행)
	struct FColorRow {
		std::weak_ptr<class CButton>    MinusButton;
		std::weak_ptr<class CTextBlock> ValueLabel;
		std::weak_ptr<class CButton>    PlusButton;
		float Step = 0.05f;
		float Value = 1.f;
		float MinHoldTime = 0.f; int MinRepeat = 0;
		float PlusHoldTime= 0.f; int PlusRepeat= 0;
	};
	FColorRow mColorRows[5];  // R G B A Opacity

	// 생성 / 할당 / 저장 / 불러오기 버튼
	std::weak_ptr<class CButton>    mCreateButton;
	std::weak_ptr<class CButton>    mAssignButton;
	std::weak_ptr<class CButton>    mSaveButton;
	std::weak_ptr<class CButton>    mLoadButton;
	std::weak_ptr<class CTextBlock> mStatusLabel;
	std::weak_ptr<class CButton>    mCloseButton;

	// ── 현재 머티리얼 설정 ───────────────────────────────────────────────
	int mShaderIdx  = 0;
	int mSamplerIdx = 0;
	int mBlendIdx   = 0;
	std::string mTexFullPath;
	std::string mTexName;
	std::string mMatName;
	int         mMatCount = 0;

	std::weak_ptr<class CActor> mSelectedActor;

	static const std::vector<std::string> sShaderNames;
	static const std::vector<std::string> sSamplerNames;
	static const std::vector<std::string> sBlendNames;

	// ── 헬퍼 함수 ────────────────────────────────────────────────────────
	void MakeDropdown(float Y, const wchar_t* Label,
	                  FMatDropdown& Dropdown,
	                  const std::vector<std::string>& Items,
	                  int InitIdx, const std::string& NamePrefix);

	void OpenDropdown(FMatDropdown& Dropdown);
	void CloseDropdown(FMatDropdown& Dropdown);
	void CloseAllDropdowns();

	void MakeNumRow(float Y, const wchar_t* Label, FColorRow& Row, float InitVal, float Step);

	std::weak_ptr<class CButton> MakeButton(const std::string& Name,
	                                        float X, float Y, float Width, float Height,
	                                        float r, float g, float b);

	std::weak_ptr<class CTextBlock> MakeLabel(const std::string& Name,
	                                          float X, float Y, float Width, float Height,
	                                          const wchar_t* Text, float FontSize = 12.f,
	                                          int ZOrder = 3);

	void RefreshDropdownDisplays();
	void RefreshNumDisplays();
	void ApplyNumRow(FColorRow& Row, float Dir, float DeltaTime);
	void ApplyToManager(const std::string& Name);
	void CreateMaterial();
	void AssignToSelected();
	void SaveToFile();
	void LoadFromFile();

	std::function<void()> mOnMaterialUpdated;

public:
	void SetSelectedActor(std::weak_ptr<class CActor> Actor) { mSelectedActor = Actor; }
	void SetOnMaterialUpdated(std::function<void()> Function) { mOnMaterialUpdated = std::move(Function); }

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual CMaterialEditorUI* Clone();
};
