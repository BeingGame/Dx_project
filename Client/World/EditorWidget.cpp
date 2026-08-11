#include "EditorWidget.h"

#include "World/TextBlock.h"

CEditorWidget::CEditorWidget()
{}

CEditorWidget::CEditorWidget(const CEditorWidget & src)
{}

CEditorWidget::~CEditorWidget()
{}

void CEditorWidget::SetEditorMode(EEditorMode Mode)
{
	switch (Mode)
	{
	case EEditorMode::TileType:
		mModeText->SetText(TEXT("EditorMode : Type"));
		break;
	case EEditorMode::TileFrame:
		mModeText->SetText(TEXT("EditorMode : Frame"));
		break;
	default:
		break;
	}
}

void CEditorWidget::SetTileType(ETileType TileType)
{
	switch (TileType)
	{
	case ETileType::Normal:
		mTypeText->SetText(TEXT("TileType : 보통타일"));
		break;
	case ETileType::Wall:
		mTypeText->SetText(TEXT("TileType : 벽타일"));
		break;
	case ETileType::End:
		break;
	default:
		break;
	}
}

void CEditorWidget::SetTileFrame(int Frame)
{
	TCHAR Text[256] = {};
	wsprintf(Text, TEXT("TileFrame : %d"), Frame);
	mFrameText->SetText(Text);
}

bool CEditorWidget::Init()
{
	CWidgetContainer::Init();

	mModeText = CreateWidget<CTextBlock>("Mode").lock();
	mTypeText = CreateWidget<CTextBlock>("Type").lock();
	mFrameText = CreateWidget<CTextBlock>("Frame").lock();

	if (mModeText)
	{
		mModeText->SetText(TEXT("EditorMode : Frame"));
		mModeText->SetTextColor(FVector4::Red);
		mModeText->SetPos(20.f, 20.f);
		mModeText->SetSize(300.f, 100.f);
		mModeText->SetFontSize(30.f);
		mModeText->EnableShadow(true);
		mModeText->SetShadowOffset(2.f, 2.f);
		mModeText->SetShadowTextColor(128, 128, 128, 128);
	}

	if (mTypeText)
	{
		mTypeText->SetText(TEXT("TileType : 보통타일"));
		mTypeText->SetTextColor(FVector4::Red);
		mTypeText->SetPos(420.f, 20.f);
		mTypeText->SetSize(300.f, 100.f);
		mTypeText->SetFontSize(30.f);
		mTypeText->EnableShadow(true);
		mTypeText->SetShadowOffset(2.f, 2.f);
		mTypeText->SetShadowTextColor(128, 128, 128, 128);
	}

	if (mFrameText)
	{
		mFrameText->SetText(TEXT("TileFrame : 0"));
		mFrameText->SetTextColor(FVector4::Red);
		mFrameText->SetPos(820.f, 20.f);
		mFrameText->SetSize(300.f, 100.f);
		mFrameText->SetFontSize(30.f);
		mFrameText->EnableShadow(true);
		mFrameText->SetShadowOffset(2.f, 2.f);
		mFrameText->SetShadowTextColor(128, 128, 128, 128);
	}

	return true;
}

CEditorWidget* CEditorWidget::Clone()
{
	return new CEditorWidget(*this);
}
