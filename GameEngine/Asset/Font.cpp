#include "Font.h"

CFont::CFont()
{
	mType = EAssetType::Font;
}

CFont::~CFont()
{}

bool CFont::LoadFont(ComPtr<IDWriteFactory5> Factory, const TCHAR* FontName, int Weight, float FontSize, const TCHAR* LocalName, int Stretch)
{
	mFactory = Factory;

	//팩토리를 기반으로 텍스트 포맷 정보를 만들어준다.
	//1. 폰트 이름
	//2. 폰트 컬렉션 -> nullptr: 윈도우가 기본적으로 제공하는 폰트를 찾아서 사용한다. 값을 넣으면 게임폴더에서 폰트를 읽어온다.
	//3.글자의 굵기 설정
	//4.글자의 기울임 설정
	//5.글자의 장평 설정(가로, 세로길이를 늘리거나 줄일수 있다)
	//6.폰트의 크기
	//7.폰트의 로케일 설정
	if (FAILED(mFactory->CreateTextFormat(FontName, nullptr, (DWRITE_FONT_WEIGHT)Weight,DWRITE_FONT_STYLE_NORMAL, (DWRITE_FONT_STRETCH)Stretch,FontSize,LocalName,mFormat.GetAddressOf())))
	{
		return false;
	}

	return true;
}

ComPtr<IDWriteTextLayout> CFont::CreateLayout(const TCHAR* Text, int Length, int Width, int Height)
{
	ComPtr<IDWriteTextLayout> Layout;
	//화면에 출력될 텍스트 정보를 만드는 함수

	//1. 화면에 출력될 텍스트
	//2. 화면에 출력될 텍스트 길이
	//3. 텍스트에 적용될 텍스트 서식 정보
	//4. 5. 텍스트가 표시될 영역의 최대 가로, 세로 길이 (클리핑) 기준으로 사용된다.
	if (FAILED(mFactory->CreateTextLayout(Text, Length, mFormat.Get(), (float)Width, (float)Height, Layout.GetAddressOf())))
	{
		return nullptr;
	}

	return Layout;
}
