#pragma once

#include "ActorComponent.h"
#include "Animation2DSequence.h"

//애니메이션 컴포넌트
//메쉬가 사용할 애니메이션의 배열을 가지고 있다.
//매니저에서 애니메이션을 가져와서 배열에 저장한다.

//메쉬 컴포넌트를 UpdateComponent로 가지고있는다.
//저장한 애니메이션을 필요할때 교체하고, 애니메이션을 실행한다.

class CAnimation2DComponent :
	public CActorComponent
{
public:
	CAnimation2DComponent();
	CAnimation2DComponent(const CAnimation2DComponent& src);
	CAnimation2DComponent(CAnimation2DComponent&& src) noexcept;
	virtual ~CAnimation2DComponent();

protected:
	//애니메이션 텍스처가 업데이트될 컴포넌트변수
	std::weak_ptr<class CMeshComponent> mUpdateComponent;

	//컴포넌트가 사용할 애니메이션 맵
	std::unordered_map<std::string, std::shared_ptr<CAnimation2DSequence>> mAnimationMap;

	//현재 사용중인 애니메이션
	//shared_ptr로 만드는 이유: 컴포넌트가 시퀀스를 관리한다.
	std::shared_ptr<CAnimation2DSequence> mCurrentAnimation;

	std::shared_ptr<class CCBufferAnimation2D> mCBufferAnim2D;
	std::shared_ptr<class CCBufferPalette> mCBufferPalette;

	//애니메이션을 업데이트 할지말지 결정해주는 변수
	bool mUpdateEnable = false;

public:
	EAnimation2DTextureType GetTextureType() const;
	int GetAnimationFrame() const;
	FVector2 GetAnimLTUV();
	FVector2 GetAnimRBUV();

	void SetUpdateComponent(std::weak_ptr<class CMeshComponent>& MeshComponent);
	void AddAnimation(const std::weak_ptr<class CAnimation2D>& Anim, float PlayTime = 1.f, float PlayRate = 1.f, bool Loop = false, bool Reverse = false, bool Symmetry = false);
	void AddAnimation(const std::string& Name, float PlayTime = 1.f, float PlayRate = 1.f, bool Loop = false, bool Reverse = false, bool Symmetry = false);
	void ChangeAnimation(const std::string& Name);
	void SetPlayTime(const std::string& Name, float PlayTime);
	void SetPlayRate(const std::string& Name, float PlayRate);
	void SetLoop(const std::string& Name, bool Loop);
	void SetReverse(const std::string& Name, bool Reverse);
	void SetSymmetry(const std::string& Name, bool Symmetry);

	void SetUsePalette(bool Use);
	void SetPaletteIndex(int Index);

	void SetShader();

private:
	void RefreshTexture();


public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

public:
	virtual CAnimation2DComponent* Clone() const
	{
		return new CAnimation2DComponent(*this);
	}

public:
	template<typename T>
	void AddNotify(const std::string& SequenceName, const std::string& Name, int Frame, T* Object, void(T::* Func)())
	{
		auto iter = mAnimationMap.find(SequenceName);

		if (iter == mAnimationMap.end())
		{
			return;
		}

		iter->second->AddNotify<T>(Name, Frame, Object, Func);
	}

	template<typename T>
	void SetFinishFunction(const std::string& SequenceName, T* Object, void(T::* Func)())
	{
		auto iter = mAnimationMap.find(SequenceName);

		if (iter == mAnimationMap.end())
		{
			return;
		}

		iter->second->SetFinishFunction<T>(Object, Func);
	}

};

