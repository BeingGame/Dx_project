#include "TitleBar.h"

#include "../Asset/AssetManager.h"
#include "../Asset/TextureManager.h"
#include "../Asset/Texture.h"

#include "World.h"
#include "Input.h"

CTitleBar::CTitleBar()
{}

CTitleBar::CTitleBar(const CTitleBar& src)
	:CWidget(src)
{}

CTitleBar::~CTitleBar()
{}

void CTitleBar::SetUpdateWidget(const std::weak_ptr<CWidget>& Widget)
{
	mUpdateWidget = Widget;
}

void CTitleBar::SetChild(const std::weak_ptr<CWidget>& Child)
{
	if (!Child.expired())
	{
		if (mChild)
		{
			mChild->Detach();
		}

		mChild = Child.lock();

		mChild->SetSize(mSize);
		mChild->SetParent(GetThisPtr<CWidget>());
		mChild->SetWorld(mWorld);
		mChild->SetUIManager(mUIManager);
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

void CTitleBar::RemoveChild(std::shared_ptr<CWidget> Child)
{
	if (mChild)
	{
		mChild.reset();
	}
}

void CTitleBar::SetParentAll()
{
	if (mChild)
	{
		mChild->SetParent(GetThisPtr<CWidget>());

		mChild->SetParentAll();
	}

}

void CTitleBar::SetOpacityAll(float Opacity)
{
	CWidget::SetOpacityAll(Opacity);

	if (mChild)
	{
		mChild->SetOpacityAll(Opacity);
	}
}

bool CTitleBar::SetTexture(const std::weak_ptr<class CTexture> Texture)
{
	mBrush.Texture = Texture;

	return true;
}

bool CTitleBar::SetTexture(const std::string& Name)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (!TextureMgr)
	{
		return false;

	}

	mBrush.Texture = TextureMgr->FindTexture(Name);

	return true;
}

bool CTitleBar::SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName)
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

bool CTitleBar::SetTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName)
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

void CTitleBar::SetTint(const FVector4& Color)
{
	mBrush.Tint = Color;
}

void CTitleBar::SetTint(float r, float g, float b, float a)
{
	mBrush.Tint = FVector4(r, g, b, a);
}

void CTitleBar::SetBrushAnimation(bool AnimationEnable)
{
	mBrush.AnimationEnable = AnimationEnable;
}

void CTitleBar::AddBrushFrame(const FVector2& Start, const FVector2& Size)
{
	FTextureFrame Frame;

	Frame.Start = Start;
	Frame.Size = Size;

	mBrush.AnimationFrames.push_back(Frame);

	mBrush.FrameTime = mBrush.PlayTime / mBrush.AnimationFrames.size();
}

void CTitleBar::AddBrushFrame(float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CTitleBar::AddBrushFrame(int Count, const FVector2& Start, const FVector2& Size)
{
	for (int i = 0; i < Count; ++i)
	{
		AddBrushFrame(Start, Size);
	}
}

void CTitleBar::AddBrushFrame(int Count, float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(Count, FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CTitleBar::SetCurrentFrame(int Frame)
{
	mBrush.Frame = Frame;
}

void CTitleBar::SetAnimationPlayTime(float PlayTime)
{
	mBrush.PlayTime = PlayTime;

	mBrush.FrameTime = mBrush.PlayTime / mBrush.AnimationFrames.size();
}

void CTitleBar::SetAnimationPlayRate(float PlayRate)
{
	mBrush.PlayRate = PlayRate;
}

void CTitleBar::SetAnimationType(EAnimation2DTextureType Type)
{
	mBrush.AnimationType = Type;
}

bool CTitleBar::Init()
{
	CWidget::Init();

	return true;
}

void CTitleBar::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	mBrush.PlayAnimation(DeltaTime);


	if (mWidgetState == EWidgetState::Clicked)
	{
		if (!mUpdateWidget.expired())
		{
			auto World = mWorld.lock();

			auto Input = World->GetInput().lock();

			FVector2 Move = Input->GetMouseMove();
			
			auto Widget = mUpdateWidget.lock();

			Widget->AddPos(Move);

		}
	}


	if (mChild)
	{
		mChild->Update(DeltaTime);
	}
}

void CTitleBar::Render()
{
	CWidget::Render();

	RenderBrush(mBrush, mRenderPos, mSize);

	if (mChild)
	{
		mChild->Render();
	}
}

bool CTitleBar::CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos)
{
	return CWidget::CollisionMouse(Result, MousePos);
}

void CTitleBar::MouseHovered()
{
	CWidget::MouseHovered();
}

void CTitleBar::MouseUnHovered()
{
	CWidget::MouseUnHovered();
}

void CTitleBar::MouseClicked()
{
	CWidget::MouseClicked();
}

void CTitleBar::MouseRelease()
{
	CWidget::MouseRelease();
}

CTitleBar* CTitleBar::Clone()
{
	return new CTitleBar(*this);
}
