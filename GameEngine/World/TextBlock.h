
#include "Widget.h"

#include "../Asset/Font.h"

enum class ETextAlignH
{
	Left,
	Center,
	Right
};

enum class ETextAlignV
{
	Top,
	Middle,
	Bottom
};

class CTextBlock
	:public CWidget
{
public:
	CTextBlock();
	CTextBlock(const CTextBlock& src);
	virtual ~CTextBlock();

protected:
	//텍스트를 출력할때 사용할 렌더타겟과 레이아웃 정보 변수
	ComPtr<ID2D1RenderTarget> mRenderTarget;
	ComPtr<IDWriteTextLayout> mLayout;

	//텍스트 블록의 실제 텍스트
	std::wstring mText;
	std::weak_ptr<CFont> mFont;

	//텍스트의 컬러값
	ComPtr<ID2D1SolidColorBrush> mTextColor = nullptr;
	//텍스트의 투명도를 활성화 체크할 변수
	bool mTransparency = false;
	float mOpacity = 1.f;

	//텍스트의 그림자
	bool mShadow = false;
	ComPtr<ID2D1SolidColorBrush> mShadowColor = nullptr;
	bool mShadowTransparency = false;
	float mShadowOpacity = 1.f;
	FVector2 mShadowOffset = FVector2(1.f, 1.f);

	float mFontSize = 22.f;
	ETextAlignH mAlignH = ETextAlignH::Left;
	ETextAlignV mAlignV = ETextAlignV::Top;

public:
	const std::wstring& GetText() const
	{
		return mText;
	}

	const TCHAR* GetTextCstr() const
	{
		return mText.c_str();
	}

	int GetTextLength() const
	{
		return (int)mText.length();
	}

	float GetFontSize() const
	{
		return mFontSize;
	}

	ETextAlignH GetAlignH() const
	{
		return mAlignH;
	}

	ETextAlignV GetAlignV() const
	{
		return mAlignV;
	}

public:
	virtual void SetSize(const FVector3& Size);
	virtual void SetSize(float x, float y);
	void SetFont(const std::string& FontName);
	//SetText는 필요하다면 각 자료형 타입별로 만들어서 데이터를 받아서 텍스트를 설정해도 된다.
	void SetText(const TCHAR* Text);
	void AddText(const TCHAR* Text);
	void PopBack();
	void ClearText();
	void SetFontSize(float Size);
	void SetAlignH(ETextAlignH AlignH);
	void SetAlignV(ETextAlignV AlignV);

	void EnableTransparency(bool Enable);
	void SetOpacity(float Opacity);
	void SetTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void SetTextColor(const FVector4& Color);


	void EnableShadow(bool Shadow);
	void EnableShadowTransparency(bool Enable);
	void SetShadowOpacity(float Opacity);
	void SetShadowTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void SetShadowTextColor(const FVector4& Color);
	void SetShadowOffset(const FVector2& Offset);
	void SetShadowOffset(float x, float y);

private:
	//레이아웃 정보를 생성하는 함수
	void CreateTextLayout();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos);
	
	virtual CTextBlock* Clone();
};