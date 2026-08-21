#pragma once

#include "../Asset/Animation2D.h"

//애니메이션 시퀀스
//애니메이션이 실제로 실행되기 위해 정해진 방향으로 프레임을 진행시킨다.
//애니메이션의 방향, 애니메이션의 실행속도, 애니메이션이 현재 실행 위치(프레임) 등과 같은 변수를 저장한다.

//노티파이
//특정시간에 어떤 사건일어나도록 알림해주는 기능
//사건 -> 콜백함수
//애니메이션 노티파이
//특정 애니메이션 시간 혹은 프레임에 특정 콜백함수를 호출하도록 만들어준다.

struct FAnimation2DNotify
{
	std::string Name;
	
	//노티파이가 실행되었는지에 대한 여부
	bool Call = false;

	//노티파이가 실행될 프레임
	int Frame = 0;

	//노티파이가 실행될때 호출될 콜백함수
	std::function<void()> Function;
};

class CAnimation2DSequence
{
public:
	CAnimation2DSequence();
	CAnimation2DSequence(const CAnimation2DSequence& src);
	CAnimation2DSequence(CAnimation2DSequence&& src) noexcept;
	~CAnimation2DSequence();

private:
	//시퀀스가 실행할 애니메이션
	std::weak_ptr<CAnimation2D> mAnimation;

	//애니메이션의 속성 변수들
	//현재 에니메이션의 실행 프레임
	int mFrame = 0;

	//프레임을 증가 혹은 감소시키기위해 누적할 시간
	float mTime = 0.f;

	//누적된 시간에 따라 프레임이 변경될 시간
	float mFrameTime = 0.f;

	//애니메이션의 총 실행 시간
	float mPlayTime = 0.f;

	//재생속도 배율
	float mPlayRate = 1.f;

	//애니메이션을 반복 재생할지 결정하는 변수
	bool mLoop = false;

	//애니메이션을 역재생시킬지 결정하는 변수
	bool mReverse = false;

	//애니메이션을 대칭으로 만들지 결정하는 변수 (가로/좌우)
	bool mSymmetry = false;

	//세로(위/아래) 반전 여부. 가로 대칭과 별개로, 스프라이트를 상하로 뒤집어 낸다.
	bool mSymmetryV = false;

	//이 시퀀스를 기울여(회전) 낼 각도(라디안). 0이면 그대로다.
	float mRotation = 0.f;

	//이 시퀀스에만 적용되는 추가 오프셋(텍셀).
	//시퀀스마다 스프라이트 아틀라스가 달라 캐릭터가 프레임 안에서 다른 위치에
	//놓일 때, 시퀀스 전체를 통째로 밀어 다른 시퀀스와 정렬을 맞추는 값이다.
	//프레임별 Offset(피벗)과 달리 애니메이션 에셋(.anim2d)이 아니라 이 시퀀스가
	//들고 있으므로, 같은 애니를 다른 액터가 써도 액터마다 다른 값을 줄 수 있다.
	//SetShader에서 프레임 Offset에 더해져 함께 정규화된다.
	FVector2 mOffset = FVector2(0.f, 0.f);

	//애니메이션이 반복되지 않을때
	//애니메이션의 실행을 멈춰줄 변수
	bool mEnd = false;

	//지금 자리에 세워둘지. (기본 공격 3타 모으기처럼 특정 프레임에서 붙잡아둘 때 쓴다)
	//프레임만 멈추고 노티파이는 계속 확인한다.
	bool mPaused = false;

	//애니메이션 노티파이를 저장할 배열
	std::vector<FAnimation2DNotify> mNotifyArray;

	//애니메이션이 종료될때 실행될 콜백함수
	std::function<void()> mFinishFunction;

public:
	const std::weak_ptr<CAnimation2D>& GetAnimation() const
	{
		return mAnimation;
	}

	EAnimation2DTextureType GetTextureType() const
	{
		auto Anim = mAnimation.lock();

		if (!Anim)
		{
			return EAnimation2DTextureType::None;
		}

		return Anim->GetType();
	}

	const std::string& GetName() const
	{
		auto Anim = mAnimation.lock();

		if (!Anim)
		{
			static std::string dummy;
			return dummy;
		}

		return Anim->GetName();
	}

	int GetFrameCount() const
	{
		auto Anim = mAnimation.lock();

		if (!Anim)
		{
			return 0;
		}

		return Anim->GetFrameCount();
	}

	//반복이 아닌 애니메이션이 마지막 프레임까지 갔는지.
	//공격/피격처럼 "끝나면 원래 상태로 돌아가는" 동작이 이걸로 종료를 판단한다.
	//(콜백 대신 매 프레임 물어보는 방식이라 소유자가 사라져도 안전하다)
	bool IsEnd() const
	{
		return mEnd;
	}

