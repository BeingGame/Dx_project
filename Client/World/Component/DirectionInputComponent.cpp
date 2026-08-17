#include "DirectionInputComponent.h"
#include "AnimStateComponent.h"

#include "World/Actor.h"
#include "World/World.h"
#include "World/Input.h"
#include "World/MovementComponent.h"

#include <fstream>

CDirectionInputComponent::CDirectionInputComponent()
{}

CDirectionInputComponent::CDirectionInputComponent(const CDirectionInputComponent& src)
    : CActorComponent(src)
{
    mDoubleTapTime = src.mDoubleTapTime;
    mWalkSpeed     = src.mWalkSpeed;
    mRunSpeed      = src.mRunSpeed;
}

CDirectionInputComponent::~CDirectionInputComponent()
{}

bool CDirectionInputComponent::Init()
{
    CActorComponent::Init();
    return true;
}

void CDirectionInputComponent::RegisterKeys()
{
    if (mKeysRegistered)
    {
        return;
    }

    auto World = mWorld.lock();
    if (!World) return;

    auto Input = World->GetInput().lock();
    if (!Input) return;

    // 반드시 VK 코드로 등록해야 한다.
    // DIK 코드를 넘기면 ConvertKey가 매칭에 실패해 0xff를 돌려주고,
    // 그 키는 영원히 눌리지 않은 상태가 된다.
    Input->AddBindKey("DirLeft",  VK_LEFT);
    Input->AddBindKey("DirRight", VK_RIGHT);
    Input->AddBindKey("DirUp",    VK_UP);
    Input->AddBindKey("DirDown",  VK_DOWN);

    mKeysRegistered = true;
}

void CDirectionInputComponent::BindFromOwner()
{
    auto Owner = mOwner.lock();
    if (!Owner) return;

    for (const auto& Comp : Owner->GetActorCompList())
    {
        if (mAnimState.expired())
        {
            auto AS = std::dynamic_pointer_cast<CAnimStateComponent>(Comp);
            if (AS) mAnimState = AS;
        }

        if (mMovement.expired())
        {
            auto MV = std::dynamic_pointer_cast<CMovementComponent>(Comp);
            if (MV) mMovement = MV;
        }
    }
}

void CDirectionInputComponent::UpdateDoubleTap(unsigned char Key, bool bPressed, bool bHeld)
{
    // 달리는 중이던 키에서 손을 떼면 달리기가 끝난다.
    if (mRunning && Key == mRunKey && !bHeld)
    {
        mRunning = false;
        mRunKey  = 0;
    }

    if (!bPressed)
    {
        return;
    }

    // 같은 키를 짧은 간격으로 다시 눌렀으면 달리기.
    if (Key == mLastTapKey && (mTimeAccum - mLastTapTime) <= mDoubleTapTime)
    {
        mRunning    = true;
        mRunKey     = Key;
        mLastTapKey = 0;      // 세 번째 입력이 또 걸리지 않게 초기화
        return;
    }

    // 첫 번째 탭으로 기록. 다른 방향을 눌렀으면 달리기는 취소된다.
    mLastTapKey  = Key;
    mLastTapTime = mTimeAccum;

    if (mRunning && Key != mRunKey)
    {
        mRunning = false;
        mRunKey  = 0;
    }
}

void CDirectionInputComponent::Update(float DeltaTime)
{
    CActorComponent::Update(DeltaTime);

    mTimeAccum += DeltaTime;

    RegisterKeys();

    if (mAnimState.expired() || mMovement.expired())
    {
        BindFromOwner();
    }

    auto World = mWorld.lock();
    if (!World) return;

    auto Input = World->GetInput().lock();
    if (!Input) return;

    struct FDirKey { unsigned char Key; float X; float Y; };

    // 화면 기준. 월드 Y는 위가 +이므로 위쪽 키가 +Y다.
    const FDirKey Keys[4] =
    {
        { VK_LEFT,  -1.f,  0.f },
        { VK_RIGHT,  1.f,  0.f },
        { VK_UP,     0.f,  1.f },
        { VK_DOWN,   0.f, -1.f },
    };

    mMoveDir = FVector2(0.f, 0.f);

    for (const auto& DK : Keys)
    {
        bool bPressed = Input->GetKey(DK.Key, EInputType::Press);
        bool bHeld    = Input->GetKey(DK.Key, EInputType::Hold);

        UpdateDoubleTap(DK.Key, bPressed, bHeld);

        if (bHeld)
        {
            mMoveDir.x += DK.X;
            mMoveDir.y += DK.Y;
        }
    }

    // 아무 방향도 안 누르면 달리기도 풀린다.
    if (mMoveDir.x == 0.f && mMoveDir.y == 0.f)
    {
        mRunning = false;
        mRunKey  = 0;
    }

    // ── 애니메이션 쪽에 넘긴다 ──
    // 좌우 반전과 상태 전환은 저쪽에서 판단한다.
    if (auto AnimState = mAnimState.lock())
    {
        AnimState->SetMoveInput(mMoveDir, mRunning);
    }

    // ── 실제 이동 ──
    if (auto Movement = mMovement.lock())
    {
        Movement->SetSpeed(mRunning ? mRunSpeed : mWalkSpeed);

        if (mMoveDir.x != 0.f || mMoveDir.y != 0.f)
        {
            FVector3 Dir(mMoveDir.x, mMoveDir.y, 0.f);
            Dir.Normalize();
            Movement->SetMoveDir(Dir);
        }
    }
}

void CDirectionInputComponent::Save(std::ofstream& File) const
{
    CActorComponent::Save(File);

    File << "DoubleTapTime=" << mDoubleTapTime << "\n";
    File << "WalkSpeed="     << mWalkSpeed     << "\n";
    File << "RunSpeed="      << mRunSpeed      << "\n";
}

void CDirectionInputComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
    CActorComponent::Load(Props);

    auto GetF = [&Props](const char* Key, float& Out)
    {
        auto It = Props.find(Key);
        if (It == Props.end()) return;
        try { Out = std::stof(It->second); }
        catch (...) {}
    };

    GetF("DoubleTapTime", mDoubleTapTime);
    GetF("WalkSpeed",     mWalkSpeed);
    GetF("RunSpeed",      mRunSpeed);

    BindFromOwner();
}
