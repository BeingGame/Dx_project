
#include "TimeManager.h"

std::chrono::high_resolution_clock::time_point CTimeManager::mPrevTime;

float CTimeManager::mDeltaTime = 0.f;

uint64_t CTimeManager::mNextTimerID = 1;
std::unordered_map<uint64_t, FTimerInfo> CTimeManager::mTimerMap;


//델타 타임
//컴퓨터 성능에 관계없이 항상 일정한 속도로 게임이 실행되도록 시간을 만든다.
//이전 프레임과 현재 프레임을 이용해서 두개를 뺀값으로 사용된다.

bool CTimeManager::Init()
{
    mPrevTime = std::chrono::high_resolution_clock::now();
    mNextTimerID = 1;
    mTimerMap.clear();

    return true;
}

void CTimeManager::Update()
{
    auto CurrentTime = std::chrono::high_resolution_clock::now();

    //이전 시간과 현재시간의 차를 구해 진행된 시간을 계산한다.
    std::chrono::duration<float> Time = CurrentTime - mPrevTime;
    mDeltaTime = Time.count();

    mPrevTime = CurrentTime;


    //타이머를 갱신해준다.
    if (mTimerMap.empty())
    {
        return;
    }

    //타이머 순회도중 타이머가 사라졌을때
    //무효화 방지
    std::vector<uint64_t> CurrentIDs;
    for (const auto& It : mTimerMap)
    {
        CurrentIDs.push_back(It.first);
    }

    for (uint64_t ID : CurrentIDs)
    {
        auto iter = mTimerMap.find(ID);

        if (iter == mTimerMap.end())
        {
            continue;
        }

        FTimerInfo& Timer = iter->second;
        Timer.ElapsedTime += mDeltaTime;


        if (Timer.ElapsedTime >= Timer.Time)
        {
            if (Timer.Callback)
            {
                Timer.Callback();
            }

            if (Timer.Loop)
            {
                Timer.ElapsedTime -= Timer.Time;
            }
            else
            {
                mTimerMap.erase(iter);
            }

        }

    }


}

void CTimeManager::ClearTimer(FTimerHandle& Handle)
{
    if (!Handle.IsValid())
    {
        return;
    }

    auto iter = mTimerMap.find(Handle.GetID());

    iter->second.Callback = nullptr;

    mTimerMap.erase(Handle.GetID());
    Handle.InValidate();
}
