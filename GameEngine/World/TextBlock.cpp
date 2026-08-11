#include "TextBlock.h"

#include "../Asset/AssetManager.h"
#include "../Asset/FontManager.h"

#include "../Device.h"

CTextBlock::CTextBlock()
{}

CTextBlock::CTextBlock(const CTextBlock& src)
	:CWidget(src)
{

	mRenderTarget = src.mRenderTarget;
	mText = src.mText;
	mFont = src.mFont;
	mTextColor = src.mTextColor;
	mTransparency = src.mTransparency;
	mOpacity = src.mOpacity;

	mShadow = src.mShadow;
	mShadowColor = src.mShadowColor;
	mShadowTransparency = src.mShadowTransparency;
	mShadowOpacity = src.mShadowOpacity;
	mShadowOffset = src.mShadowOffset;

	mFontSize = src.mFontSize;
	mAlignH = src.mAlignH;
	mAlignV = src.mAlignV;

	mLayout = nullptr;

	CreateTextLayout();
}

CTextBlock::~CTextBlock()
{}

void CTextBlock::SetSize(const FVector3& Size)
{
	CWidget::SetSize(Size);

	CreateTextLayout();
}

void CTextBlock::SetSize(float x, float y)
{
	CWidget::SetSize(x, y);

	CreateTextLayout();
}

void CTextBlock::SetFont(const std::string& FontName)
{
	auto FontMgr = CAssetManager::GetInst()->GetSubManager<CFontManager>(EAssetType::Font);

	if (FontMgr)
	{
		mFont = FontMgr->FindFont(FontName);
	}

	CreateTextLayout();
}

void CTextBlock::SetText(const TCHAR* Text)
{
	mText = Text;

	CreateTextLayout();
}

void CTextBlock::AddText(const TCHAR* Text)
{
	mText += Text;

	CreateTextLayout();
}

void CTextBlock::PopBack()
{
	mText.pop_back();

	CreateTextLayout();
}

void CTextBlock::ClearText()
{
	mText.clear();

	CreateTextLayout();
}

void CTextBlock::SetFontSize(float Size)
{
	//레이아웃이 존재하면 폰트 크기를 레이아웃에 적용시켜준다.
	//없으면 새로운 레이아웃을 생성해준다.

	mFontSize = Size;

	if (!mLayout)
	{
		CreateTextLayout();
	}
	else
	{
		DWRITE_TEXT_RANGE Range = {};

		Range.startPosition = 0;
		Range.length = (UINT32)mText.length();

		mLayout->SetFontSize(mFontSize, Range);
	}
}

void CTextBlock::SetAlignH(ETextAlignH AlignH)
{
	mAlignH = AlignH;

	if (!mLayout)
	{
		CreateTextLayout();
	}
	else
	{
		switch (mAlignH)
		{
		case ETextAlignH::Left:
			mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			break;
		case ETextAlignH::Center:
			mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			break;
		case ETextAlignH::Right:
			mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
			break;
		default:
			break;
		}
	}
}

void CTextBlock::SetAlignV(ETextAlignV AlignV)
{
	mAlignV = AlignV;

	if (!mLayout)
	{
		CreateTextLayout();
	}
	else
	{
		switch (mAlignV)
		{
		case ETextAlignV::Top:
			mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			break;
		case ETextAlignV::Middle:
			mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			break;
		case ETextAlignV::Bottom:
			mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
			break;
		default:
			break;
		}
	}
}

void CTextBlock::EnableTransparency(bool Enable)
{
	mTransparency = Enable;
}

void CTextBlock::SetOpacity(float Opacity)
{
	mOpacity = Opacity;
}

void CTextBlock::SetTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	auto FontMgr = CAssetManager::GetInst()->GetSubManager<CFontManager>(EAssetType::Font);

	if (FontMgr)
	{
		if (FontMgr->CreateFontColor(r, g, b, a))
		{
			mTextColor = FontMgr->FindFontColor(r, g, b, a);
		}
	}
}

void CTextBlock::SetTextColor(const FVector4& Color)
{
	auto FontMgr = CAssetManager::GetInst()->GetSubManager<CFontManager>(EAssetType::Font);

	if (FontMgr)
	{
		if (FontMgr->CreateFontColor(Color))
		{
			mTextColor = FontMgr->FindFontColor(Color);
		}
	}
}

void CTextBlock::EnableShadow(bool Shadow)
{
	mShadow = Shadow;
}

void CTextBlock::EnableShadowTransparency(bool Enable)
{
	mShadowTransparency = Enable;
}

void CTextBlock::SetShadowOpacity(float Opacity)
{
	mShadowOpacity = Opacity;
}

