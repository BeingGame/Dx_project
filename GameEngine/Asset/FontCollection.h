#pragma once

#include "Asset.h"

//폰트 컬렉션
//폰트 여러개를 하나로 묶어서 관리하는 주체
//윈도우가 제공하는 폰트 컬렉션, 우리가 직접 사용하기 위해서 다운로드 받는 폰트 컬렉션

//폰트 컬렉션을 로드했을때 폰트의 이름(facename)을 저장해서
//폰트를 불러오는 작업을 실행한다.

class CFontCollection :
	public CAsset
{
public:
	CFontCollection();
	virtual ~CFontCollection();

private:
	ComPtr<IDWriteFontCollection1> mCollection = nullptr;
	TCHAR mFontFaceName[128] = {};

public:
	const TCHAR* GetFontFaceName() const
	{
		return mFontFaceName;
	}

public:
	bool LoadFontCollection(ComPtr<IDWriteFactory5> Factory, const TCHAR* FileName, const std::string& PathName);

};

