#include "Image.h"

#include "../Asset/AssetManager.h"
#include "../Asset/TextureManager.h"
#include "../Asset/Texture.h"

#include "WorldManager.h"
#include "Input.h"

CImage::CImage()
{}

CImage::CImage(const CImage& src)
	:CWidget(src)
{
	mBrush = src.mBrush;
}

CImage::~CImage()
{}

bool CImage::SetTexture(const std::weak_ptr<class CTexture> Texture)
{
	mBrush.Texture = Texture;

	return true;
}

bool CImage::SetTexture(const std::string& Name)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (!TextureMgr)
	{
		return false;

	}

	mBrush.Texture = TextureMgr->FindTexture(Name);

	return true;
}

bool CImage::SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName)
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

bool CImage::SetTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName)
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

void CImage::SetTint(const FVector4& Color)
{
	mBrush.Tint = Color;
}

void CImage::SetTint(float r, float g, float b, float a)
{
	mBrush.Tint = FVector4(r, g, b, a);
}

void CImage::SetBrushAnimation(bool AnimationEnable)
{
	mBrush.AnimationEnable = AnimationEnable;
}

void CImage::AddBrushFrame(const FVector2& Start, const FVector2& Size)
{
	FTextureFrame Frame;

	Frame.Start = Start;
	Frame.Size = Size;

	mBrush.AnimationFrames.push_back(Frame);

	mBrush.FrameTime = mBrush.PlayTime / mBrush.AnimationFrames.size();
}

void CImage::AddBrushFrame(float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CImage::AddBrushFrame(int Count, const FVector2& Start, const FVector2& Size)
{
	for (int i = 0; i < Count; ++i)
	{
		AddBrushFrame(Start, Size);
	}
}

void CImage::AddBrushFrame(int Count, float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(Count, FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CImage::SetCurrentFrame(int Frame)
{
	mBrush.Frame = Frame;
}

void CImage::SetAnimationPlayTime(float PlayTime)
{
	mBrush.PlayTime = PlayTime;

	mBrush.FrameTime = mBrush.PlayTime / mBrush.AnimationFrames.size();
}

void CImage::SetAnimationPlayRate(float PlayRate)
{
	mBrush.PlayRate = PlayRate;
}

void CImage::SetAnimationType(EAnimation2DTextureType Type)
{
	mBrush.AnimationType = Type;
}

bool CImage::Init()
{
	CWidget::Init();

	return true;
}

void CImage::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	mBrush.PlayAnimation(DeltaTime);
}

void CImage::Render()
{
	CWidget::Render();

	RenderBrush(mBrush, mRenderPos, mSize);
}

bool CImage::CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos)
{
	return false;
}

CImage* CImage::Clone()
{
	return new CImage(*this);
}