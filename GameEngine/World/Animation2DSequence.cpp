#include "Animation2DSequence.h"

CAnimation2DSequence::CAnimation2DSequence()
{}

CAnimation2DSequence::CAnimation2DSequence(const CAnimation2DSequence & src)
{
    mAnimation = src.mAnimation;
    mFrame = src.mFrame;
    mTime = src.mTime;
    mFrameTime = src.mFrameTime;
    mPlayTime = src.mPlayTime;
    mPlayRate = src.mPlayRate;
    mLoop = src.mLoop;
    mReverse = src.mReverse;
    mSymmetry = src.mSymmetry;
    mEnd = src.mEnd;
}

CAnimation2DSequence::CAnimation2DSequence(CAnimation2DSequence && src) noexcept
{
    mAnimation = src.mAnimation;
    mFrame = src.mFrame;
    mTime = src.mTime;
    mFrameTime = src.mFrameTime;
    mPlayTime = src.mPlayTime;
    mPlayRate = src.mPlayRate;
    mLoop = src.mLoop;
    mReverse = src.mReverse;
    mSymmetry = src.mSymmetry;
    mEnd = src.mEnd;
}

CAnimation2DSequence::~CAnimation2DSequence()
{}

void CAnimation2DSequence::Update(float DeltaTime)
{
    //애니메이션을 업데이트해준다.
    //시퀀스가지고있는 변수를 통해 애니메이션이 어떤 식으로 흘러갈지 업데이트해준다.

    if (mAnimation.expired())
    {
        return;
    }

    int FrameCount = GetFrameCount();

    if (FrameCount <= 0)
    {
        return;
    }

    if (!mEnd)
    {
        auto Anim = mAnimation.lock();

        //머무는 시간은 프레임마다 따로 정해진다. (FTextureFrame::Duration)
        //시퀀스 전체 재생 시간은 이 값들의 합이다.
        if (mFrame >= 0 && mFrame < FrameCount)
        {
            mFrameTime = Anim->GetFrame(mFrame).Duration;
        }

        if (mFrameTime < 0.001f)
        {
            mFrameTime = 0.001f;
        }

        mTime += DeltaTime * mPlayRate;

        if (mTime >= mFrameTime)
        { 
            //애니메이션이 렉걸려서 프리징이 걸리더라도
            //애니메이션을 정상적으로 작동시킨다.
            //오류를 방지하기 위함
            mTime -= mFrameTime;

            if (mReverse)
            {
                --mFrame;

                if (mFrame < 0)
                {
                    if (mFinishFunction)
                    {
                        mFinishFunction();
                    }

                    if (mLoop)
                    {
                        mFrame = GetFrameCount() - 1;

                        size_t Size = mNotifyArray.size();

                        for (size_t i = 0; i < Size; ++i)
                        {
                            mNotifyArray[i].Call = false;
                        }
                    }
                    else
                    {
                        mFrame = 0;
                        mEnd = true;
                    }
                }

            }
            else
            {
                ++mFrame;

                if (mFrame == GetFrameCount())
                {
                    if (mFinishFunction)
                    {
                        mFinishFunction();
                    }

                    if (mLoop)
                    {
                        mFrame = 0;

                        size_t Size = mNotifyArray.size();

                        for (size_t i = 0; i < Size; ++i)
                        {
                            mNotifyArray[i].Call = false;
                        }
                    }
                    else
                    {
                        mFrame = GetFrameCount() - 1;
                        mEnd = true;
                    }
                }
            }
        }
    }

    CallNotify();
}

void CAnimation2DSequence::PlayAnimation()
{
    mEnd = false;
    Clear();
}

void CAnimation2DSequence::Clear()
{
    //애니메이션을 초기화해서 재실행될수있도록 만들어준다.

    if (!mAnimation.expired())
    {
        if (mReverse)
        {
            mFrame = GetFrameCount() - 1;
        }
        else
        {
            mFrame = 0;
        }

        size_t Size = mNotifyArray.size();

        for (size_t i = 0; i < Size; ++i)
        {
            mNotifyArray[i].Call = false;
        }

        //실행시간 초기화
        mTime = 0.f;
    }
}

CAnimation2DSequence* CAnimation2DSequence::Clone()
{
    return new CAnimation2DSequence(*this);
}

void CAnimation2DSequence::CallNotify()
{
    //Ranged-for 문
    for (auto& It : mNotifyArray)
    {
        if (It.Frame == mFrame && !It.Call)
        {
            It.Call = true;

            if (It.Function)
            {
                It.Function();
            }
        }
    }
}
