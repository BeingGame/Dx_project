#include "DirectionInputComponent.h"
#include "ActionStateComponent.h"

#include "World/Actor.h"
#include "World/World.h"
#include "World/Input.h"
#include "World/MovementComponent.h"

#include <fstream>
#include <string>

CDirectionInputComponent::CDirectionInputComponent()
{}

CDirectionInputComponent::CDirectionInputComponent(const CDirectionInputComponent& src)
    : CActorComponent(src)
{
    mDoubleTapTime = src.mDoubleTapTime;
    mWalkSpeed     = src.mWalkSpeed;
    mRunSpeed      = src.mRunSpeed;
    mRunVertScale  = src.mRunVertScale;
    mRunDiagVertScale = src.mRunDiagVertScale;
    mAttackKey     = src.mAttackKey;
    mJumpKey       = src.mJumpKey;
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
    // 공격키는 인스펙터에서 바뀔 수 있다. 같은 이름으로 다시 등록하면 무시되므로
    // 이름 뒤에 키 코드를 붙여 키마다 다른 이름이 되게 한다.
    Input->AddBindKey("Attack_" + std::to_string((int)mAttackKey), mAttackKey);

    // 점프키도 인스펙터에서 바뀔 수 있다. 같은 이유로 이름에 키 코드를 붙인다.
    Input->AddBindKey("Jump_" + std::to_string((int)mJumpKey), mJumpKey);

    mKeysRegistered = true;
}

void CDirectionInputComponent::BindFromOwner()
{
    auto Owner = mOwner.lock();
    if (!Owner) return;

    for (const auto& Comp : Owner->GetActorCompList())
    {
        if (mActionState.expired())
        {
            auto ActionStateComp = std::dynamic_pointer_cast<CActionStateComponent>(Comp);
            if (ActionStateComp) mActionState = ActionStateComp;
        }

        if (mMovement.expired())
        {
            auto MovementComp = std::dynamic_pointer_cast<CMovementComponent>(Comp);
            if (MovementComp) mMovement = MovementComp;
        }
    }
}

