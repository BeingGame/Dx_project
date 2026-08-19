#include "ActionStateComponent.h"

#include "World/Actor.h"
#include "World/Animation2DComponent.h"
#include "World/Animation2DSequence.h"
#include "World/MovementComponent.h"
#include "HeightComponent.h"

#include "LogManager.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

namespace
{
    // 상태 이름 — 저장 파일에 이 이름으로 적힌다.
    // enum 순서를 바꿔도 저장된 데이터가 깨지지 않도록 이름으로 찾는다.
    const char* sStateNames[EActionState::End] =
    {
        "Idle", "Walk", "Run",
        "Attack1", "Attack2", "Attack3", "Jump", "Hit", "Dash", "Dead",
        "Airborne", "Down", "GetUp"
    };
}

CActionStateComponent::CActionStateComponent()
{
    // 기본값은 애님 에디터에서 만들어 둔 이름에 맞춰뒀다.
    // 우선순위: 사망(100) > 피격(20) > 대쉬(15) > 공격(10) > 이동계(0)

    auto SetStateDef = [this](EActionState::Type State, const char* Anim, int Priority,
                      bool bLoop, bool bCanMove, bool bCanTurn, float CancelTime)
    {
        mDef[State].Anim       = Anim;
        mDef[State].Priority   = Priority;
        mDef[State].bLoop      = bLoop;
        mDef[State].bCanMove   = bCanMove;
        mDef[State].bCanTurn   = bCanTurn;
        mDef[State].CancelTime = CancelTime;
    };

    //           상태                    애니메이션        우선  Loop  Move  Turn  Cancel
    SetStateDef(EActionState::Idle,    "Idle",          0,   true,  true,  true, -1.f);
    SetStateDef(EActionState::Walk,    "PlayerWalk",    0,   true,  true,  true, -1.f);
    SetStateDef(EActionState::Run,     "PlayerRunning", 0,   true,  true,  true, -1.f);

    SetStateDef(EActionState::Attack1, "BasicAttack",  10,  false, false, false, 0.15f);
    SetStateDef(EActionState::Attack2, "BasicAttack2", 10,  false, false, false, 0.15f);
    SetStateDef(EActionState::Attack3, "BasicAttack3", 10,  false, false, false, 0.15f);

    // 3타는 모아치기다. 공격키를 누르고 있으면 첫 프레임에서 멈춰 있다가
    // 손을 떼는 순간 나머지 프레임이 재생된다.
    mDef[EActionState::Attack3].bChargeHold = true;

    // 기본 공격은 바라보는 쪽으로 조금씩 파고든다.
    // 타수가 올라갈수록 크게 나가도록 해서 콤보가 앞으로 밀고 들어가는 느낌을 준다.
    // 이 월드는 걷기 속도가 0.3(초당 유닛)이라 0.06이면 걸음의 5분의 1 정도다.
    mDef[EActionState::Attack1].MoveDist = 0.06f;
    mDef[EActionState::Attack2].MoveDist = 0.09f;
    mDef[EActionState::Attack3].MoveDist = 0.14f;

    // 전진은 타수가 시작하는 순간에만 짧게 일어난다.
    // 애니메이션 길이 전체에 걸쳐 밀면 동작 내내 미끄러져서 이동으로 보인다.
    // 3타는 더 멀리 나가므로 시간도 조금 더 준다.
    mDef[EActionState::Attack1].StepTime = 0.10f;
    mDef[EActionState::Attack2].StepTime = 0.10f;
    mDef[EActionState::Attack3].StepTime = 0.14f;

    // 점프. 공중에서도 방향키가 먹도록 이동과 좌우 반전을 열어둔다.
    // 우선순위는 공격(10)보다 높고 피격(20)보다 낮다.
    // 즉 공격 중에 점프로 빠져나갈 수는 있지만, 점프는 맞아야만 끊긴다.
    SetStateDef(EActionState::Jump,    "PlayerJump",   12,  false, true,  true,  -1.f);

    SetStateDef(EActionState::Hit,     "PlayerHit",    20,  false, false, false, -1.f);

    //대쉬 전용 애니메이션은 아직 없다. 이름을 비워두면 그 상태는 요청해도 거절된다.
    SetStateDef(EActionState::Dash,    "",             15,  false, false, false, -1.f);

    SetStateDef(EActionState::Dead,    "PlayerDown",  100,  false, false, false, -1.f);

    // ── 띄우기 계열 ──
    // 우선순위가 피격(20)보다 높아서 뜬 뒤에는 경직으로 끊기지 않는다.
    // Down/GetUp이 더 높은 건 다운 무적이다. (맞아도 요청이 거절된다)
    //
    // 전용 애니메이션이 아직 없어서 있는 걸 돌려 쓴다.
    //   뜬 상태 → 피격 동작의 마지막 프레임에 멈춰 있는다
    //   눕기    → PlayerDown
    //   기상    → PlayerDown 역재생
    SetStateDef(EActionState::Airborne, "PlayerHit",  25,  false, false, false, -1.f);
    SetStateDef(EActionState::Down,     "PlayerDown", 30,  false, false, false, -1.f);
    SetStateDef(EActionState::GetUp,    "PlayerDown", 30,  false, false, false, -1.f);

    mDef[EActionState::GetUp].bReverse = true;

    // ── 이 동작이 맞은 쪽에게 주는 힘 ──
    // 1·2타는 밀기만 하고, 3타에서 띄운다. (콤보를 마무리로 끊는 느낌)
    mDef[EActionState::Attack1].KnockPower = 0.25f;
    mDef[EActionState::Attack2].KnockPower = 0.35f;
    mDef[EActionState::Attack3].KnockPower = 0.60f;

    mDef[EActionState::Attack3].LaunchPower = 1.60f;
}

