#include "Animation2DComponent.h"
#include "MeshComponent.h"

#include "../Asset/AssetManager.h"
#include "../Asset/Animation2D.h"
#include "../Asset/AnimationManager.h"
#include "../Asset/CBufferAnimation2D.h"
#include "../Asset/CBufferPalette.h"
#include "../Asset/Texture.h"


CAnimation2DComponent::CAnimation2DComponent()
{}

CAnimation2DComponent::CAnimation2DComponent(const CAnimation2DComponent& src)
	:CActorComponent(src)
{
	mUpdateEnable = src.mUpdateEnable;

	mCBufferAnim2D = std::shared_ptr<CCBufferAnimation2D>(src.mCBufferAnim2D->Clone());

	mCBufferAnim2D->Init();

	//애니메이션 리스트들을 복사해준다.
	for (auto& It : mAnimationMap)
	{
		auto Sequence = std::shared_ptr<CAnimation2DSequence>(It.second->Clone());

		mAnimationMap.insert(std::make_pair(It.first, Sequence));
	}

	if (src.mCurrentAnimation)
	{
		auto iter = mAnimationMap.find(src.mCurrentAnimation->GetName());

		if (iter != mAnimationMap.end())
		{
			mCurrentAnimation = mAnimationMap[iter->first];
		}

	}

}

CAnimation2DComponent::CAnimation2DComponent(CAnimation2DComponent&& src) noexcept
	:CActorComponent(std::move(src))
{
	mUpdateEnable = src.mUpdateEnable;

	mCBufferAnim2D = src.mCBufferAnim2D;
	src.mCBufferAnim2D.reset();

	mAnimationMap = src.mAnimationMap;
	src.mAnimationMap.clear();

	mCurrentAnimation = src.mCurrentAnimation;
	src.mCurrentAnimation.reset();
}

CAnimation2DComponent::~CAnimation2DComponent()
{}

EAnimation2DTextureType CAnimation2DComponent::GetTextureType() const
{
	if (mCurrentAnimation)
	{
		return mCurrentAnimation->GetTextureType();
	}

	return EAnimation2DTextureType();
}

int CAnimation2DComponent::GetAnimationFrame() const
{
	if (mCurrentAnimation)
	{
		return mCurrentAnimation->GetFrame();
	}

	return 0;
}

FVector2 CAnimation2DComponent::GetAnimLTUV()
{
	int Frame = mCurrentAnimation->GetFrame();

	auto Anim = mCurrentAnimation->GetAnimation().lock();

	if (Anim)
	{
		const FTextureFrame& TexFrame = Anim->GetFrame(Frame);

		//스프라이트 애니메이션일때 추가될 조건문
		if (Anim->GetType() == EAnimation2DTextureType::SpriteSheet)
		{
			auto Texture = Anim->GetTexture().lock();

			if (Texture)
			{
				const FTextureInfo* TexInfo = Texture->GetTexture();

				return FVector2(TexFrame.Start.x / TexInfo->Width, TexFrame.Start.y / TexInfo->Height);
			}
		}
	}

	//타입이 스프라이트 시트아니면 LTUV는 항상 0,0
	return FVector2(0.f, 0.f);
}

FVector2 CAnimation2DComponent::GetAnimRBUV()
{
	int Frame = mCurrentAnimation->GetFrame();

	auto Anim = mCurrentAnimation->GetAnimation().lock();

	if (Anim)
	{
		const FTextureFrame& TexFrame = Anim->GetFrame(Frame);

		//스프라이트 애니메이션일때 추가될 조건문
		if (Anim->GetType() == EAnimation2DTextureType::SpriteSheet)
		{
			auto Texture = Anim->GetTexture().lock();

			if (Texture)
			{
				const FTextureInfo* TexInfo = Texture->GetTexture();

				return FVector2((TexFrame.Start.x + TexFrame.Size.x) / TexInfo->Width, (TexFrame.Start.y + TexFrame.Size.y) / TexInfo->Height);
			}
		}
	}

	//타입이 스프라이트 시트아니면 RBUV는 항상 1,1
	return FVector2(1.f, 1.f);
}

void CAnimation2DComponent::SetUpdateComponent(std::weak_ptr<class CMeshComponent>& MeshComponent)
{
	mUpdateComponent = MeshComponent;
}

