#include "AnimStateComponent.h"

#include "World/Actor.h"
#include "World/Animation2DComponent.h"

#include <fstream>

CAnimStateComponent::CAnimStateComponent()
{
    // 기본값은 애님 에디터에서 만들어 둔 이름에 맞춰뒀다.
    mStateAnim[EAnimMoveState::Idle] = "Idle";
    mStateAnim[EAnimMoveState::Walk] = "PlayerWalk";
    mStateAnim[EAnimMoveState::Run]  = "PlayerRunning";
}

CAnimStateComponent::CAnimStateComponent(const CAnimStateComponent& src)
    : CActorComponent(src)
{
    for (int i = 0; i < EAnimMoveState::End; ++i)
        mStateAnim[i] = src.mStateAnim[i];

    mState        = src.mState;
    mPendingState = src.mPendingState;
    mFacingRight  = src.mFacingRight;
    mMinStateTime = src.mMinStateTime;
}

CAnimStateComponent::~CAnimStateComponent()
{}

bool CAnimStateComponent::Init()
{
    CActorComponent::Init();
    return true;
}

bool CAnimStateComponent::BindAnimCompFromOwner()
{
    auto Owner = mOwner.lock();

    if (!Owner)
    {
        return false;
    }

    for (const auto& Comp : Owner->GetActorCompList())
    {
        auto AnimComp = std::dynamic_pointer_cast<CAnimation2DComponent>(Comp);

        if (AnimComp)
        {
            mAnimComp = AnimComp;
            return true;
        }
    }

    return false;
}

EAnimMoveState::Type CAnimStateComponent::DecideState() const
{
    // 아주 작은 입력은 정지로 본다. (아날로그 입력이나 감속 잔여값 대비)
    float LenSq = mMoveDir.x * mMoveDir.x + mMoveDir.y * mMoveDir.y;

    if (LenSq < 0.0001f)
    {
        return EAnimMoveState::Idle;
    }

    return mWantRun ? EAnimMoveState::Run : EAnimMoveState::Walk;
}

void CAnimStateComponent::ApplyState(EAnimMoveState::Type State)
{
    if (State < 0 || State >= EAnimMoveState::End)
    {
        return;
    }

    const std::string& Name = mStateAnim[State];

    // 그 상태에 애니메이션을 지정하지 않았으면 지금 재생 중인 걸 그대로 둔다.
    if (Name.empty())
    {
        return;
    }

    auto AnimComp = mAnimComp.lock();

    if (!AnimComp)
    {
        return;
    }

    // 같은 애니메이션이면 다시 걸지 않는다. (프레임이 0으로 되감기는 걸 막는다)
    if (AnimComp->GetCurrentAnimationName() == Name)
    {
        return;
    }

    AnimComp->ChangeAnimation(Name);
}

void CAnimStateComponent::ApplyFacing()
{
    auto AnimComp = mAnimComp.lock();

    if (!AnimComp)
    {
        return;
    }

    const std::string Current = AnimComp->GetCurrentAnimationName();

    if (Current.empty())
    {
        return;
    }

    // 원본 스프라이트가 오른쪽을 보므로, 왼쪽을 볼 때만 좌우를 뒤집는다.
    AnimComp->SetSymmetry(Current, !mFacingRight);
}

void CAnimStateComponent::PlayOnce(const std::string& AnimName)
{
    if (AnimName.empty())
    {
        return;
    }

    auto AnimComp = mAnimComp.lock();

    if (!AnimComp)
    {
        return;
    }

    AnimComp->ChangeAnimation(AnimName);
    AnimComp->SetSymmetry(AnimName, !mFacingRight);

    // 이동 상태를 강제로 어긋나게 해서, 다음 프레임에 상태가 유지되더라도
    // ApplyState가 한 번 더 불려 원래 애니메이션으로 돌아오게 한다.
    mState = EAnimMoveState::End;
}

void CAnimStateComponent::Update(float DeltaTime)
{
    CActorComponent::Update(DeltaTime);

    if (mAnimComp.expired())
    {
        BindAnimCompFromOwner();
    }

    // ── 바라보는 방향 ──
    // 좌우 입력이 있을 때만 갱신한다.
    // 위/아래만 눌렀을 때는 바라보던 방향을 그대로 유지해야 하므로 건드리지 않는다.
    if (mMoveDir.x > 0.0001f)
    {
        mFacingRight = true;
    }
    else if (mMoveDir.x < -0.0001f)
    {
        mFacingRight = false;
    }

    // ── 상태 ──
    EAnimMoveState::Type Next = DecideState();

    mStateHold += DeltaTime;

    // 걷기↔달리기가 한두 프레임 만에 왔다갔다 하는 걸 막는다.
    if (Next != mState && mStateHold >= mMinStateTime)
    {
        mState     = Next;
        mStateHold = 0.f;
        ApplyState(mState);
    }
    else if (mState == EAnimMoveState::End)
    {
        // PlayOnce로 다른 애니메이션이 걸려 있던 상태. 이동 상태로 복귀시킨다.
        mState     = Next;
        mStateHold = 0.f;
        ApplyState(mState);
    }

    ApplyFacing();

    // 입력은 매 프레임 새로 받는다. 안 들어오면 정지로 간주해야 하기 때문이다.
    mMoveDir = FVector2(0.f, 0.f);
    mWantRun = false;
}

void CAnimStateComponent::Save(std::ofstream& File) const
{
    CActorComponent::Save(File);

    File << "IdleAnim=" << mStateAnim[EAnimMoveState::Idle] << "\n";
    File << "WalkAnim=" << mStateAnim[EAnimMoveState::Walk] << "\n";
    File << "RunAnim="  << mStateAnim[EAnimMoveState::Run]  << "\n";
    File << "MinStateTime=" << mMinStateTime << "\n";
}

void CAnimStateComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
    CActorComponent::Load(Props);

    auto Get = [&Props](const char* Key, std::string& Out)
    {
        auto It = Props.find(Key);
        if (It != Props.end()) Out = It->second;
    };

    Get("IdleAnim", mStateAnim[EAnimMoveState::Idle]);
    Get("WalkAnim", mStateAnim[EAnimMoveState::Walk]);
    Get("RunAnim",  mStateAnim[EAnimMoveState::Run]);

    if (auto It = Props.find("MinStateTime"); It != Props.end())
    {
        try { mMinStateTime = std::stof(It->second); }
        catch (...) {}
    }

    BindAnimCompFromOwner();
}
