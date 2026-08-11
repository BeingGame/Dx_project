#pragma once

#include "Image.h"

//마우스가 사용할 텍스처, 애니메이션을 적용할 시스템을 만들어준다.

class CMouseWidget :
    public CImage
{
public:
	CMouseWidget();
	CMouseWidget(const CMouseWidget& src);
	virtual ~CMouseWidget();

public:
	virtual void Update(float DeltaTime);
	virtual CMouseWidget* Clone();

};