void CAnimation2DComponent::AddAnimation(const std::weak_ptr<class CAnimation2D>& Anim, float PlayTime, float PlayRate, bool Loop, bool Reverse, bool Symmetry)
{
	//애니메이션이 추가되면 새로운 시퀀스를 등록한다.
	auto _Anim = Anim.lock();

	if (!_Anim)
	{
		return;
	}

	//동일한 애니메이션이 시퀀스를 새로만들지 않는다.

	auto iter = mAnimationMap.find(_Anim->GetName());

	if (iter != mAnimationMap.end())
	{
		return;
	}

	std::shared_ptr<CAnimation2DSequence> NewSequence = std::make_shared<CAnimation2DSequence>();

	NewSequence->SetAnimation(Anim);
	NewSequence->SetPlayTime(PlayTime);
	NewSequence->SetPlayRate(PlayRate);
	NewSequence->SetLoop(Loop);
	NewSequence->SetReverse(Reverse);
	NewSequence->SetSymmetry(Symmetry);

	mAnimationMap.insert(std::make_pair(_Anim->GetName(), NewSequence));

	//애니메이션 추가 될 때 현재 실행중인 애니메이션없으면 갱신해준다.

	if (!mCurrentAnimation)
	{
		mCurrentAnimation = NewSequence;

		//상수버퍼의 텍스처 타입을 업데이트 한다.
		RefreshTexture();
	}


}

void CAnimation2DComponent::AddAnimation(const std::string& Name, float PlayTime, float PlayRate, bool Loop, bool Reverse, bool Symmetry)
{
	//Name으로 먼저 애니메이션을 찾아준다.
	auto AnimationMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);

	if (AnimationMgr)
	{
		auto Anim = AnimationMgr->FindAnimation(Name);

		AddAnimation(Anim, PlayTime, PlayRate, Loop, Reverse, Symmetry);
	}
}

void CAnimation2DComponent::ChangeAnimation(const std::string& Name)
{
	//애니메이션 변경시 컴포넌트 확인, 현재 애니메이션이 존재하는지 유무, 동일한 애니메이션인지 체크
	//std::string CheckName = "Animation2D_" + Name;

	if (mUpdateComponent.expired())
	{
		return;
	}
	//현재 애니메이션을 체크하는 이유
	//현재 애니메이션이 갱신되지 않았다면 애니메이션 맵에 어떤 애니메이션도 없다는 뜻이기 때문
	else if (!mCurrentAnimation)
	{
		return;
	}
	//현재 애니메이션이랑 동일한 애니메이션으로 바꿔주는걸 막아준다.
	else if (mCurrentAnimation->GetName() == Name)
	{
		return;
	}

	auto iter = mAnimationMap.find(Name);

	if (iter == mAnimationMap.end())
	{
		return;
	}

	//먼저 교체하기전에 현재 애니메이션의 정보를 초기화한다.

	mCurrentAnimation->Clear();

	//애니메이션을 변경한다.
	mCurrentAnimation = iter->second;

	//상수버퍼의 텍스처 타입을 갱신한다.
	RefreshTexture();
}

void CAnimation2DComponent::SetPlayTime(const std::string& Name, float PlayTime)
{
	auto iter = mAnimationMap.find(Name);

	if (iter == mAnimationMap.end())
	{
		return;
	}

	iter->second->SetPlayTime(PlayTime);
}

void CAnimation2DComponent::SetPlayRate(const std::string& Name, float PlayRate)
{
	auto iter = mAnimationMap.find(Name);

	if (iter == mAnimationMap.end())
	{
		return;
	}

	iter->second->SetPlayRate(PlayRate);
}

void CAnimation2DComponent::SetLoop(const std::string& Name, bool Loop)
{
	auto iter = mAnimationMap.find(Name);

	if (iter == mAnimationMap.end())
	{
		return;
	}

	iter->second->SetLoop(Loop);
}

void CAnimation2DComponent::SetReverse(const std::string& Name, bool Reverse)
{
	auto iter = mAnimationMap.find(Name);

	if (iter == mAnimationMap.end())
	{
		return;
	}

	iter->second->SetReverse(Reverse);
}

void CAnimation2DComponent::SetSymmetry(const std::string& Name, bool Symmetry)
{
	auto iter = mAnimationMap.find(Name);

	if (iter == mAnimationMap.end())
	{
		return;
	}

	iter->second->SetSymmetry(Symmetry);
}

void CAnimation2DComponent::SetUsePalette(bool Use)
{
	if (!mCurrentAnimation)
	{
		return;
	}

	auto Animation = mCurrentAnimation->GetAnimation().lock();

	if (!Animation)
	{
		return;
	}

	auto Texture = Animation->GetTexture().lock();

	if (!Texture)
	{
		return;
	}

	auto Palette = Texture->GetPaletteColorAll();

	if (Palette.empty())
	{
		return;
	}


	mCBufferPalette->SetUsePalette(Use);
}