void CDirectionInputComponent::UpdateDoubleTap(unsigned char Key, bool bPressed, bool bHeld)
{
    // 달리던 키에서 손을 뗐는지는 여기서 보지 않는다.
    // 위/아래가 달리기를 이어받을 수 있어서, 네 키를 다 읽은 뒤에 Update가 판단한다.

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

    // 위/아래는 예외다. 달리는 도중 방향만 바꾸는 입력으로 보고 달리기를 유지한다.
    const bool bVertical = (Key == VK_UP || Key == VK_DOWN);

    if (mRunning && Key != mRunKey && !bVertical)
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

    if (mActionState.expired() || mMovement.expired())
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

    // 달리기 판정은 네 키를 다 읽은 뒤에 해야 해서 눌린 상태를 모아둔다.
    bool bHeldNow[4] = {};

    for (int i = 0; i < 4; ++i)
    {
        const FDirKey& DirKey = Keys[i];

        bool bPressed = Input->GetKey(DirKey.Key, EInputType::Press);
        bool bHeld    = Input->GetKey(DirKey.Key, EInputType::Hold);

        bHeldNow[i] = bHeld;

        UpdateDoubleTap(DirKey.Key, bPressed, bHeld);

        if (bHeld)
        {
            mMoveDir.x += DirKey.X;
            mMoveDir.y += DirKey.Y;
        }
    }

    // ── 달리는 중의 위/아래 ──
    // 위/아래는 달리기를 끊지 않는다. 누르고 있는 동안에는 좌우를 무시하고
    // 위/아래로만 달린다. 달리던 키에서 손을 떼도 위/아래를 누르고 있으면
    // 달리기는 그 키가 이어받는다.
    if (mRunning)
    {
        // Keys[2]=위, Keys[3]=아래
        unsigned char VertKey = 0;

        if (bHeldNow[2])      VertKey = VK_UP;
        else if (bHeldNow[3]) VertKey = VK_DOWN;

        // 달리기를 시작한 키를 아직 누르고 있는지
        bool bRunKeyHeld = false;

        for (int i = 0; i < 4; ++i)
        {
            if (Keys[i].Key == mRunKey)
            {
                bRunKeyHeld = bHeldNow[i];
                break;
            }
        }

        if (VertKey != 0)
        {
            // 달리던 키가 좌우이고 아직 누르고 있으면: 좌우 달리기를 유지한 채
            // 위/아래를 조금만 섞어 얕은 대각선으로 흐르게 한다. (좌우가 주, 상하가 덤)
            const bool bHorizRun = (mRunKey == VK_LEFT || mRunKey == VK_RIGHT);

            if (bRunKeyHeld && bHorizRun)
            {
                // mMoveDir.x(좌우 달리기 방향)는 그대로 두고 위/아래만 작게 줄인다.
                // 뒤에서 방향을 정규화하므로, 비율이 작을수록 얕은 대각선이 된다.
                mMoveDir.y *= mRunDiagVertScale;
            }
            else
            {
                // 좌우 달리기 키에서 손을 뗐다: 예전처럼 위/아래가 달리기를 이어받아
                // 위/아래로만 달린다. (좌우가 0이라 바라보는 방향도 유지된다)
                mMoveDir.x = 0.f;

                if (!bRunKeyHeld)
                {
                    mRunKey = VertKey;
                }
            }
        }
        else if (!bRunKeyHeld)
        {
            // 달리던 키를 놨고 이어받을 위/아래도 없다.
            mRunning = false;
            mRunKey  = 0;
        }
    }

    // 아무 방향도 안 누르면 달리기도 풀린다.
    if (mMoveDir.x == 0.f && mMoveDir.y == 0.f)
    {
        mRunning = false;
        mRunKey  = 0;
    }

    // ── 상태 컴포넌트에 의도를 넘긴다 ──
    // 좌우 반전과 상태 전환 판단은 전부 저쪽 몫이다.
    auto ActionState = mActionState.lock();

    if (ActionState)
    {
        ActionState->SetMoveInput(mMoveDir, mRunning);

        // 공격키를 누르고 있는지도 넘겨준다. (3타 모아치기가 이걸로 붙잡아둔다)
        ActionState->SetAttackHeld(Input->GetKey(mAttackKey, EInputType::Hold));

        // 공격 요청. 받아줄지 말지는 저쪽이 우선순위를 보고 정한다.
        if (Input->GetKey(mAttackKey, EInputType::Press))
        {
            ActionState->RequestAttack();
        }

        // 점프 요청. 점프는 공격보다 우선순위가 높게 잡혀 있어서
        // 공격 중에 눌러도 그 자리에서 점프로 빠져나간다.
        if (Input->GetKey(mJumpKey, EInputType::Press))
        {
            ActionState->RequestState(EActionState::Jump);
        }
    }

    // ── 실제 이동 ──
    // 상태 컴포넌트가 이동을 막고 있으면(공격/피격 중) 아예 넘기지 않는다.
    // 이동 컴포넌트는 매 프레임 방향을 새로 받으므로, 안 주면 그대로 멈춘다.
    bool bCanMove = !ActionState || ActionState->CanMove();

    if (auto Movement = mMovement.lock(); Movement && bCanMove)
    {
        // 좌우 달리기 키를 놓고 위/아래가 달리기를 이어받으면(좌우=0) 위/아래로만
        // 가는 상태가 된다. 그때는 달리기 속도 대신 걷기 속도의 배율을 쓴다.(기본 2배)
        // 좌우를 유지한 얕은 대각선(mMoveDir.x != 0)은 그냥 달리기 속도로 간다.
        const bool bRunVertical = mRunning
                               && (mMoveDir.x == 0.f)
                               && (mMoveDir.y != 0.f);

        // 좌우를 유지한 얕은 대각선으로 달리는 중인지. (좌우·상하가 둘 다 있음)
        const bool bRunDiagonal = mRunning
                               && (mMoveDir.x != 0.f)
                               && (mMoveDir.y != 0.f);

        float Speed = bRunVertical ? (mWalkSpeed * mRunVertScale)
                                   : (mRunning ? mRunSpeed : mWalkSpeed);

        // 공중에서는 배율을 곱한다. 배율이 1이면 땅에서와 같은 속도다.
        // 속도를 넘기는 건 이쪽이지만 값은 상태 컴포넌트가 들고 있다.
        // (점프 설정을 한곳에 모아두려고 그렇게 뒀다)
        if (ActionState && ActionState->IsInAir())
        {
            Speed *= ActionState->GetJumpMoveScale();
        }

        if (mMoveDir.x != 0.f || mMoveDir.y != 0.f)
        {
            // 속도 벡터 = 속도 × 단위방향.
            FVector2 Vel = mMoveDir;
            Vel.Normalize();
            Vel *= Speed;

            // 대각으로 달릴 때는 좌우로 가던 속도를 반으로 줄인다.
            // 위/아래 성분은 그대로 두고 좌우 성분만 절반으로 깎는다.
            if (bRunDiagonal)
            {
                Vel.x *= 0.5f;
            }

            // 이동 컴포넌트는 속도(크기)와 단위방향을 따로 받으므로 다시 분해한다.
            Movement->SetSpeed(Vel.Length());

            FVector3 Dir(Vel.x, Vel.y, 0.f);
            Dir.Normalize();
            Movement->SetMoveDir(Dir);
        }
        else
        {
            // 입력이 없어도 속도는 갱신해 둔다. (방향은 이전 값을 유지)
            Movement->SetSpeed(Speed);
        }
    }
}