CActionStateComponent::CActionStateComponent(const CActionStateComponent& src)
    : CActorComponent(src)
{
    for (int i = 0; i < EActionState::End; ++i)
        mDef[i] = src.mDef[i];

    mMinStateTime   = src.mMinStateTime;
    mActionTimeout  = src.mActionTimeout;
    mComboResetTime = src.mComboResetTime;
    mComboMoveLimit = src.mComboMoveLimit;
    mFacingRight    = src.mFacingRight;

    mJumpHeight     = src.mJumpHeight;
    mJumpTime       = src.mJumpTime;
    mJumpMoveScale  = src.mJumpMoveScale;
    mDownTime       = src.mDownTime;
}

CActionStateComponent::~CActionStateComponent()
{}

bool CActionStateComponent::Init()
{
    CActorComponent::Init();
    return true;
}

const char* CActionStateComponent::GetStateName(EActionState::Type State)
{
    return (State >= 0 && State < EActionState::End) ? sStateNames[State] : "";
}

EActionState::Type CActionStateComponent::FindStateByName(const std::string& Name)
{
    for (int i = 0; i < EActionState::End; ++i)
    {
        if (Name == sStateNames[i])
            return (EActionState::Type)i;
    }

    return EActionState::End;
}

bool CActionStateComponent::BindAnimCompFromOwner()
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

bool CActionStateComponent::BindMovementFromOwner()
{
    auto Owner = mOwner.lock();

    if (!Owner)
    {
        return false;
    }

    for (const auto& Comp : Owner->GetActorCompList())
    {
        auto Movement = std::dynamic_pointer_cast<CMovementComponent>(Comp);

        if (Movement)
        {
            mMovement = Movement;
            return true;
        }
    }

    return false;
}

float CActionStateComponent::GetAnimDuration(const std::string& Anim) const
{
    if (Anim.empty())
    {
        return 0.f;
    }

    auto AnimComp = mAnimComp.lock();

    if (!AnimComp)
    {
        return 0.f;
    }

    auto Seq = AnimComp->FindSequence(Anim).lock();

    if (!Seq)
    {
        return 0.f;
    }

    const float Rate = Seq->GetPlayRate();
    const float Time = Seq->GetPlayTime();

    if (Rate <= 0.f || Time <= 0.f)
    {
        return 0.f;
    }

    return Time / Rate;
}

// ── 상태 전진 ────────────────────────────────────────────────────────────────
//
// 공격은 CanMove가 꺼져 있어서 입력 컴포넌트가 이동을 아예 안 넘긴다.
// 그래서 "1타에 조금 파고들기" 같은 건 상태를 쥐고 있는 이쪽이 직접 밀어야 한다.
//
// 이동 컴포넌트는 방향만 받고 거리는 speed * DeltaTime으로 스스로 정한다.
// 그래서 미는 동안에는 속도를 잠깐 우리 값으로 바꿔 쓰고, 끝나면 원래 값으로 돌려둔다.
// (CanMove가 켜진 상태에 MoveDist를 넣으면 입력 컴포넌트가 매 프레임 속도를
//  다시 덮어쓰므로 서로 싸운다. 전진은 이동이 잠긴 상태에서만 쓰는 게 맞다)

