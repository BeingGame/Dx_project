#include "FontCollection.h"
#include "PathManager.h"

CFontCollection::CFontCollection()
{
	mType = EAssetType::FontCollection;
}

CFontCollection::~CFontCollection()
{}

bool CFontCollection::LoadFontCollection(ComPtr<IDWriteFactory5> Factory, const TCHAR* FileName, const std::string& PathName)
{
	TCHAR FullPath[MAX_PATH] = {};

	const TCHAR* Path = CPathManager::FindPath(PathName);

	lstrcpy(FullPath, Path);
	lstrcat(FullPath, FileName);

	ComPtr<IDWriteFontFile> FontFile = nullptr;

	//파일경로를 이용해 폰트 파일 참조를 위한 객체를 생성한다.
	if (FAILED(Factory->CreateFontFileReference(FullPath, nullptr, FontFile.GetAddressOf())))
	{
		return false;
	}

	//폰트 세트 빌더를 생성한다.
	//폰트를 하나로 묶어주는 역할
	ComPtr<IDWriteFontSetBuilder1> FontBuilder = nullptr;

	if (FAILED(Factory->CreateFontSetBuilder(FontBuilder.GetAddressOf())))
	{
		return false;
	}

	FontBuilder->AddFontFile(FontFile.Get());

	ComPtr<IDWriteFontSet> FontSet = nullptr;

	if (FAILED(FontBuilder->CreateFontSet(FontSet.GetAddressOf())))
	{
		return false;
	}

	//실제 폰트를 사용하기 위해서 폰트 세트를 만들어 준뒤
	//폰트의 실제 이름을 가져온다.(facename)
	if (FAILED(Factory->CreateFontCollectionFromFontSet(FontSet.Get(), mCollection.GetAddressOf())))
	{
		return false;
	}

	//폰트 패밀리의 숫자
	UINT32 Count = mCollection->GetFontFamilyCount();

	ComPtr<IDWriteFontFamily> Family = nullptr;

	if (FAILED(mCollection->GetFontFamily(0, Family.GetAddressOf())))
	{
		return false;
	}

	//로컬 이름을 가져온다.
	ComPtr<IDWriteLocalizedStrings> LocalName = nullptr;

	if (FAILED(Family->GetFamilyNames(LocalName.GetAddressOf())))
	{
		return false;
	}
	
	//로컬 이름을 통해 실제 폰트의 이름을 가져와준다.
	if (FAILED(LocalName->GetString(0, mFontFaceName, 128)))
	{
		return false;
	}

	return true;
}
