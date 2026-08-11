#pragma once

//C++ 시간 관련 라이브러리
#include <chrono>
#include <unordered_map>
#include <functional>

//정해진 시간에 도달하면
//콜백함수를 호출하는 시스템
//직접 deltatime을 더해줄 필요없이 간단하게 시간을 정해서 액션을 실행해준다.

//타이머 식별용 핸들 구조체
struct FTimerHandle
{
private:
	//각 타이머의 식별자
	//string으로 사용할때 이름을 계속 바꿔줘야한다.
	//따라서 간단하게 정수형으로 ID를 증가시켜 각 타이머가 작동되도록 해준다.
	uint64_t ID = 0;

public:
	FTimerHandle() = default;
	FTimerHandle(uint64_t InitID)
		: ID(InitID)
	{

	}

	bool IsValid() const
	{
		return ID != 0;
	}

	void InValidate()
	{
		ID = 0;
	}

	uint64_t GetID() const
	{
		return ID;
	}

	bool operator==(const FTimerHandle& rhs) const
	{
		return ID == rhs.ID;
	}

	bool operator!=(const FTimerHandle& rhs) const
	{
		return ID != rhs.ID;
	}

};

//내부 타이머 구조체
struct FTimerInfo
{
	FTimerHandle Handle;
	float Time = 0.f; //목표 시간
	float ElapsedTime = 0.f; //누적 시간
	bool Loop = false;
	std::function<void()> Callback;
};

class CTimeManager
{
private:
	//time_point : 시점
	//duration : 기간
	//time_point1, time_point2 : 특정 시점을 가진 타임포인트 변수
	//ex)time_point1 : 26-07-01, time_point2 : 26-07-04
	//time_point2 - time_point1 -> 3일 기간 <- duration

	//high_resolution_clock 정밀한 시계도구
	//queryperformancecounter <- 이것도 정밀한 시계도구
	static std::chrono::high_resolution_clock::time_point mPrevTime;

	static float mDeltaTime;

	static uint64_t mNextTimerID;
	static std::unordered_map<uint64_t, FTimerInfo> mTimerMap;

public:
	static bool Init();
	static void Update();

	static float GetDeltaTime()
	{
		return mDeltaTime;
	}

	//타이머 삭제
	static void ClearTimer(FTimerHandle& Handle);

public:
	template<typename T>
	static FTimerHandle SetTimer(float Time, bool Loop, T* Object, void (T::* Func)())
	{
		FTimerHandle Handle(mNextTimerID++);

		FTimerInfo Info;
		Info.Handle = Handle;
		Info.Time = Time;
		Info.ElapsedTime = 0.f;
		Info.Loop = Loop;
		Info.Callback = std::bind(Func, Object);

		mTimerMap[Handle.GetID()] = Info;

		return Handle;
	}

};