void CActionStateComponent::BeginStep(EActionState::Type State)
{
    // 이전 동작이 밀고 있던 중이면 먼저 정리한다.
    EndStep();

    const FActionStateDef& StateDef = mDef[State];

    if (StateDef.MoveDist <= 0.f)
    {
        return;
    }

    if (mMovement.expired())
    {
        BindMovementFromOwner();
    }

    auto Movement = mMovement.lock();

    if (!Movement)
    {
        return;
    }

    // 상태에 들어간 직후부터 StepTime 동안만 민다.
    // 애니메이션 길이 전체에 걸쳐 밀면 동작 내내 미끄러져서 전진이 아니라
    // 이동으로 보인다. 시작할 때 잠깐 치고 나가고 나머지는 제자리에서 친다.
    const float AnimDuration = GetAnimDuration(StateDef.Anim);

    float Duration = StateDef.StepTime;

    // 0 이하로 두면 예전처럼 애니메이션 내내 나눠 민다.
    if (Duration <= 0.f)
    {
        Duration = AnimDuration;
    }

    // 동작보다 오래 밀 수는 없다. 넘치면 남은 거리를 못 가고 잘린다.
    if (AnimDuration > 0.f && Duration > AnimDuration)
    {
        Duration = AnimDuration;
    }

    // 시퀀스를 못 찾으면(이름이 비었거나 아직 안 붙었으면) 짧게 잡아둔다.
    if (Duration <= 0.f)
    {
        Duration = 0.2f;
    }

    // 한두 프레임 만에 순간이동하는 걸 막는 하한.
    if (Duration < 0.03f)
    {
        Duration = 0.03f;
    }

    mStepRemain = StateDef.MoveDist;
    mStepSpeed  = StateDef.MoveDist / Duration;

    // 들어가는 순간 보고 있던 쪽으로 나간다. 도중에 방향이 바뀌어도 따라가지 않는다.
    // (공격은 CanTurn이 꺼져 있어서 어차피 방향이 안 바뀐다)
    mStepDirX   = mFacingRight ? 1.f : -1.f;

    mSavedSpeed = Movement->GetMoveSpeed();
    mStepping   = true;
}

void CActionStateComponent::UpdateStep(float DeltaTime)
{
    if (!mStepping)
    {
        return;
    }

    auto Movement = mMovement.lock();

    if (!Movement || mStepRemain <= 0.f || DeltaTime <= 0.f)
    {
        EndStep();
        return;
    }

    // 마지막 프레임에 넘치지 않도록 남은 거리에 맞춰 속도를 줄인다.
    float Speed = mStepSpeed;

    if (Speed * DeltaTime > mStepRemain)
    {
        Speed = mStepRemain / DeltaTime;
    }

    Movement->SetSpeed(Speed);
    Movement->SetMoveDir(FVector3(mStepDirX, 0.f, 0.f));

    mStepRemain -= Speed * DeltaTime;

    if (mStepRemain <= 0.f)
    {
        EndStep();
    }
}

void CActionStateComponent::EndStep()
{
    if (!mStepping)
    {
        return;
    }

    // 빌려 쓴 속도를 반드시 돌려준다. 안 그러면 공격이 끝난 뒤 걸음이 이상해진다.
    if (auto Movement = mMovement.lock())
    {
        Movement->SetSpeed(mSavedSpeed);
    }

    mStepping   = false;
    mStepRemain = 0.f;
    mStepSpeed  = 0.f;
}

// ── 점프 ─────────────────────────────────────────────────────────────────────
//
// 몸을 실제로 띄우는 계산은 CHeightComponent가 한다.
// 여기는 "언제 뛰는가"와 튜닝값만 들고 있다가 넘기고, 착지 통보를 받아
// 다음 상태를 고른다. 상태 머신이 물리를 직접 굴리면 나중에 붙일
// 에어본·다운·바운드까지 전부 이 클래스로 몰린다.

bool CActionStateComponent::BindHeightFromOwner()
{
    auto Owner = mOwner.lock();

    if (!Owner)
    {
        return false;
    }

    if (mHeight.expired())
    {
        for (const auto& Comp : Owner->GetActorCompList())
        {
            auto HeightComp = std::dynamic_pointer_cast<CHeightComponent>(Comp);

            if (HeightComp)
            {
                mHeight = HeightComp;
                break;
            }
        }
    }

    //없으면 만들어 붙인다. 점프가 있는 액터는 세로축이 반드시 필요한데
    //예전에 저장된 월드에는 이 컴포넌트가 없다. 손으로 붙이게 하면
    //불러올 때마다 점프가 조용히 죽는다. (한 번 저장하면 파일에도 남는다)
    if (mHeight.expired())
    {
        mHeight = Owner->CreateComponent<CHeightComponent>("Height");
    }

    auto HeightComp = mHeight.lock();

    if (!HeightComp)
    {
        return false;
    }

    //착지 통보는 한 번만 건다.
    if (!mLandHooked)
    {
        std::weak_ptr<CActionStateComponent> Self =
            std::dynamic_pointer_cast<CActionStateComponent>(GetThisPtr<CComponent>());

        HeightComp->SetOnLanded([Self](float ImpactVel)
        {
            if (auto This = Self.lock())
            {
                This->OnLanded(ImpactVel);
            }
        });

        mLandHooked = true;
    }

    return true;
}

float CActionStateComponent::GetAirTime() const
{
    // 체공 시간을 지정하지 않았으면 점프 애니메이션이 끝나는 시간에 맞춘다.
    // 그래야 착지 프레임이 나오는 순간과 실제로 땅에 닿는 순간이 맞는다.
    float AirTime = mJumpTime;

    if (AirTime <= 0.f)
    {
        AirTime = GetAnimDuration(mDef[EActionState::Jump].Anim);
    }

    // 애니메이션도 아직 안 붙었으면 눈에 보이는 값으로 잡아둔다.
    if (AirTime <= 0.f)
    {
        AirTime = 0.6f;
    }

    // 너무 짧으면 한 프레임 만에 뜨고 내려서 점프로 보이지 않는다.
    if (AirTime < 0.1f)
    {
        AirTime = 0.1f;
    }

    return AirTime;
}