void CTextBlock::SetShadowTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	auto FontMgr = CAssetManager::GetInst()->GetSubManager<CFontManager>(EAssetType::Font);

	if (FontMgr)
	{
		if (FontMgr->CreateFontColor(r, g, b, a))
		{
			mShadowColor = FontMgr->FindFontColor(r, g, b, a);
		}
	}
}

void CTextBlock::SetShadowTextColor(const FVector4& Color)
{
	auto FontMgr = CAssetManager::GetInst()->GetSubManager<CFontManager>(EAssetType::Font);

	if (FontMgr)
	{
		if (FontMgr->CreateFontColor(Color))
		{
			mShadowColor = FontMgr->FindFontColor(Color);
		}
	}
}

void CTextBlock::SetShadowOffset(const FVector2& Offset)
{
	mShadowOffset = Offset;
}

void CTextBlock::SetShadowOffset(float x, float y)
{
	mShadowOffset = FVector2(x, y);
}

void CTextBlock::CreateTextLayout()
{
	//레이아웃 정보는 한번 만들어지면 고정된 정보로 계속해서 적용된다.
	//따라서 폰트크기, 혹은 텍스트, 텍스트길이등이 변경되면 새로운 레이아웃을 생성해줘야된다.

	//이전 레이아웃 정보를 해제한다.
	mLayout.Reset();

	auto Font = mFont.lock();

	mLayout = Font->CreateLayout(GetTextCstr(), GetTextLength(), (int)mSize.x, (int)mSize.y);

	if (!mLayout)
	{
		return;
	}

	//텍스트를 정렬해준다.
	DWRITE_TEXT_RANGE Range = {};

	Range.startPosition = 0;
	Range.length = (UINT32)mText.length();

	mLayout->SetFontSize(mFontSize, Range);

	switch (mAlignH)
	{
	case ETextAlignH::Left:
		mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		break;
	case ETextAlignH::Center:
		mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		break;
	case ETextAlignH::Right:
		mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		break;
	default:
		break;
	}

	switch (mAlignV)
	{
	case ETextAlignV::Top:
		mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		break;
	case ETextAlignV::Middle:
		mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		break;
	case ETextAlignV::Bottom:
		mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		break;
	default:
		break;
	}

}

bool CTextBlock::Init()
{
	CWidget::Init();

	mRenderTarget = CDevice::GetInst()->Get2DRenderTarget();

	//기본적으로 적용될 폰트와 컬러를 넣어준다.
	SetFont("EngineDefault");
	SetText(TEXT("Text Block"));
	SetTextColor(FVector4::Black);
	SetShadowTextColor(FVector4::White);

	return true;
}

void CTextBlock::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);
}

void CTextBlock::Render()
{
	CWidget::Render();

	//텍스트 출력하기 위해 RenderTarget에서 BeginDraw호출
	mRenderTarget->BeginDraw();

	//텍스트 출력할땐 원본 텍스트가 가려지지 않게 먼저 그림자 텍스처를 렌더링한다.
	if (mShadow)
	{
		D2D1_POINT_2F ShadowPoint;

		ShadowPoint.x = mRenderPos.x + mShadowOffset.x;
		ShadowPoint.y = mRenderPos.y + mShadowOffset.y;

		if (mShadowTransparency)
		{
			mShadowColor->SetOpacity(mShadowOpacity);
		}
		else
		{
			mShadowColor->SetOpacity(1.f);
		}

		//마지막 인자는 플래그 옵션을 설정할수 있는 인자
		/*
		D2D1_DRAW_TEXT_OPTIONS_NONE : 기본값, 레이아웃 사각형 밖으로 텍스트가 나가도 클리핑 하지 않는다.
		D2D1_DRAW_TEXT_OPTIONS_NO_SNAP : 픽셀 스냅을 끄고, 글자를 픽셀단위가 아닌 부드럽게 렌더링을 한다.
		D2D1_DRAW_TEXT_OPTIONS_CLIP : 텍스트가 레이아웃의 사각형 밖으로 렌더링 되지 않는다.
		D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT, : 폰트가 지원하는 폰트커를 가져와서 사용한
		D2D1_DRAW_TEXT_OPTIONS_DISABLE_COLOR_BITMAP_SNAPPING : 비트맵의 픽셀 스냅을 끈다.

		*/
		mRenderTarget->DrawTextLayout(ShadowPoint, mLayout.Get(), mShadowColor.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
	}

	if (mTransparency)
	{
		mTextColor->SetOpacity(mOpacity);
	}
	else
	{
		mTextColor->SetOpacity(1.f);
	}

	D2D1_POINT_2F Point;

	Point.x = mRenderPos.x;
	Point.y = mRenderPos.y;

	mRenderTarget->DrawTextLayout(Point, mLayout.Get(), mTextColor.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);

	mRenderTarget->EndDraw();
}

bool CTextBlock::CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos)
{
	return false;
}

CTextBlock* CTextBlock::Clone()
{
	return new CTextBlock(*this);
}
