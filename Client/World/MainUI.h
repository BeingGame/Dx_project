#pragma once

#include "World/WidgetContainer.h"

class CMainUI :
	public CWidgetContainer
{

public:
	CMainUI();
	CMainUI(const CMainUI& src);
	virtual ~CMainUI();

protected:
	std::weak_ptr<class CButton> mTestButton;
	std::weak_ptr<class CTextBlock> mText;
	std::weak_ptr<class CProgressBar> mBar;
	std::weak_ptr<class CTitleBar> mTitleBar;
	std::weak_ptr<class CNumberWidget> mNumber;


	std::vector<std::function<void()>> mOnFuncCall;

public:
	virtual bool Init();
	virtual CMainUI* Clone();


public:
	void TestButtonHovered();
	void TestButtonClicked();

public:
	void ProgressBarTimer();
	void NumberTimer();

public:
	template<typename T>
	void SetOnFunc(T* Object, void(T::* Func)())
	{
		std::function<void()> BindFunc = std::bind(Func, Object);

		mOnFuncCall.push_back(BindFunc);
	}

};

