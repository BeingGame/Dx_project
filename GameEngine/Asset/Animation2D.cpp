#include "Animation2D.h"
#include "AssetManager.h"
#include "TextureManager.h"

CAnimation2D::CAnimation2D()
{
	mType = EAssetType::Animation2D;
}

CAnimation2D::~CAnimation2D()
{}

void CAnimation2D::SetTexture(const std::weak_ptr<class CTexture>& Texture)
{
	mTexture = Texture;
}

void CAnimation2D::SetTexture(const std::string& Name)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		mTexture = TextureMgr->FindTexture(Name);
	}
}

void CAnimation2D::SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		TextureMgr->LoadTexture(Name, FileName, PathName);

		mTexture = TextureMgr->FindTexture(Name);
	}
}

void CAnimation2D::SetTextureFullPath(const std::string& Name, const TCHAR* FullPath)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		TextureMgr->LoadTexture(Name, FullPath);

		mTexture = TextureMgr->FindTexture(Name);
	}
}

void CAnimation2D::SetTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		TextureMgr->LoadTexture(Name, FileName, PathName);

		mTexture = TextureMgr->FindTexture(Name);
	}
}

void CAnimation2D::SetTextureFullPath(const std::string& Name, std::vector<const TCHAR*> FullPath)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		TextureMgr->LoadTexture(Name, FullPath);

		mTexture = TextureMgr->FindTexture(Name);
	}
}

void CAnimation2D::AddFrame(const FVector2& Start, const FVector2& Size, const FVector2& Offset)
{
	FTextureFrame Frame;
	Frame.Start = Start;
	Frame.Size = Size;
	Frame.Offset = Offset;

	mFrameArray.push_back(Frame);
}

void CAnimation2D::AddFrame(float StartX, float StartY, float SizeX, float SizeY, float OffsetX, float OffsetY)
{
	FTextureFrame Frame;
	Frame.Start = FVector2(StartX, StartY);
	Frame.Size = FVector2(SizeX, SizeY);
	Frame.Offset = FVector2(OffsetX, OffsetY);

	mFrameArray.push_back(Frame);
}

void CAnimation2D::AddFrame(int Count, const FVector2& Start, const FVector2& Size, const FVector2& Offset)
{
	for (int i = 0; i < Count; ++i)
	{
		FTextureFrame Frame;
		Frame.Start = Start;
		Frame.Size = Size;
		Frame.Offset = Offset;

		mFrameArray.push_back(Frame);
	}
}

void CAnimation2D::AddFrame(int Count, float StartX, float StartY, float SizeX, float SizeY, float OffsetX, float OffsetY)
{
	for (int i = 0; i < Count; ++i)
	{
		FTextureFrame Frame;
		Frame.Start = FVector2(StartX, StartY);
		Frame.Size = FVector2(SizeX, SizeY);
		Frame.Offset = FVector2(OffsetX, OffsetY);

		mFrameArray.push_back(Frame);
	}

}

void CAnimation2D::ClearFrame()
{
	mFrameArray.clear();
}

void CAnimation2D::CalculateFrameRatio()
{
	if (mFrameArray.empty())
	{
		return;
	}

	FVector2 MaxSize = FVector2(0.f, 0.f);

	if (mTextureType == EAnimation2DTextureType::SpriteSheet)
	{
		for (auto& Frame : mFrameArray)
		{
			if (Frame.Size.x > MaxSize.x)
			{
				MaxSize.x = Frame.Size.x;
			}

			if (Frame.Size.y > MaxSize.y)
			{
				MaxSize.y = Frame.Size.y;
			}
		}
	}
	else
	{
		//텍스처 타입이 프레임일때 텍스처의 최대크기를 가져와준다.
		auto Texture = mTexture.lock();

		if (Texture)
		{
			size_t Size = mFrameArray.size();

			for (size_t i = 0; i < Size; ++i)
			{
				const FTextureInfo* TexInfo = Texture->GetTexture((int)i);

				if (TexInfo)
				{
					if ((float)TexInfo->Width > MaxSize.x)
					{
						MaxSize.x = (float)TexInfo->Width;
					}

					if ((float)TexInfo->Height > MaxSize.y)
					{
						MaxSize.y = (float)TexInfo->Height;
					}

				}
			}
		}
	}

	//maxsize가 따로 없다면 ratio계산을 하지 않는다.
	if (MaxSize.x <= 0.f || MaxSize.y <= 0.f)
	{
		return;
	}

	size_t Size = mFrameArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		FVector2 FrameSize = mFrameArray[i].Size;

		if (mTextureType == EAnimation2DTextureType::Frame)
		{
			auto Texture = mTexture.lock();

			if (Texture)
			{
				const FTextureInfo* TexInfo = Texture->GetTexture((int)i);

				if (TexInfo)
				{
					FrameSize.x = (float)TexInfo->Width;
					FrameSize.y= (float)TexInfo->Height;
				}
			}
		}

		mFrameArray[i].Ratio.x = FrameSize.x / MaxSize.x;
		mFrameArray[i].Ratio.y = FrameSize.y / MaxSize.y;
	}
}

void CAnimation2D::MakePaletteColor(const FVector2& OriginPos, const FVector2& TargetPos, const FVector2& FrameSize)
{
	if (mTextureType != EAnimation2DTextureType::SpriteSheet)
	{
		return;
	}

	auto Texture = mTexture.lock();
	if (!Texture)
	{
		return;
	}

	if (!Texture->CreatePaletteTexture())
	{
		return;
	}

	Texture->MakePaletteColor(OriginPos, TargetPos, FrameSize);

}