void CActionStateComponent::BeginJump()
{
    if (!BindHeightFromOwner())
    {
        //높이 컴포넌트가 없으면 애니메이션만 나가고 만다.
        return;
    }

    mHeight.lock()->LaunchArc(mJumpHeight, GetAirTime());
}

void CActionStateComponent::OnLanded(float ImpactVel)
{
    //튕겨서 아직 다시 떠 있으면 그건 착지가 아니다. 완전히 멈출 때까지 기다린다.
    if (IsInAir())
    {
        return;
    }

    //맞고 떠 있던 몸이 땅에 닿았다. 여기서부터 다운이다.
    if (mState == EActionState::Airborne)
    {
        EnterState(EActionState::Down);
        return;
    }

    //점프는 애니메이션이 아직 남았으면 그대로 두고, 끝났으면 이동계로 돌아간다.
    if (mState == EActionState::Jump && IsActionFinished())
    {
        EnterState(DecideLocomotion());
    }
}

bool CActionStateComponent::IsInAir() const
{
    auto HeightComp = mHeight.lock();
    return HeightComp && HeightComp->IsInAir();
}

float CActionStateComponent::GetAirHeight() const
{
    auto HeightComp = mHeight.lock();
    return HeightComp ? HeightComp->GetHeight() : 0.f;
}

// ── 상태 전이 ────────────────────────────────────────────────────────────────

EActionState::Type CActionStateComponent::DecideLocomotion() const
{
    // 아주 작은 입력은 정지로 본다. (아날로그 입력이나 감속 잔여값 대비)
    float LenSq = mMoveDir.x * mMoveDir.x + mMoveDir.y * mMoveDir.y;

    if (LenSq < 0.0001f)
    {
        return EActionState::Idle;
    }

    return mWantRun ? EActionState::Run : EActionState::Walk;
}

bool CActionStateComponent::IsCancelOpen() const
{
    float Cancel = mDef[mState].CancelTime;

    return (Cancel >= 0.f) && (mStateTime >= Cancel);
}

bool CActionStateComponent::RequestState(EActionState::Type State)
{
    if (State < 0 || State >= EActionState::End)
    {
        return false;
    }

    // 애니메이션을 지정하지 않은 상태는 쓰지 않는 상태로 본다.
    if (mDef[State].Anim.empty())
    {
        return false;
    }

    // 죽은 뒤에는 아무 요청도 받지 않는다.
    if (mState == EActionState::Dead)
    {
        return false;
    }

    // 이동계는 언제든 액션에 자리를 내준다.
    if (IsLocomotion(mState))
    {
        EnterState(State);
        return true;
    }

    const int CurPriority = mDef[mState].Priority;
    const int NewPriority = mDef[State].Priority;

    // 더 센 동작은 진행 중인 액션을 끊는다. (피격이 공격을 끊는 경로)
    if (NewPriority > CurPriority)
    {
        EnterState(State);
        return true;
    }

    // 같은 등급이면 취소 창이 열렸을 때만 선입력으로 받아둔다.
    // 지금 동작이 끝나는 순간 이어서 나간다. (콤보)
    if (NewPriority == CurPriority && IsCancelOpen())
    {
        mQueued = State;
        return true;
    }

    return false;
}

bool CActionStateComponent::RequestHit(float LaunchPower, float KnockPower, float DirX)
{
    // 띄우는 힘이 있으면 경직 대신 뜬다.
    const EActionState::Type Want = (LaunchPower > 0.f) ? EActionState::Airborne
                                                        : EActionState::Hit;

    // 이미 떠 있는 상대를 또 때린 경우. (공중 콤보)
    // 상태를 다시 넣으면 애니메이션이 처음부터 나가서 뚝뚝 끊기므로,
    // 상태는 그대로 두고 힘만 다시 실어준다.
    if (mState == EActionState::Airborne)
    {
        if (DirX == 0.f)
        {
            DirX = mFacingRight ? -1.f : 1.f;
        }

        if (BindHeightFromOwner())
        {
            auto HeightComp = mHeight.lock();

            if (LaunchPower > 0.f) HeightComp->Launch(LaunchPower);
            if (KnockPower > 0.f)  HeightComp->AddKnockback(FVector2(DirX, 0.f), KnockPower);
        }

        return true;
    }

    if (!RequestState(Want))
    {
        // 다운 중이라 거절됐다. 밀리지도 뜨지도 않는다. (다운 무적)
        return false;
    }

    // 밀려나는 쪽. 안 주면 바라보는 반대쪽으로 밀린다.
    if (DirX == 0.f)
    {
        DirX = mFacingRight ? -1.f : 1.f;
    }

    if (BindHeightFromOwner())
    {
        auto HeightComp = mHeight.lock();

        if (LaunchPower > 0.f)
        {
            HeightComp->Launch(LaunchPower);
        }

        if (KnockPower > 0.f)
        {
            HeightComp->AddKnockback(FVector2(DirX, 0.f), KnockPower);
        }
    }

    return true;
}