void CDirectionInputComponent::Save(std::ofstream& File) const
{
    CActorComponent::Save(File);

    File << "DoubleTapTime=" << mDoubleTapTime << "\n";
    File << "WalkSpeed="     << mWalkSpeed     << "\n";
    File << "RunSpeed="      << mRunSpeed      << "\n";
    File << "RunVertScale=" << mRunVertScale  << "\n";
    File << "RunDiagVertScale=" << mRunDiagVertScale << "\n";
    File << "AttackKey="     << (int)mAttackKey << "\n";
    File << "JumpKey="       << (int)mJumpKey   << "\n";
}

void CDirectionInputComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
    CActorComponent::Load(Props);

    auto GetF = [&Props](const char* Key, float& Out)
    {
        auto Found = Props.find(Key);
        if (Found == Props.end()) return;
        try { Out = std::stof(Found->second); }
        catch (...) {}
    };

    GetF("DoubleTapTime", mDoubleTapTime);
    GetF("WalkSpeed",     mWalkSpeed);
    GetF("RunSpeed",      mRunSpeed);

    //나중에 생긴 항목이라 예전 월드에는 없다. 없으면 기본값(2배)을 쓴다.
    GetF("RunVertScale",  mRunVertScale);

    //달리기 중 위/아래를 섞는 대각선 비율. 예전 월드엔 없으니 없으면 기본값(0.35)을 쓴다.
    GetF("RunDiagVertScale", mRunDiagVertScale);

    if (auto Found = Props.find("AttackKey"); Found != Props.end())
    {
        try { mAttackKey = (unsigned char)std::stoi(Found->second); }
        catch (...) {}
    }

    //예전에 저장된 월드에는 없다. 없으면 기본값(Space)을 그대로 쓴다.
    if (auto Found = Props.find("JumpKey"); Found != Props.end())
    {
        try { mJumpKey = (unsigned char)std::stoi(Found->second); }
        catch (...) {}
    }

    BindFromOwner();
}