	//현재 프레임에 머문 시간. (프레임이 왜 안 넘어가는지 볼 때 쓴다)
	float GetTime() const
	{
		return mTime;
	}

	float GetFrameTime() const
	{
		return mFrameTime;
	}

	int GetFrame() const
	{
		return mFrame;
	}

	//총 재생 시간은 프레임별 Duration의 합이다. (저장된 값이 아니라 매번 더해서 낸다)
	float GetPlayTime() const
	{
		auto Anim = mAnimation.lock();

		if (Anim && Anim->GetFrameCount() > 0)
		{
			return Anim->GetTotalDuration();
		}

		return mPlayTime;
	}

	float GetPlayRate() const
	{
		return mPlayRate;
	}

	bool GetReverse() const
	{
		return mReverse;
	}

	bool GetSymmetry() const
	{
		return mSymmetry;
	}

	bool GetSymmetryV() const
	{
		return mSymmetryV;
	}

	float GetRotation() const
	{
		return mRotation;
	}

	const FVector2& GetOffset() const
	{
		return mOffset;
	}

	bool GetLoop() const
	{
		return mLoop;
	}

	bool IsPaused() const
	{
		return mPaused;
	}

	//지금 프레임에 세워둔다. 풀어주면 그 프레임부터 이어서 재생된다.
	void SetPause(bool Pause)
	{
		mPaused = Pause;
	}

	void SetAnimation(const std::weak_ptr<CAnimation2D>& Animation)
	{
		mAnimation = Animation;

		//프레임 시간은 이제 프레임마다 따로 들고 있으므로 여기서 계산할 게 없다.
		//(Update가 매 틱 현재 프레임의 Duration을 읽어간다)
	}

	//총 재생 시간을 Time으로 맞춘다.
	//프레임별 Duration을 같은 비율로 늘리고 줄이므로 완급은 그대로 유지된다.
	//프레임별 시간이 없던 시절의 호출부(Player, Monster, .anim2d 로드 등)가
	//그대로 동작하도록 남겨둔 진입점이다.
	void SetPlayTime(float Time)
	{
		mPlayTime = Time;

		auto Anim = mAnimation.lock();

		if (Anim && Anim->GetFrameCount() > 0)
		{
			Anim->ScaleTotalDuration(Time);
		}
	}

	void SetPlayRate(float Rate)
	{
		mPlayRate = Rate;
	}

	void SetLoop(bool Loop)
	{
		mLoop = Loop;
	}

	void SetReverse(bool Reverse)
	{
		mReverse = Reverse;

		//애니메이션이 역재생으로 설정되면 시작프레임을 마지막 프레임으로 설정한다.
		auto Anim = mAnimation.lock();

		if (Anim)
		{
			if (mReverse)
			{
				mFrame = Anim->GetFrameCount() - 1;
			}
			else
			{
				mFrame = 0;
			}
		}
	}

	void SetSymmetry(bool Symmetry)
	{
		mSymmetry = Symmetry;
	}

	void SetSymmetryV(bool Symmetry)
	{
		mSymmetryV = Symmetry;
	}

	void SetRotation(float Radian)
	{
		mRotation = Radian;
	}

	void SetOffset(const FVector2& Offset)
	{
		mOffset = Offset;
	}

public:
	void Update(float DeltaTime);

	//Loop-> 반복이 아닐때 애니메이션을 재실행하기 위한 함수
	void PlayAnimation();
	void Clear();

	CAnimation2DSequence* Clone();

private:
	void CallNotify();

public:
	template<typename T>
	void AddNotify(const std::string& Name, int Frame, T* Object, void(T::* Func)())
	{
		FAnimation2DNotify Notify;
		Notify.Name = Name;
		Notify.Frame = Frame;
		Notify.Function = std::bind(Func, Object);

		mNotifyArray.push_back(Notify);
	}

	template<typename T>
	void SetFinishFunction(T* Object, void(T::* Func)())
	{
		mFinishFunction = std::bind(Func, Object);
	}

	//람다/std::function을 그대로 넘기고 싶을 때 쓴다.
	void AddNotify(const std::string& Name, int Frame, const std::function<void()>& Func)
	{
		FAnimation2DNotify Notify;
		Notify.Name = Name;
		Notify.Frame = Frame;
		Notify.Function = Func;

		mNotifyArray.push_back(Notify);
	}

	void SetFinishFunction(const std::function<void()>& Func)
	{
		mFinishFunction = Func;
	}
};