bool CActionStateComponent::RequestAttack()
{
    // 선입력이 이미 차 있으면 무시한다. (연타로 큐가 밀리는 걸 막는다)
    if (mQueued != EActionState::End)
    {
        return false;
    }

    // 어디까지 이어져 있는지부터 본다.
    //   · 공격 중이면 그 타수 — 취소 창이 열려 있으면 다음 타가 선입력으로 예약된다
    //   · 공격이 끝난 직후면 접수 시간(ComboResetTime)이 남아 있는 동안만 이어진다
    //   · 그 외에는 0이라 1타부터 다시 시작한다
    int Stage = 0;

    if (IsAttack(mState))
    {
        Stage = GetAttackStage(mState);
    }
    else if (mComboWindow > 0.f)
    {
        Stage = mComboStage;
    }

    // 3타까지 갔으면 다시 1타로 돌아온다.
    EActionState::Type Next = (Stage >= 3) ? EActionState::Attack1
                                           : GetAttackState(Stage + 1);

    // 다음 타를 아직 안 만들었으면 1타로 되돌린다.
    if (mDef[Next].Anim.empty())
    {
        Next = EActionState::Attack1;
    }

    return RequestState(Next);
}

void CActionStateComponent::EnterState(EActionState::Type State)
{
    //상태가 언제 어떻게 바뀌는지 로그로 남긴다.
    //(로거는 서식 문자열을 처리하지 않고 인자를 뒤에 붙이므로 직접 문자열을 만든다)
    LOG_DEBUG(std::string("[ActionState] ") + GetStateName(mState) + " -> " + GetStateName(State)
              + " anim=" + (mDef[State].Anim.empty() ? "(none)" : mDef[State].Anim));

    // ── 콤보 단계 ──
    // 공격에 들어가면 그 타수를 기억해 두고, 공격이 끝나 제자리에 서는 순간부터
    // 다음 타 접수 시간이 흐르기 시작한다.
    //
    // 1타와 2타 사이에 다른 상태가 끼어들면 처음부터 다시 나간다.
    //   · 점프/대쉬/피격/다운 — 상태가 바뀌는 즉시 끊는다
    //   · 걷기/달리기          — 상태만으로는 안 끊고, 실제로 움직인 거리로 끊는다
    // 이동만 거리로 재는 이유는, 공격이 끝나면 반드시 이동계를 거치기 때문이다.
    // 방향키를 걸친 채로 쳤다고 바로 끊으면 콤보가 사실상 이어지지 않는다.
    {
        const bool bWasAttack = IsAttack(mState);

        if (IsAttack(State))
        {
            mComboStage  = GetAttackStage(State);
            mComboWindow = 0.f;     // 공격하는 동안에는 접수 시간이 흐르지 않는다
        }
        else if (IsLocomotion(State))
        {
            // 공격이 방금 끝났다. 여기서부터 다음 타를 기다려준다.
            //
            // 방향키를 누른 채로 쳤으면 이 순간 곧바로 Walk로 들어오는데,
            // 걷기 상태가 됐다는 것만으로는 끊지 않는다.
            // 실제로 몸이 얼마나 옮겨졌는지는 UpdateComboMove가 잰다.
            if (bWasAttack)
            {
                mComboWindow    = mComboResetTime;
                mComboMoveDist  = 0.f;
                mComboMoveValid = GetGroundPos(mComboLastPos);
            }
        }
        else
        {
            // 점프·대쉬·피격·다운처럼 이동계가 아닌 상태가 끼어들면 그 자리에서 버린다.
            ResetCombo();
        }
    }

    // 이전 동작이 모아치기로 멈춰 있었다면 반드시 풀고 나간다.
    // (피격에 끊겨서 나가는 경우 그대로 두면 그 시퀀스가 멈춘 채로 남는다)
    if (mChargeHolding)
    {
        if (auto Anim = mAnimComp.lock())
        {
            Anim->SetCurrentAnimationPause(false);
        }

        mChargeHolding = false;
    }

    // 점프에서 다른 상태로 빠져나가면 떠 있던 높이를 되돌린다.
    // (공중에서 맞고 피격으로 넘어가는 경로. 그대로 두면 뜬 채로 굳는다)
    // 에어본 상태를 붙이면 여기서 내리지 않고 그대로 날려보내게 된다.
    if (State != EActionState::Jump && !IsKnockdown(State))
    {
        if (auto HeightComp = mHeight.lock())
        {
            HeightComp->ForceLand();
        }
    }

    //눕는 순간부터 다운 시간을 잰다.
    if (State == EActionState::Down)
    {
        mDownTimer = 0.f;
    }

    mState     = State;
    mStateTime = 0.f;
    mQueued    = EActionState::End;

    // 전진 준비. 애니메이션을 못 걸고 빠져나가는 경로에서도
    // 이전 동작이 밀던 걸 반드시 정리해야 하므로 아래 return들보다 먼저 부른다.
    BeginStep(State);

    // 도약. 이쪽도 애니메이션이 없어서 빠져나가는 경로보다 먼저 걸어둔다.
    if (State == EActionState::Jump)
    {
        BeginJump();
    }

    const FActionStateDef& StateDef = mDef[State];

    if (StateDef.Anim.empty())
    {
        return;
    }

    auto AnimComp = mAnimComp.lock();

    if (!AnimComp)
    {
        return;
    }

    // 액션계는 반드시 끝나야 하므로 반복을 꺼둔다.
    // (에셋이 Loop=1로 저장돼 있어도 여기서 상태 정의가 이긴다)
    AnimComp->SetLoop(StateDef.Anim, StateDef.bLoop);
    AnimComp->SetSymmetry(StateDef.Anim, !mFacingRight);

    //역재생은 반드시 재생을 걸기 전에 정해야 한다.
    //되감기(Clear)가 역재생이면 마지막 프레임에서 시작하도록 되어 있어서,
    //순서가 바뀌면 첫 프레임에서 시작해 곧바로 끝나버린다.
    AnimComp->SetReverse(StateDef.Anim, StateDef.bReverse);

    if (IsLocomotion(State))
    {
        // 이동계는 같은 애니메이션이면 되감지 않는다. (걷다가 프레임이 튀는 걸 막는다)
        AnimComp->ChangeAnimation(StateDef.Anim);
    }
    else
    {
        // 액션계는 같은 공격을 다시 눌러도 처음부터 나가야 한다.
        AnimComp->PlayAnimation(StateDef.Anim);
    }

    // 모아치기 동작이면 공격키를 놓을 때까지 첫 프레임에서 붙잡아둔다.
    // 들어오는 순간 이미 손을 뗀 상태면 붙잡을 것도 없이 그대로 재생한다.
    if (StateDef.bChargeHold && mAttackHeld)
    {
        mChargeHolding = true;
        AnimComp->SetCurrentAnimationPause(true);
    }
}

