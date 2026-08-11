#pragma once

#include "World/WidgetContainer.h"

class CStartWidget :
    public CWidgetContainer
{

public:
	CStartWidget();
	CStartWidget(const CStartWidget& src);
	virtual ~CStartWidget();

protected:
	std::weak_ptr<class CTextBlock> mMainText;
	std::weak_ptr<class CTextBlock> mEditorText;

public:
	virtual bool Init();
	virtual CStartWidget* Clone();
};

