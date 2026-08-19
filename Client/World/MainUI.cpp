#include "MainUI.h"

#include "World/Button.h"
#include "World/TextBlock.h"
#include "World/ProgressBar.h"
#include "World/TitleBar.h"
#include "World/NumberWidget.h"

#include "LogManager.h"

#include "MainWorld.h"

CMainUI::CMainUI()
{}

CMainUI::CMainUI(const CMainUI& src)
{}

CMainUI::~CMainUI()
{}

bool CMainUI::Init()
{
	CWidgetContainer::Init();

	mTestButton = CreateWidget<CButton>("TestButton", 0);
	mText = CreateWidget<CTextBlock>("Text", 0);
	mBar = CreateWidget<CProgressBar>("Bar", 0);
	mTitleBar = CreateWidget<CTitleBar>("Title", 1);
	mNumber = CreateWidget<CNumberWidget>("Number", 2);

	auto TestButton = mTestButton.lock();

	if (TestButton)
	{
		TestButton->SetPos(100.f, 100.f);
		TestButton->SetSize(200.f, 70.f);

		TestButton->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CMainUI::TestButtonHovered);
		TestButton->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CMainUI::TestButtonClicked);
	}

	auto Text = mText.lock();

	if (Text)
	{
		Text->SetPos(100.f, 100.f);
		Text->SetSize(300.f, 100.f);
		Text->SetText(TEXT("My Text Block"));
		Text->SetFontSize(60.f);
		Text->EnableShadow(true);
		Text->SetTextColor(FVector4::Green);
		Text->SetShadowTextColor(FVector4::White);
		Text->SetShadowOffset(5.f, 5.f);
	}

	auto ProgressBar = mBar.lock();

	if (ProgressBar)
	{
		ProgressBar->SetPos(200.f, 200.f);
		ProgressBar->SetSize(300.f, 30.f);

		ProgressBar->SetMinValue(0.f);
		ProgressBar->SetMaxValue(100.f);
		ProgressBar->SetPercent(0.5f);

		ProgressBar->SetTint(EProgressBarImageType::Back, 0.3f, 0.3f, 0.3f, 1.f);
		ProgressBar->SetTint(EProgressBarImageType::Fill, 1.f, 1.f, 1.f, 1.f);

		auto World = mWorld.lock();

		if (World)
		{
			World->SetTimer(1.f, true, this, &CMainUI::ProgressBarTimer);
		}
	}

	auto TitleBar = mTitleBar.lock();

	if (TitleBar)
	{
		TitleBar->SetSize(300.f, 30.f);
		TitleBar->SetTint(FVector4::White);
		TitleBar->SetUpdateWidget(GetThisPtr<CMainUI>());

	}

	auto NumberWidget = mNumber.lock();

	if (NumberWidget)
	{
		NumberWidget->SetPos(100.f, 350.f);
		NumberWidget->SetSize(40.f, 50.f); // 숫자 한개의 크기
		NumberWidget->SetNumberSize(46.8f, 68.f);
		NumberWidget->SetCount(10);
		NumberWidget->SetTexture("Number", TEXT("Number.png"));
		NumberWidget->SetNumber(0);

		auto World = mWorld.lock();

		if (World)
		{
			World->SetTimer(0.001f, true, this, &CMainUI::NumberTimer);
		}

	}

	return true;
}

CMainUI* CMainUI::Clone()
{
	return new CMainUI(*this);
}


void CMainUI::TestButtonHovered()
{
	LOG_DEBUG("Button Hovered");
}

void CMainUI::TestButtonClicked()
{
	LOG_DEBUG("Button Clicked");

	for (auto Found : mOnFuncCall)
	{
		if (Found)
		{
			Found();
		}
	}
}

void CMainUI::ProgressBarTimer()
{
	auto ProgressBar = mBar.lock();

	if (ProgressBar)
	{
		float Percent = ProgressBar->GetPercent();
		Percent += 0.1f;

		if (Percent > 1.f)
		{
			Percent = 0.f;
		}

		ProgressBar->SetPercent(Percent);


	}
}

void CMainUI::NumberTimer()
{
	auto NumberWidget = mNumber.lock();

	if (NumberWidget)
	{
		NumberWidget->AddNumber(7);
	}
}
