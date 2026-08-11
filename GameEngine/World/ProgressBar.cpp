#include "ProgressBar.h"

#include "../Asset/AssetManager.h"
#include "../Asset/TextureManager.h"
#include "../Asset/Texture.h"

#include "../Asset/CBufferUIDefault.h"

CProgressBar::CProgressBar()
{}

CProgressBar::CProgressBar(const CProgressBar& src)
	:CWidget(src)
{}

CProgressBar::~CProgressBar()
{}

void CProgressBar::SetPercent(float Percent)
{
	mPercent = Percent;

	if (mPercent < 0.f)
	{
		mPercent = 0.f;
	}
	else if (mPercent > 1.f)
	{
		mPercent = 1.f;
	}
}

void CProgressBar::SetMinValue(float Min)
{
	if (Min >= mMaxValue)
	{
		return;
	}

	mMinValue = Min;
}

void CProgressBar::SetMaxValue(float Max)
{
	if (Max <= mMinValue)
	{
		return;
	}

	mMaxValue = Max;
}

void CProgressBar::SetDir(EProgressBarDir Dir)
{
	mBarDir = Dir;
}

void CProgressBar::SetValue(float Value)
{
	//Min 기준을 0으로 맞춰주기 위해
	//ex) Value : 100 MinValue :50 MaxValue : 150 -> Value - MinValue = 50
	//max - min = 100 -> Value - MinValue = 50
	Value = Value - mMinValue;

	SetPercent(Value / mMaxValue - mMinValue);
}

void CProgressBar::SetChild(const std::weak_ptr<CWidget>& Child)
{
	if (!Child.expired())
	{
		if (mChild)
		{
			mChild->Detach();
		}

		mChild = Child.lock();
		mChild->SetParent(GetThisPtr<CWidget>());
		mChild->SetSize(mSize);
		mChild->SetUIManager(mUIManager);
		mChild->SetWorld(mWorld);
	}
	else
	{
		if (mChild)
		{
			mChild->Detach();
		}

		mChild.reset();
	}
}

void CProgressBar::RemoveChild(std::shared_ptr<CWidget> Child)
{
	if (mChild)
	{
		mChild.reset();
	}
}

void CProgressBar::SetParentAll()
{
	if (mChild)
	{
		mChild->SetParent(GetThisPtr<CWidget>());

		mChild->SetParentAll();
	}

}

void CProgressBar::SetOpacityAll(float Opacity)
{
	CWidget::SetOpacityAll(Opacity);

	for (int i = 0; i < EProgressBarImageType::End; ++i)
	{
		mBrush[i].Tint.w = Opacity;
	}

	if (mChild)
	{
		mChild->SetOpacityAll(Opacity);
	}
}

bool CProgressBar::SetTexture(EProgressBarImageType::Type Type, const std::weak_ptr<class CTexture> Texture)
{
	mBrush[Type].Texture = Texture;

	return true;
}

bool CProgressBar::SetTexture(EProgressBarImageType::Type Type, const std::string& Name)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (!TextureMgr)
	{
		return false;

	}

	mBrush[Type].Texture = TextureMgr->FindTexture(Name);

	return true;
}

bool CProgressBar::SetTexture(EProgressBarImageType::Type Type, const std::string& Name, const TCHAR* FileName, const std::string& PathName)
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

	mBrush[Type].Texture = TextureMgr->FindTexture(Name);

	return true;
}

bool CProgressBar::SetTexture(EProgressBarImageType::Type Type, const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName)
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

	mBrush[Type].Texture = TextureMgr->FindTexture(Name);

	return true;
}

void CProgressBar::SetTint(EProgressBarImageType::Type Type, const FVector4& Color)
{
	mBrush[Type].Tint = Color;
}

void CProgressBar::SetTint(EProgressBarImageType::Type Type, float r, float g, float b, float a)
{
	mBrush[Type].Tint = FVector4(r, g, b, a);
}

void CProgressBar::SetBrushAnimation(EProgressBarImageType::Type Type, bool AnimationEnable)
{
	mBrush[Type].AnimationEnable = AnimationEnable;
}

void CProgressBar::AddBrushFrame(EProgressBarImageType::Type Type, const FVector2& Start, const FVector2& Size)
{
	FTextureFrame Frame;

	Frame.Start = Start;
	Frame.Size = Size;

	mBrush[Type].AnimationFrames.push_back(Frame);

	mBrush[Type].FrameTime = mBrush[Type].PlayTime / mBrush[Type].AnimationFrames.size();
}

void CProgressBar::AddBrushFrame(EProgressBarImageType::Type Type, float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(Type, FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CProgressBar::AddBrushFrame(EProgressBarImageType::Type Type, int Count, const FVector2& Start, const FVector2& Size)
{
	for (int i = 0; i < Count; ++i)
	{
		AddBrushFrame(Type, Start, Size);
	}
}

void CProgressBar::AddBrushFrame(EProgressBarImageType::Type Type, int Count, float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(Type, Count, FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CProgressBar::SetCurrentFrame(EProgressBarImageType::Type Type, int Frame)
{
	mBrush[Type].Frame = Frame;
}

void CProgressBar::SetAnimationPlayTime(EProgressBarImageType::Type Type, float PlayTime)
{
	mBrush[Type].PlayTime = PlayTime;

	mBrush[Type].FrameTime = mBrush[Type].PlayTime / mBrush[Type].AnimationFrames.size();
}

void CProgressBar::SetAnimationPlayRate(EProgressBarImageType::Type Type, float PlayRate)
{
	mBrush[Type].PlayRate = PlayRate;
}

bool CProgressBar::Init()
{
	CWidget::Init();

	mCBufferUIDefault->SetBrushPercentEnable(true);

	return true;
}

void CProgressBar::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	for (int i = 0; i < EProgressBarImageType::End; ++i)
	{
		mBrush[i].PlayAnimation(DeltaTime);
	}

	if (mChild)
	{
		mChild->Update(DeltaTime);
	}
}

void CProgressBar::Render()
{
	CWidget::Render();

	mCBufferUIDefault->SetBrushPercent(mPercent);
	mCBufferUIDefault->SetBrushBarDir((int)mBarDir);

	//백그라운드 이미지보다 막대기 이미지가 더 위로 올라와서 렌더링되어야한다.
	for (int i = EProgressBarImageType::Back; i >= EProgressBarImageType::Fill; --i)
	{
		if (i == EProgressBarImageType::Fill)
		{
			mCBufferUIDefault->SetBrushPercentEnable(true);
		}
		else
		{
			mCBufferUIDefault->SetBrushPercentEnable(false);
		}

		RenderBrush(mBrush[i], mRenderPos, mSize);
	}

	if (mChild)
	{
		mChild->Render();
	}

}

bool CProgressBar::CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos)
{
	if (mChild)
	{
		if (mChild->CollisionMouse(Result, MousePos))
		{
			return true;
		}
	}

	return CWidget::CollisionMouse(Result, MousePos);
}

CProgressBar* CProgressBar::Clone()
{
	return new CProgressBar(*this);
}
