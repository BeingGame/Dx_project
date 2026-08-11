#include "FontManager.h"

#include "../Device.h"

CFontManager::CFontManager()
{}

CFontManager::~CFontManager()
{
	mFontColorMap.clear();
	mFontCollectionMap.clear();
	mFontMap.clear();
}

bool CFontManager::LoadFont(const std::string& Name, const TCHAR* FontName, int Weight, float FontSize, const TCHAR* LocalName, int Stretch)
{
	if (!FindFont(Name).expired())
	{
		return true;
	}

	std::string CheckName = "Font_" + Name;

	std::shared_ptr<CFont> NewFont= std::make_shared<CFont>();

	NewFont->SetName(CheckName);

	if (!NewFont->LoadFont(mWriteFactory, FontName, Weight, FontSize, LocalName, Stretch))
	{
		return false;
	}

	mFontMap.insert(std::make_pair(CheckName, NewFont));

	return true;
}

bool CFontManager::LoadFontCollection(const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	if (!FindFontCollection(Name).expired())
	{
		return true;
	}

	std::string CheckName = "FontCollection_" + Name;

	std::shared_ptr<CFontCollection> NewFontCollection = std::make_shared<CFontCollection>();

	NewFontCollection->SetName(CheckName);

	if (!NewFontCollection->LoadFontCollection(mWriteFactory, FileName, PathName))
	{
		return false;
	}

	mFontCollectionMap.insert(std::make_pair(CheckName, NewFontCollection));

	return true;
}

bool CFontManager::CreateFontColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	//이미 설정한 폰트컬러가 있으면 true를 반환한다.
	ComPtr<ID2D1SolidColorBrush> Brush = FindFontColor(r, g, b, a);

	if (Brush)
	{
		return true;
	}

	//키를 만들기 위해 비트연산을 실행한다.
	unsigned int Key = a;
	Key <<= 8;
	Key |= r;
	Key <<= 8;
	Key |= g;
	Key <<= 8;
	Key |= b;

	if (FAILED(CDevice::GetInst()->Get2DRenderTarget()->CreateSolidColorBrush(D2D1::ColorF(r / 255.f, g / 255.f, b / 255.f, a / 255.f), Brush.GetAddressOf())))
	{
		return false;
	}

	mFontColorMap.insert(std::make_pair(Key, Brush));

	return true;
}

bool CFontManager::CreateFontColor(const FVector4& Color)
{
	//이미 설정한 폰트컬러가 있으면 true를 반환한다.
	ComPtr<ID2D1SolidColorBrush> Brush = FindFontColor(Color);

	if (Brush)
	{
		return true;
	}

	//Fvector4를 먼저 rgba로 나눠준다.
	unsigned char r = (unsigned char)(Color.x * 255);
	unsigned char g = (unsigned char)(Color.y * 255);
	unsigned char b = (unsigned char)(Color.z * 255);
	unsigned char a = (unsigned char)(Color.w * 255);

	//키를 만들기 위해 비트연산을 실행한다.
	unsigned int Key = a;
	Key <<= 8;
	Key |= r;
	Key <<= 8;
	Key |= g;
	Key <<= 8;
	Key |= b;

	if (FAILED(CDevice::GetInst()->Get2DRenderTarget()->CreateSolidColorBrush(D2D1::ColorF(r / 255.f, g / 255.f, b / 255.f, a / 255.f), Brush.GetAddressOf())))
	{
		return false;
	}

	mFontColorMap.insert(std::make_pair(Key, Brush));

	return true;
}

std::weak_ptr<CFont> CFontManager::FindFont(const std::string& Name)
{
	std::string CheckName = "Font_" + Name;

	auto iter = mFontMap.find(CheckName);

	if (iter == mFontMap.end())
	{
		return std::weak_ptr<CFont>();
	}

	return iter->second;
}

std::weak_ptr<CFontCollection> CFontManager::FindFontCollection(const std::string& Name)
{
	std::string CheckName = "FontCollection_" + Name;

	auto iter = mFontCollectionMap.find(CheckName);

	if (iter == mFontCollectionMap.end())
	{
		return std::weak_ptr<CFontCollection>();
	}

	return iter->second;
}

const TCHAR* CFontManager::GetFontFaceName(const std::string& Name)
{
	std::string CheckName = "FontCollection_" + Name;

	auto iter = mFontCollectionMap.find(CheckName);

	if (iter == mFontCollectionMap.end())
	{
		return nullptr;
	}

	return iter->second->GetFontFaceName();
}

ComPtr<ID2D1SolidColorBrush> CFontManager::FindFontColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{

	//키를 만들기 위해 비트연산을 실행한다.
	unsigned int Key = a;
	Key <<= 8;
	Key |= r;
	Key <<= 8;
	Key |= g;
	Key <<= 8;
	Key |= b;

	auto iter = mFontColorMap.find(Key);

	if (iter == mFontColorMap.end())
	{
		return nullptr;
	}

	return iter->second;
}

ComPtr<ID2D1SolidColorBrush> CFontManager::FindFontColor(const FVector4& Color)
{
	//Fvector4를 먼저 rgba로 나눠준다.
	unsigned char r = (unsigned char)(Color.x * 255);
	unsigned char g = (unsigned char)(Color.y * 255);
	unsigned char b = (unsigned char)(Color.z * 255);
	unsigned char a = (unsigned char)(Color.w * 255);

	//키를 만들기 위해 비트연산을 실행한다.
	unsigned int Key = a;
	Key <<= 8;
	Key |= r;
	Key <<= 8;
	Key |= g;
	Key <<= 8;
	Key |= b;

	auto iter = mFontColorMap.find(Key);

	if (iter == mFontColorMap.end())
	{
		return nullptr;
	}

	return iter->second;
}

bool CFontManager::Init()
{
	//폰트를 로드하고 사용하기위한 팩토리를 생성해준다.
	if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(mWriteFactory), (IUnknown**)mWriteFactory.GetAddressOf())))
	{
		return false;
	}

	//엔진의 기본 글꼴을 넣어준다.
	LoadFontCollection("EngineDefault", TEXT("NotoSansKR-Regular.otf"), "EngineFont");
	LoadFont("EngineDefault", GetFontFaceName("EngineDefault"), 600, 22.f, TEXT("ko"));

	CreateFontColor(FVector4::White);
	CreateFontColor(FVector4::Black);
	CreateFontColor(FVector4::Red);
	CreateFontColor(FVector4::Blue);
	CreateFontColor(FVector4::Green);

	return true;
}
