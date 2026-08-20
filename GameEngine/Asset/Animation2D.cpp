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

void CAnimation2D::ScaleTotalDuration(float TotalTime)
{
	if (mFrameArray.empty() || TotalTime <= 0.f)
	{
		return;
	}

	float Current = GetTotalDuration();

	//기존 합이 0이면 비율을 알 수 없으므로 균등하게 나눠준다.
	if (Current <= 0.f)
	{
		float Each = TotalTime / (float)mFrameArray.size();

		for (auto& Frame : mFrameArray)
			Frame.Duration = Each;

		return;
	}

	float Scale = TotalTime / Current;

	for (auto& Frame : mFrameArray)
	{
		Frame.Duration *= Scale;

		if (Frame.Duration < 0.001f)
			Frame.Duration = 0.001f;
	}
}

void CAnimation2D::AddFrame(const FVector2& Start, const FVector2& Size, const FVector2& Offset, float Duration)
{
	FTextureFrame Frame;
	Frame.Start = Start;
	Frame.Size = Size;
	Frame.Offset = Offset;
	Frame.Duration = (Duration < 0.001f) ? 0.001f : Duration;

	mFrameArray.push_back(Frame);

	mFrameRatioDirty = true;
}

void CAnimation2D::AddFrame(float StartX, float StartY, float SizeX, float SizeY, float OffsetX, float OffsetY)
{
	FTextureFrame Frame;
	Frame.Start = FVector2(StartX, StartY);
	Frame.Size = FVector2(SizeX, SizeY);
	Frame.Offset = FVector2(OffsetX, OffsetY);

	mFrameArray.push_back(Frame);

	mFrameRatioDirty = true;
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

	mFrameRatioDirty = true;
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


	mFrameRatioDirty = true;
}

void CAnimation2D::ClearFrame()
{
	mFrameArray.clear();

	mMaxFrameSize    = FVector2(0.f, 0.f);
	mFrameRatioDirty = true;
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

	//이 애니메이션 자체의 최대 크기는 공용 기준과 무관하게 보관해둔다.
	//(컴포넌트가 이 값을 모아 공용 기준을 정한다)
	mOwnMaxFrameSize = MaxSize;

	//정규화 기준(Base). 공용 기준이 설정돼 있으면 그걸 쓰고, 아니면 자기 최대를 쓴다.
	//여러 아틀라스를 섞어 써도 이 Base가 같으면 픽셀 크기가 화면 크기로 일관되게 매핑된다.
	FVector2 Base = (mRatioReference.x > 0.f && mRatioReference.y > 0.f)
		? mRatioReference : MaxSize;

	//Offset을 픽셀 단위로 환산할 때 기준이 되므로 실제로 쓴 Base를 보관해둔다.
	mMaxFrameSize = Base;

	//base가 따로 없다면 ratio계산을 하지 않는다.
	if (Base.x <= 0.f || Base.y <= 0.f)
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

		mFrameArray[i].Ratio.x = FrameSize.x / Base.x;
		mFrameArray[i].Ratio.y = FrameSize.y / Base.y;
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
