#include "StartWidget.h"

#include "World/TextBlock.h"

CStartWidget::CStartWidget()
{}

CStartWidget::CStartWidget(const CStartWidget & src)
{}

CStartWidget::~CStartWidget()
{}

bool CStartWidget::Init()
{
	CWidgetContainer::Init();

    mMainText = CreateWidget<CTextBlock>("Main");
    mEditorText = CreateWidget<CTextBlock>("Editor");

	auto Text = mMainText.lock();

	if (Text)
	{
		Text->SetPos(0.f, 200.f);
		Text->SetSize(1280.f, 100.f);
		Text->SetText(TEXT("Press Space : Main World"));
		Text->SetFontSize(60.f);
		Text->EnableShadow(true);
		Text->SetTextColor(FVector4::Green);
		Text->SetShadowTextColor(FVector4::White);
		Text->SetShadowOffset(5.f, 5.f);
		Text->SetAlignH(ETextAlignH::Center);
	}

	Text = mEditorText.lock();

	if (Text)
	{
		Text->SetPos(0.f, 400.f);
		Text->SetSize(1280.f, 100.f);
		Text->SetText(TEXT("Press Enter : Editor World"));
		Text->SetFontSize(60.f);
		Text->EnableShadow(true);
		Text->SetTextColor(FVector4::Green);
		Text->SetShadowTextColor(FVector4::White);
		Text->SetShadowOffset(5.f, 5.f);
		Text->SetAlignH(ETextAlignH::Center);
	}

    return true;
}

CStartWidget* CStartWidget::Clone()
{
    return new CStartWidget(*this);
}