bool CActionStateComponent::IsActionFinished() const
{
    // 점프는 애니메이션이 아니라 땅이 끝을 정한다.
    // 애니메이션이 먼저 끝나도 공중에 떠 있는 동안에는 상태를 유지해야 한다.
    // (여기서 풀리면 아직 떠 있는 채로 걷기 상태가 되어 그대로 굳는다)
    if (mState == EActionState::Jump && IsInAir())
    {
        return false;
    }

    //뜬 상태는 땅에 닿아야 끝난다. 착지는 OnLanded가 다운으로 넘겨준다.
    if (mState == EActionState::Airborne)
    {
        return false;
    }

    //누워 있는 시간은 애니메이션과 별개로 센다.
    //눕는 동작이 짧아도 다운 무적을 충분히 주기 위해서다.
    if (mState == EActionState::Down && mDownTimer < mDownTime)
    {
        return false;
    }

    auto AnimComp = mAnimComp.lock();

    // 애니메이션이 없으면 붙잡아 둘 이유가 없다.
    if (!AnimComp)
    {
        return true;
    }

    const std::string& Name = mDef[mState].Anim;

    if (Name.empty())
    {
        return true;
    }

    // 다른 곳에서 애니메이션을 바꿔치웠으면 이 상태를 유지할 근거가 없다.
    if (AnimComp->GetCurrentAnimationName() != Name)
    {
        return true;
    }

    return AnimComp->IsCurrentAnimationEnd();
}

bool CActionStateComponent::GetGroundPos(FVector3& Out) const
{
    auto Owner = mOwner.lock();

    if (!Owner)
    {
        return false;
    }

    //루트는 지면 위치다. 점프/띄우기로 뜬 높이는 자식 메시가 갖고 있으므로
    //여기에는 섞이지 않는다. 즉 공중에 떠도 이 값은 안 흔들린다.
    auto Root = Owner->GetRootComponent().lock();

    if (!Root)
    {
        return false;
    }

    Out = Root->GetWorldPos();

    return true;
}

void CActionStateComponent::UpdateComboMove(float DeltaTime)
{
    if (mComboWindow <= 0.f || !mComboMoveValid)
    {
        return;
    }

    FVector3 Now;

    if (!GetGroundPos(Now))
    {
        return;
    }

    const float dx = Now.x - mComboLastPos.x;
    const float dy = Now.y - mComboLastPos.y;

    mComboMoveDist += sqrtf(dx * dx + dy * dy);
    mComboLastPos   = Now;

    //키를 눌렀는지가 아니라 몸이 옮겨졌는지가 기준이다.
    if (mComboMoveDist >= mComboMoveLimit)
    {
        ResetCombo();
    }
}

void CActionStateComponent::ApplyFacing()
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

// ── 매 프레임 ────────────────────────────────────────────────────────────────