void CAnimation2DComponent::SetPaletteIndex(int Index)
{
	//팔레트 색상 배열 인덱스를 받아와서
	//상수버퍼에 적용할 컬러를 상수버퍼 데이터에 넣어준다.
	if (mCurrentAnimation)
	{
		auto Anim = mCurrentAnimation->GetAnimation().lock();

		if (Anim)
		{
			auto Texture = Anim->GetTexture().lock();

			if (Texture)
			{
				auto ColorArray = Texture->GetPaletteColor(Index);
				mCBufferPalette->SetColor(ColorArray, ColorArray.size());
			}
		}
	}

}

void CAnimation2DComponent::SetShader()
{
	//SetShader에서 상수버퍼에 데이터를 레지스터에 등록하기전에
	//데이터를 전부 업데이트해준다.
	int Frame = mCurrentAnimation->GetFrame();

	mCBufferAnim2D->SetTextureSymmetry(mCurrentAnimation->GetSymmetry());

	auto Anim = mCurrentAnimation->GetAnimation().lock();

	if (Anim)
	{
		const FTextureFrame& TexFrame = Anim->GetFrame(Frame);
		mCBufferAnim2D->SetAnimRatio(TexFrame.Ratio);
		mCBufferAnim2D->SetAnimOffset(TexFrame.Offset);

		//스프라이트 애니메이션일때 추가될 조건문
		if (Anim->GetType() == EAnimation2DTextureType::SpriteSheet)
		{
			auto Texture = Anim->GetTexture().lock();

			if (Texture)
			{
				const FTextureInfo* TexInfo = Texture->GetTexture();

				mCBufferAnim2D->SetLTUV(TexFrame.Start.x / TexInfo->Width, TexFrame.Start.y / TexInfo->Height);
				mCBufferAnim2D->SetRBUV((TexFrame.Start.x + TexFrame.Size.x) / TexInfo->Width, (TexFrame.Start.y + TexFrame.Size.y) / TexInfo->Height);
			}
		}
	}

	mCBufferPalette->UpdateBuffer();
	mCBufferAnim2D->UpdateBuffer();
}

void CAnimation2DComponent::RefreshTexture()
{
	//컴포넌트가 유효하면
	//현재 애니메이션 존재할때 애니메이션을 활성화하면서 
	//상수버퍼의 텍스처 타입을 업데이트한다.
	auto MeshComp = mUpdateComponent.lock();

	if (MeshComp && mCurrentAnimation)
	{
		if (!mUpdateEnable)
		{
			MeshComp->SetAnimComp(GetThisPtr<CAnimation2DComponent>());

			mUpdateEnable = true;
		}

		auto Anim = mCurrentAnimation->GetAnimation().lock();

		if (Anim)
		{
			//상수버퍼의 텍스처 타입을 갱신한다.
			mCBufferAnim2D->SetAnimation2DTextureType(Anim->GetType());
			//메쉬 컴포넌트의 텍스처를 애니메이션의 텍스처로 변경한다.
			MeshComp->SetTexture(0, 0, Anim->GetTexture());
		}
	}
}

bool CAnimation2DComponent::Init()
{
	CActorComponent::Init();

	//상수버퍼를 초기화해준다.
	mCBufferAnim2D = std::make_shared<CCBufferAnimation2D>();

	mCBufferAnim2D->Init();

	mCBufferAnim2D->SetAnimation2DEnable(true);

	mCBufferPalette = std::make_shared<CCBufferPalette>();

	mCBufferPalette->Init();

	return true;
}

void CAnimation2DComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);

	//현재 애니메이션이 활성화되어있는지 확인한다.
	//활성화 되어 있지 않으면 다시 활성화를 해주고, 시간에 따라서 활성화된 프레임을 변경해준다.

	if (mCurrentAnimation)
	{
		if (!mUpdateEnable)
		{
			auto MeshComp = mUpdateComponent.lock();

			if (MeshComp)
			{
				MeshComp->SetAnimComp(GetThisPtr<CAnimation2DComponent>());

				mUpdateEnable = true;
			}
		}

		mCurrentAnimation->Update(DeltaTime);

		mCBufferAnim2D->SetAnimFrame(mCurrentAnimation->GetFrame());
	}
}

void CAnimation2DComponent::PostUpdate(float DeltaTime)
{
	CActorComponent::PostUpdate(DeltaTime);

}

void CAnimation2DComponent::Destroy()
{
	CActorComponent::Destroy();
}
