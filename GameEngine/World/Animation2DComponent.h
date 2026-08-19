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

	//같은 애니메이션이어도 처음부터 다시 재생한다. (공격 연타 등)
	void PlayAnimation(const std::string& Name);

	//반복이 아닌 현재 애니메이션이 끝까지 재생됐는지.
	//액션 상태(공격/피격)가 종료 시점을 판단하는 데 쓴다.
	bool IsCurrentAnimationEnd() const;

	//지금 재생 중인 시퀀스를 그 프레임에 세워둔다. 풀면 그 프레임부터 이어서 간다.
	//(기본 공격 3타 모으기 — 공격키를 누르고 있는 동안 첫 프레임에서 붙잡아둔다)
	void SetCurrentAnimationPause(bool Pause);
	bool IsCurrentAnimationPaused() const;

	std::weak_ptr<CAnimation2DSequence> FindSequence(const std::string& Name) const;

	void SetPlayTime(const std::string& Name, float PlayTime);
	void SetPlayRate(const std::string& Name, float PlayRate);
	void SetLoop(const std::string& Name, bool Loop);
	void SetReverse(const std::string& Name, bool Reverse);
	void SetSymmetry(const std::string& Name, bool Symmetry);

	void SetUsePalette(bool Use);
	void SetPaletteIndex(int Index);

	void SetShader();

	//---- 저장/복원 ----
	//등록된 시퀀스 목록. 월드 저장과 애니메이션 에디터가 훑어본다.
	const std::unordered_map<std::string, std::shared_ptr<CAnimation2DSequence>>& GetAnimationMap() const
	{
		return mAnimationMap;
	}

	//현재 재생 중인 시퀀스 이름 (없으면 빈 문자열)
	std::string GetCurrentAnimationName() const;

	virtual void Save(std::ofstream& File) const override;
	virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;

private:
	void RefreshTexture();

	//소유 액터의 메시 컴포넌트를 찾아 UpdateComponent로 물린다.
	//에디터에서 붙이거나 월드에서 불러온 컴포넌트는 아무도 짝을 지어주지 않아서,
	//이게 없으면 애니메이션이 화면에 나오지 않는다.
	bool BindUpdateComponentFromOwner();

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

	virtual std::string GetTypeName() const override { return "CAnimation2DComponent"; }

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