void CActionStateComponent::Update(float DeltaTime)
{
    CActorComponent::Update(DeltaTime);

    if (mAnimComp.expired())
    {
        BindAnimCompFromOwner();
    }

    //높이 컴포넌트는 나중에 붙을 수도 있으니 잡을 때까지 매 프레임 찾는다.
    if (mHeight.expired() || !mLandHooked)
    {
        BindHeightFromOwner();
    }

    // ── 모아치기 ──
    // 붙잡아 두는 동안에는 애니메이션도 시간도 멈춘다.
    // 시간까지 세워야 하는 이유는, 그냥 두면 ActionTimeout이 흘러서 몇 초 뒤에
    // 동작이 강제로 풀려버리기 때문이다. 취소 창(CancelTime)도 같이 닫아둔다.
    if (mChargeHolding)
    {
        if (mAttackHeld)
        {
            if (auto Anim = mAnimComp.lock())
            {
                Anim->SetCurrentAnimationPause(true);
            }

            ApplyFacing();

            // 입력은 매 프레임 새로 받는다.
            mMoveDir = FVector2(0.f, 0.f);
            mWantRun = false;
            return;
        }

        // 손을 뗐다. 멈춰 있던 프레임부터 나머지가 이어서 재생된다.
        if (auto Anim = mAnimComp.lock())
        {
            Anim->SetCurrentAnimationPause(false);
        }

        mChargeHolding = false;
    }

    mStateTime += DeltaTime;

    // ── 콤보 접수 시간 ──
    // 공격이 끝난 뒤부터 흐른다. 이 시간이 다 지나도록 공격키가 안 들어오면
    // 단계를 버리고 다음 공격은 다시 1타부터 나간다.
    if (mComboWindow > 0.f)
    {
        mComboWindow -= DeltaTime;

        if (mComboWindow <= 0.f)
        {
            ResetCombo();
        }
        else
        {
            //너무 많이 걸어갔으면 시간이 남았어도 콤보를 버린다.
            UpdateComboMove(DeltaTime);
        }
    }

    // ── 바라보는 방향 ──
    // 좌우 입력이 있을 때만 갱신한다.
    // 위/아래만 눌렀을 때는 바라보던 방향을 그대로 유지해야 하므로 건드리지 않는다.
    if (mDef[mState].bCanTurn)
    {
        if (mMoveDir.x > 0.0001f)
        {
            mFacingRight = true;
        }
        else if (mMoveDir.x < -0.0001f)
        {
            mFacingRight = false;
        }
    }

    //누워 있는 시간
    if (mState == EActionState::Down)
    {
        mDownTimer += DeltaTime;
    }

    // ── 상태 전진 ──
    // 이동이 잠긴 공격 중에도 여기서 직접 밀어준다. (기본 공격 1/2/3타)
    UpdateStep(DeltaTime);


    // ── 액션계: 끝났는지 본다 ──
    if (!IsLocomotion(mState) && mState != EActionState::Dead)
    {
        bool bTimeout  = (mStateTime >= mActionTimeout);
        bool bFinished = IsActionFinished();

        if (bFinished || bTimeout)
        {
            if (mQueued != EActionState::End)
            {
                EActionState::Type Next = mQueued;
                mQueued = EActionState::End;
                EnterState(Next);
            }
            else if (mState == EActionState::Down)
            {
                // 다 누웠으면 일어난다. 일어나는 동안도 아직 무적이다.
                EnterState(EActionState::GetUp);
            }
            else
            {
                // 이동 의도에 맞는 상태로 즉시 복귀한다.
                EnterState(DecideLocomotion());
            }
        }
    }
    // ── 이동계: 매 프레임 다시 판단한다 ──
    else if (IsLocomotion(mState))
    {
        EActionState::Type Next = DecideLocomotion();

        if (Next != mState && mStateTime >= mMinStateTime)
        {
            EnterState(Next);
        }
    }

    ApplyFacing();

    // 입력은 매 프레임 새로 받는다. 안 들어오면 정지로 간주해야 하기 때문이다.
    mMoveDir = FVector2(0.f, 0.f);
    mWantRun = false;
}

// ── 저장 / 복원 ──────────────────────────────────────────────────────────────

void CActionStateComponent::Save(std::ofstream& File) const
{
    CActorComponent::Save(File);

    File << "MinStateTime="   << mMinStateTime   << "\n";
    File << "ActionTimeout="  << mActionTimeout  << "\n";
    File << "ComboResetTime=" << mComboResetTime << "\n";
    File << "ComboMoveLimit=" << mComboMoveLimit << "\n";
    File << "JumpHeight="     << mJumpHeight     << "\n";
    File << "JumpTime="       << mJumpTime       << "\n";
    File << "JumpMoveScale=" << mJumpMoveScale  << "\n";
    File << "DownTime="      << mDownTime      << "\n";
    File << "StateCount="     << (int)EActionState::End << "\n";

    // 이름|애니메이션|우선순위|Loop|CanMove|CanTurn|CancelTime|ChargeHold|MoveDist|StepTime
    //     |Reverse|LaunchPower|KnockPower
    for (int i = 0; i < EActionState::End; ++i)
    {
        const FActionStateDef& StateDef = mDef[i];

        File << "State" << i << "="
             << sStateNames[i]        << "|"
             << StateDef.Anim              << "|"
             << StateDef.Priority          << "|"
             << (StateDef.bLoop    ? 1 : 0) << "|"
             << (StateDef.bCanMove ? 1 : 0) << "|"
             << (StateDef.bCanTurn ? 1 : 0) << "|"
             << StateDef.CancelTime        << "|"
             << (StateDef.bChargeHold ? 1 : 0) << "|"
             << StateDef.MoveDist          << "|"
             << StateDef.StepTime          << "|"
             << (StateDef.bReverse ? 1 : 0) << "|"
             << StateDef.LaunchPower       << "|"
             << StateDef.KnockPower        << "\n";
    }
}

void CActionStateComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
    CActorComponent::Load(Props);

    auto GetF = [&Props](const char* Key, float& Out)
    {
        auto Found = Props.find(Key);
        if (Found == Props.end()) return;
        try { Out = std::stof(Found->second); }
        catch (...) {}
    };

    GetF("MinStateTime",   mMinStateTime);
    GetF("ActionTimeout",  mActionTimeout);
    GetF("ComboResetTime", mComboResetTime);
    GetF("ComboMoveLimit", mComboMoveLimit);

    // 점프 설정은 나중에 생긴 항목이라 예전 월드에는 없다. 없으면 기본값을 쓴다.
    GetF("JumpHeight",    mJumpHeight);
    GetF("JumpTime",      mJumpTime);
    GetF("JumpMoveScale", mJumpMoveScale);
    GetF("DownTime",      mDownTime);

    int StateCount = 0;

    if (auto Found = Props.find("StateCount"); Found != Props.end())
    {
        try { StateCount = std::stoi(Found->second); }
        catch (...) { StateCount = 0; }
    }

    for (int i = 0; i < StateCount; ++i)
    {
        auto Found = Props.find("State" + std::to_string(i));

        if (Found == Props.end())
        {
            continue;
        }

        std::vector<std::string> Fields;
        {
            const std::string& Line = Found->second;
            size_t Start = 0;

            while (true)
            {
                size_t BarPos = Line.find('|', Start);

                if (BarPos == std::string::npos)
                {
                    Fields.push_back(Line.substr(Start));
                    break;
                }

                Fields.push_back(Line.substr(Start, BarPos - Start));
                Start = BarPos + 1;
            }
        }

        if (Fields.size() < 7)
        {
            continue;
        }

        // enum 순서가 아니라 이름으로 찾는다.
        EActionState::Type State = FindStateByName(Fields[0]);

        if (State == EActionState::End)
        {
            continue;
        }

        FActionStateDef StateDef;
        StateDef.Anim = Fields[1];

        // ChargeHold / MoveDist / StepTime은 나중에 생긴 항목이라 예전 파일에는 없다.
        // 없으면 생성자가 정해둔 값(3타만 모아치기, 공격에만 전진 거리)을 유지한다.
        StateDef.bChargeHold = mDef[State].bChargeHold;
        StateDef.MoveDist    = mDef[State].MoveDist;
        StateDef.StepTime    = mDef[State].StepTime;
        StateDef.bReverse    = mDef[State].bReverse;
        StateDef.LaunchPower = mDef[State].LaunchPower;
        StateDef.KnockPower  = mDef[State].KnockPower;

        if (Fields.size() >= 8)
        {
            StateDef.bChargeHold = (Fields[7] == "1");
        }

        if (Fields.size() >= 9)
        {
            try { StateDef.MoveDist = std::stof(Fields[8]); }
            catch (...) {}
        }

        if (Fields.size() >= 10)
        {
            try { StateDef.StepTime = std::stof(Fields[9]); }
            catch (...) {}
        }

        if (Fields.size() >= 11)
        {
            StateDef.bReverse = (Fields[10] == "1");
        }

        if (Fields.size() >= 13)
        {
            try
            {
                StateDef.LaunchPower = std::stof(Fields[11]);
                StateDef.KnockPower  = std::stof(Fields[12]);
            }
            catch (...) {}
        }

        try
        {
            StateDef.Priority   = std::stoi(Fields[2]);
            StateDef.CancelTime = std::stof(Fields[6]);
        }
        catch (...)
        {
            //숫자가 깨졌으면 기본값으로 둔다.
        }

        StateDef.bLoop    = (Fields[3] == "1");
        StateDef.bCanMove = (Fields[4] == "1");
        StateDef.bCanTurn = (Fields[5] == "1");

        mDef[State] = StateDef;
    }

    // 예전 AnimState 컴포넌트로 저장된 데이터도 그대로 읽어준다.
    auto GetOld = [&Props](const char* Key, std::string& Out)
    {
        auto Found = Props.find(Key);
        if (Found != Props.end() && !Found->second.empty()) Out = Found->second;
    };

    GetOld("IdleAnim", mDef[EActionState::Idle].Anim);
    GetOld("WalkAnim", mDef[EActionState::Walk].Anim);
    GetOld("RunAnim",  mDef[EActionState::Run].Anim);

    BindAnimCompFromOwner();
}
