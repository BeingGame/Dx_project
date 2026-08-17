#include "Animation2DComponent.h"
#include "MeshComponent.h"
#include "Actor.h"

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

		//Offset은 에디터에서 텍셀(픽셀) 단위로 입력한다.
		//셰이더의 정점은 -0.5~0.5 정규화 좌표이고, 그 폭 1.0이 시퀀스 최대 프레임
		//크기에 대응하므로 최대 크기로 나눠서 넘겨준다.
		//Y는 부호를 뒤집는다. 텍스처 좌표는 아래로 증가하지만 월드 Y는 위로 증가하므로,
		//그대로 넘기면 에디터에서 +를 눌렀을 때 스프라이트가 위로 올라간다.
		const FVector2& MaxSize = Anim->GetMaxFrameSize();

		FVector2 NormalizedOffset;
		NormalizedOffset.x = (MaxSize.x > 0.f) ?  TexFrame.Offset.x / MaxSize.x : 0.f;
		NormalizedOffset.y = (MaxSize.y > 0.f) ? -TexFrame.Offset.y / MaxSize.y : 0.f;

		mCBufferAnim2D->SetAnimOffset(NormalizedOffset);

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

// ── 저장 / 복원 ───────────────────────────────────────────────────────────────

std::string CAnimation2DComponent::GetCurrentAnimationName() const
{
	if (!mCurrentAnimation)
	{
		return std::string();
	}

	return mCurrentAnimation->GetName();
}

bool CAnimation2DComponent::BindUpdateComponentFromOwner()
{
	auto Owner = mOwner.lock();

	if (!Owner)
	{
		return false;
	}

	//씬 컴포넌트 중 첫 번째 메시를 짝으로 삼는다.
	//메시 하나당 애니메이션 컴포넌트 하나이므로, 여러 개를 쓰려면
	//SetUpdateComponent로 직접 지정해야 한다.
	for (const auto& Comp : Owner->GetSceneCompList())
	{
		auto MeshComp = std::dynamic_pointer_cast<CMeshComponent>(Comp);

		if (!MeshComp)
		{
			continue;
		}

		std::weak_ptr<CMeshComponent> WeakMesh = MeshComp;
		SetUpdateComponent(WeakMesh);

		return true;
	}

	return false;
}

void CAnimation2DComponent::Save(std::ofstream& File) const
{
	CActorComponent::Save(File);

	//프레임 데이터는 .anim2d 에셋에 있고 시작할 때 전부 등록되므로
	//여기에는 "어떤 애니메이션을 어떤 재생 설정으로 쓰는가"만 적는다.
	File << "SeqCount=" << mAnimationMap.size() << "\n";

	int Index = 0;

	for (const auto& Pair : mAnimationMap)
	{
		const auto& Seq = Pair.second;

		File << "Seq" << Index++ << "="
			<< Pair.first            << "|"
			<< Seq->GetPlayTime()    << "|"
			<< Seq->GetPlayRate()    << "|"
			<< (Seq->GetLoop()     ? 1 : 0) << "|"
			<< (Seq->GetReverse()  ? 1 : 0) << "|"
			<< (Seq->GetSymmetry() ? 1 : 0) << "\n";
	}

	File << "Current=" << GetCurrentAnimationName() << "\n";
}

void CAnimation2DComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
	CActorComponent::Load(Props);

	int SeqCount = 0;

	if (auto It = Props.find("SeqCount"); It != Props.end())
	{
		try { SeqCount = std::stoi(It->second); }
		catch (...) { SeqCount = 0; }
	}

	for (int i = 0; i < SeqCount; ++i)
	{
		auto It = Props.find("Seq" + std::to_string(i));

		if (It == Props.end())
		{
			continue;
		}

		//Name|PlayTime|PlayRate|Loop|Reverse|Symmetry
		std::vector<std::string> Fields;
		{
			const std::string& Line = It->second;
			size_t Start = 0;

			while (true)
			{
				size_t Bar = Line.find('|', Start);

				if (Bar == std::string::npos)
				{
					Fields.push_back(Line.substr(Start));
					break;
				}

				Fields.push_back(Line.substr(Start, Bar - Start));
				Start = Bar + 1;
			}
		}

		if (Fields.size() < 6 || Fields[0].empty())
		{
			continue;
		}

		float PlayTime = 1.f, PlayRate = 1.f;

		try
		{
			PlayTime = std::stof(Fields[1]);
			PlayRate = std::stof(Fields[2]);
		}
		catch (...)
		{
			//숫자가 깨졌으면 기본값으로 둔다.
		}

		AddAnimation(Fields[0], PlayTime, PlayRate,
			Fields[3] == "1", Fields[4] == "1", Fields[5] == "1");
	}

	//현재 시퀀스 복원.
	//ChangeAnimation은 메시가 아직 안 물려 있으면 그냥 빠져나가므로 직접 지정한다.
	if (auto It = Props.find("Current"); It != Props.end() && !It->second.empty())
	{
		auto Found = mAnimationMap.find(It->second);

		if (Found != mAnimationMap.end())
		{
			mCurrentAnimation = Found->second;
		}
	}

	//씬 컴포넌트가 먼저 복원되므로 여기서 메시를 찾을 수 있다.
	BindUpdateComponentFromOwner();
	RefreshTexture();
}

void CAnimation2DComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);

	//현재 애니메이션이 활성화되어있는지 확인한다.
	//활성화 되어 있지 않으면 다시 활성화를 해주고, 시간에 따라서 활성화된 프레임을 변경해준다.

	//에디터에서 붙였거나 월드에서 불러온 컴포넌트는 짝지어진 메시가 없다.
	//(SetUpdateComponent를 직접 부르는 건 Player/Monster 같은 하드코딩 액터뿐)
	//그대로 두면 애니메이션이 화면에 나오지 않으므로 여기서 스스로 찾아 붙는다.
	if (mUpdateComponent.expired())
	{
		BindUpdateComponentFromOwner();
	}

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
