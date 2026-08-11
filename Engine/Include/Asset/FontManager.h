#pragma once

#include "AssetSubManager.h"
#include "Font.h"
#include "FontCollection.h"

class CFontManager :
	public CAssetSubManager
{
public:
	CFontManager();
	virtual ~CFontManager();

private:
	ComPtr<IDWriteFactory5> mWriteFactory = nullptr;

	//폰트, 폰트매니저를 관리할 맵 변수
	std::unordered_map<std::string, std::shared_ptr<CFont>> mFontMap;
	std::unordered_map<std::string, std::shared_ptr<CFontCollection>> mFontCollectionMap;

	//폰트의 컬러를 적용하기 위한 비트 연산자를 사용한 컬러
	std::unordered_map<unsigned int, ComPtr<ID2D1SolidColorBrush>> mFontColorMap;

public:
	bool LoadFont(const std::string& Name, const TCHAR* FontName, int Weight, float FontSize, const TCHAR* LocalName, int Stretch = DWRITE_FONT_STRETCH_NORMAL);
	bool LoadFontCollection(const std::string& Name, const TCHAR* FileName, const std::string& PathName);
	bool CreateFontColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	bool CreateFontColor(const FVector4& Color);

	std::weak_ptr<CFont> FindFont(const std::string& Name);
	std::weak_ptr<CFontCollection> FindFontCollection(const std::string& Name);
	const TCHAR* GetFontFaceName(const std::string& Name);

	ComPtr<ID2D1SolidColorBrush> FindFontColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	ComPtr<ID2D1SolidColorBrush> FindFontColor(const FVector4& Color);

public:
	virtual bool Init();



};

