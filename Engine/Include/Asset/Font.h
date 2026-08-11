#pragma once

#include "Asset.h"

//폰트
//실제로 출력될 텍스트의 서식 정보를 바탕으로 레이아웃을 만드는데 사용되는 클래스

//시스템폰트, 혹은 설치한 폰트를 facename으로 폰트를 로드해서 사용할수 있다.
//불특정 다수에게 배포되는 게임 패키지 폰트컬렉션을 포함해서 배포한뒤,
//폰트 컬렉션 내부의 facename을 이용해서 폰트를 로드해서 사용한다.

class CFont :
    public CAsset
{
public:
    CFont();
    virtual ~CFont();

protected:
    ComPtr<IDWriteFactory5> mFactory = nullptr;
    ComPtr<IDWriteTextFormat> mFormat = nullptr;

public:
    bool LoadFont(ComPtr<IDWriteFactory5> Factory, const TCHAR* FontName, int Weight, float FontSize, const TCHAR* LocalName, int Stretch = DWRITE_FONT_STRETCH_NORMAL);
    ComPtr<IDWriteTextLayout> CreateLayout(const TCHAR* Text, int Length, int Width, int Height);

};

