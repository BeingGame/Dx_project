#include "NumberWidget.h"

#include "../Asset/AssetManager.h"
#include "../Asset/TextureManager.h"
#include "../Asset/Texture.h"

CNumberWidget::CNumberWidget()
{}

CNumberWidget::CNumberWidget(const CNumberWidget& src)
	:CWidget(src)
{}

CNumberWidget::~CNumberWidget()
{}


bool CNumberWidget::SetTexture(const std::weak_ptr<class CTexture> Texture)
{
	mBrush.Texture = Texture;

	return true;
}

bool CNumberWidget::SetTexture(const std::string& Name)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (!TextureMgr)
	{
		return false;

	}

	mBrush.Texture = TextureMgr->FindTexture(Name);

	return true;
}

bool CNumberWidget::SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (!TextureMgr)
	{
		return false;

	}

	if (!TextureMgr->LoadTexture(Name, FileName, PathName))
	{
		return false;
	}

	mBrush.Texture = TextureMgr->FindTexture(Name);

	return true;
}

bool CNumberWidget::SetTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (!TextureMgr)
	{
		return false;

	}

	if (!TextureMgr->LoadTexture(Name, FileName, PathName))
	{
		return false;
	}

	mBrush.Texture = TextureMgr->FindTexture(Name);

	return true;
}

void CNumberWidget::SetTint(const FVector4& Color)
{
	mBrush.Tint = Color;
}

void CNumberWidget::SetTint(float r, float g, float b, float a)
{
	mBrush.Tint = FVector4(r, g, b, a);
}

void CNumberWidget::SetBrushAnimation(bool AnimationEnable)
{
	mBrush.AnimationEnable = AnimationEnable;
}

void CNumberWidget::AddBrushFrame(const FVector2& Start, const FVector2& Size)
{
	FTextureFrame Frame;

	Frame.Start = Start;
	Frame.Size = Size;

	mBrush.AnimationFrames.push_back(Frame);

	mBrush.FrameTime = mBrush.PlayTime / mBrush.AnimationFrames.size();
}

void CNumberWidget::AddBrushFrame(float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CNumberWidget::AddBrushFrame(int Count, const FVector2& Start, const FVector2& Size)
{
	for (int i = 0; i < Count; ++i)
	{
		AddBrushFrame(Start, Size);
	}
}

void CNumberWidget::AddBrushFrame(int Count, float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(Count, FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CNumberWidget::SetCurrentFrame(int Frame)
{
	mBrush.Frame = Frame;
}

void CNumberWidget::SetAnimationPlayTime(float PlayTime)
{
	mBrush.PlayTime = PlayTime;

	mBrush.FrameTime = mBrush.PlayTime / mBrush.AnimationFrames.size();
}

void CNumberWidget::SetAnimationPlayRate(float PlayRate)
{
	mBrush.PlayRate = PlayRate;
}

void CNumberWidget::SetAnimationType(EAnimation2DTextureType Type)
{
	mBrush.AnimationType = Type;
}

bool CNumberWidget::Init()
{
	CWidget::Init();

	mNumberSize = FVector2(10.f, 10.f);

	return true;
}

void CNumberWidget::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);
}

void CNumberWidget::Render()
{
	CWidget::Render();

	//음수인지 양수인지 구분한다.
	bool Minus = false;

	if (mNumber < 0.f)
	{
		Minus = true;
	}

	//정수 부분을 구해준다.
	int Number = (int)mNumber;

	//소수 부분을 구해준다.
	float Decimal = mNumber - Number;

	//소수점 부분을 구해주기 위해서
	//소수점 부분을 *n 만큼 해줘서 소수를 정수로 변경해준다.
	int Value = 1;

	for (int i = 0; i < mDecimalCount; ++i)
	{
		Value *= 10;
	}

	Decimal *= Value;

	int DecimalNumber = (int)Decimal;

	//먼저 정수부분의 자릿수를 구해준다.
	std::stack<int> NumberStack;

	mNumberArray.clear();
	mDecimalNumberArray.clear();

	while (Number > 0)
	{
		NumberStack.push(Number % 10);
		Number /= 10;
	}

	while (NumberStack.size() < mCount)
	{
		NumberStack.push(0);
	}

	//스택의 들어간 데이터들을 array에 넣어준다.
	while (!NumberStack.empty())
	{
		mNumberArray.push_back(NumberStack.top());
		NumberStack.pop();
	}

	if (mNumberArray.size() < 0)
	{
		mNumberArray.push_back(0);
	}

	//소수점 자리
	while (DecimalNumber > 0)
	{
		NumberStack.push(DecimalNumber % 10);
		DecimalNumber /= 10;
	}

	while (!NumberStack.empty())
	{
		mDecimalNumberArray.push_back(NumberStack.top());
		NumberStack.pop();
	}

	FVector3 WidgetSize;

	WidgetSize.y = mSize.y;
	WidgetSize.x = mSize.x * mNumberArray.size() + mSize.x * mDecimalNumberArray.size();

	//소수점이나 부호가 표현되면 한칸만큼 더 늘려준다.
	if (!mDecimalNumberArray.empty())
	{
		WidgetSize.x += mSize.x;
	}

	if (Minus)
	{
		WidgetSize.x += mSize.x;
	}

	mNumberRenderPos = mRenderPos;

	//넘버위젯의 피벗은 렌더링할 숫자의 중앙을 기준으로 만들어준다.
	mNumberRenderPos.y -= mSize.y * 0.5f;
	mNumberRenderPos.x -= WidgetSize.x * 0.5f;

	if (Minus)
	{
		//-를 출력해준다.
		FVector2 FrameStart;
		FrameStart.x = mNumberSize.x * 11.f;
		FrameStart.y = 0.f;

		RenderBrush(mBrush, mNumberRenderPos, mSize, FrameStart, mNumberSize);

		mNumberRenderPos.x += mSize.x;
	}


	RenderNumber();
}

bool CNumberWidget::CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos)
{
	return false;
}

CNumberWidget* CNumberWidget::Clone()
{
	return new CNumberWidget(*this);
}

void CNumberWidget::RenderNumber()
{
	//숫자를 출력
	FVector2 FrameStart;

	size_t Size = mNumberArray.size();

	for (int i = 0; i < Size; ++i)
	{
		//프레임의 시작위치는 넘버의 사이즈 * NumberArray[i]
		//FVector2(10.f,10.f) * 
		FrameStart.x = mNumberSize.x * mNumberArray[i];
		FrameStart.y = 0.f;

		RenderBrush(mBrush, mNumberRenderPos, mSize, FrameStart, mNumberSize);

		mNumberRenderPos.x += mSize.x;
	}

	if (!mDecimalNumberArray.empty())
	{
		FrameStart.x = mNumberSize.x * 10.f;
		FrameStart.y = 0.f;

		RenderBrush(mBrush, mNumberRenderPos, mSize, FrameStart, mNumberSize);

		Size = mDecimalNumberArray.size();

		for (int i = 0; i < Size; ++i)
		{
			//프레임의 시작위치는 넘버의 사이즈 * NumberArray[i]
			//FVector2(10.f,10.f) * 
			FrameStart.x = mNumberSize.x * mDecimalNumberArray[i];
			FrameStart.y = 0.f;

			RenderBrush(mBrush, mNumberRenderPos, mSize, FrameStart, mNumberSize);

			mNumberRenderPos.x += mSize.x;
		}
	}

}