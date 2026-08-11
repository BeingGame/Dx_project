#pragma once

#include "../EngineInfo.h"


//서브 관리자를 만들기 위한 추상클래스

class CAssetSubManager
{
public:
	CAssetSubManager();
	virtual ~CAssetSubManager();
	
public:
	virtual bool Init() = 0;
	virtual void Update()
	{

	}


};

