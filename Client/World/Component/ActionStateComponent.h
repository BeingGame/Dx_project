#pragma once

#include "World/ActorComponent.h"

#include <string>
#include <unordered_map>

// 액터가 "지금 무엇을 하는 중인지"를 혼자서 들고 있는 컴포넌트.
//
// 구조는 세 층으로 나눠져 있다.
//
//   [의도] DirectionInput(플레이어) / AIComponent(몬스터)
//            "이동하고 싶다 / 공격하겠다"만 알려준다. 애니메이션은 건드리지 않는다.
//              │  SetMoveInput() / RequestState()
//   [상태] CActionStateComponent   ← 여기. 상태의 유일한 주인이다.
//            우선순위와 잠금 규칙으로 요청을 받을지 말지 정한다.
//              │
//   [출력] Animation2DComponent(재생) / MovementComponent(이동 허용 여부)
//
// ChangeAnimation을 부르는 곳이 두 군데가 되면 서로 덮어쓰기 때문에,
// 애니메이션 교체는 반드시 이 컴포넌트만 한다.
//
// 상태는 두 부류다.
//   · 이동계(Idle/Walk/Run) — 입력에 따라 매 프레임 자유롭게 바뀐다.
//   · 액션계(Attack/Jump/Hit/Dash/Dead) — 한 번 들어가면 애니메이션이 끝날 때까지 유지된다.
//     끝을 애니메이션에게 물어보므로(IsCurrentAnimationEnd) 그 시퀀스는 Loop가 꺼져 있어야 한다.
//     ApplyState에서 강제로 꺼주기 때문에 에셋이 Loop=1이어도 동작은 한다.
//
// 공격 콤보(1타 → 2타 → 3타)는 "콤보 단계"를 따로 들고 이어간다.
//   · 공격에 들어가면 그 타수가 단계로 기록된다.
//   · 공격 애니메이션이 끝나고 제자리에 서면 ComboResetTime 만큼 접수 시간이 열린다.
//     그 안에 공격키를 누르면 다음 타로, 시간이 지나면 단계가 0으로 풀려 다시 1타부터 나간다.
//   · 타수 사이에 Idle 아닌 상태가 하나라도 끼어들면 그 자리에서 단계를 버린다.
//     걷기·달리기도 포함이다. 한 발짝 움직였으면 다음 공격은 1타다.
//   · 3타처럼 bChargeHold가 켜진 동작은 공격키를 누르고 있는 동안 첫 프레임에서
//     멈춰 있다가, 손을 떼면 나머지가 이어서 재생된다. (모아치기)

namespace EActionState
{
    enum Type
    {
        Idle,
        Walk,
        Run,

        Attack1,
        Attack2,
        Attack3,
        Jump,
        Hit,
        Dash,
        Dead,

        // ── 띄우기 계열 ──
        // 맞고 뜬다(Airborne) → 떨어져 눕는다(Down) → 일어난다(GetUp).
        // Down/GetUp은 우선순위가 피격보다 높아서 그 동안은 다시 맞지 않는다.
        // (던파의 다운 무적이 이 우선순위 하나로 나온다)
        Airborne,
        Down,
        GetUp,

        End
    };
}

// 상태 하나의 정의. 전부 데이터라서 월드/프리팹에 저장되고 에디터에서 바꿀 수 있다.
struct FActionStateDef
{
    // 재생할 애니메이션 이름. 비어 있으면 그 상태는 아예 사용하지 않는다.
    std::string Anim;

    // 클수록 세다. 센 요청만 약한 상태를 끊을 수 있다. (피격이 공격을 끊는 근거)
    int Priority = 0;

    bool bLoop    = true;   // 이동계만 true
    bool bCanMove = true;   // 이 상태에서 이동 입력을 받아줄지
    bool bCanTurn = true;   // 이 상태에서 좌우 반전을 갱신할지

    // 진입 후 이 시간이 지나면 같은 등급의 다음 동작을 선입력으로 받아준다. (콤보 창)
    // 음수면 애니메이션이 끝날 때까지 아무것도 못 받는다.
    float CancelTime = -1.f;

    // 공격키를 누르고 있는 동안 첫 프레임에서 멈춰 세울지. (모아치기)
    // 손을 떼는 순간 나머지 프레임이 이어서 재생된다. 기본 공격 3타에만 켜져 있다.
    bool bChargeHold = false;

    // 이 상태에 들어가면 바라보는 쪽으로 이만큼(월드 유닛) 밀고 나간다.
    // 이동 축은 X 하나뿐이다. (위/아래를 보고 있어도 앞으로만 나간다)
    float MoveDist = 0.f;

    // 그 거리를 밀어내는 데 쓰는 시간. 상태에 들어간 직후부터 이만큼만 움직인다.
    // 짧게 잡아야 "치고 나가는" 느낌이 나고, 길게 잡으면 동작 내내 미끄러진다.
    // 0 이하면 애니메이션이 재생되는 시간 전체에 걸쳐 나눠 민다.
    float StepTime = 0.12f;

    // 애니메이션을 거꾸로 돌린다. 기상(GetUp)이 쓰러지는 동작을 뒤집어 쓴다.
    bool bReverse = false;

    // ── 이 동작이 맞은 쪽에게 주는 힘 ──
    // 여기 적힌 값은 "때리는 쪽"의 데이터다. 판정이 붙으면 맞은 액터의
    // RequestHit에 그대로 넘겨주면 된다.

    // 0보다 크면 상대를 띄운다. 그 순간 상대는 Airborne으로 넘어간다.
    float LaunchPower = 0.f;

    // 뒤로 밀어내는 속도. 띄우지 않아도 밀 수는 있다.
    float KnockPower = 0.f;
};

class CActionStateComponent : public CActorComponent
{
public:
    CActionStateComponent();
    CActionStateComponent(const CActionStateComponent& src);
    virtual ~CActionStateComponent();

protected:
    // 조종할 애니메이션 컴포넌트. 비워두면 소유 액터에서 알아서 찾는다.
    std::weak_ptr<class CAnimation2DComponent> mAnimComp;

    FActionStateDef mDef[EActionState::End];

    EActionState::Type mState  = EActionState::Idle;

    // 지금 몇 타까지 이어져 있는지. 0이면 콤보가 끊긴 상태라 다음 공격은 1타다.
    int mComboStage = 0;

    // 공격이 끝난 뒤 다음 타를 받아주는 남은 시간. 0이 되면 mComboStage가 풀린다.
    float mComboWindow = 0.f;

    // 공격이 끝난 뒤 다음 타를 기다려주는 시간. (문제의 "특정 시간")
    float mComboResetTime = 0.5f;

    // 접수 시간 동안 이만큼 움직이면 콤보를 버린다. (월드 유닛)
    //
    // "걷기 상태에 들어갔는가"가 아니라 "실제로 몸이 옮겨졌는가"로 재는 이유:
    // 방향키를 누른 채로 치면 공격이 끝나는 순간 곧바로 Walk로 들어오는데,
    // 그것만으로 끊으면 키를 살짝 걸친 것만으로 콤보가 죽는다.
    // 걷기 속도가 0.3이니 0.06이면 0.2초쯤 걸어야 끊긴다.
    float mComboMoveLimit = 0.06f;

    // 접수 시간이 열린 뒤 움직인 거리와, 그걸 재기 위한 직전 지면 위치.
    float    mComboMoveDist = 0.f;
    FVector3 mComboLastPos;
    bool     mComboMoveValid = false;

    // 액션이 끝나면 이어서 들어갈 상태. (콤보 선입력)
    EActionState::Type mQueued = EActionState::End;

    // 현재 상태에 머문 시간
    float mStateTime = 0.f;

    // 공격키를 지금 누르고 있는지. 입력 컴포넌트가 매 프레임 알려준다.
    bool mAttackHeld = false;

    // 모아치기로 첫 프레임에 붙잡아 두고 있는 중인지.
    // 한 번 손을 떼면 그 동작이 끝날 때까지 다시 붙잡지 않는다.
    // (재생 도중 공격키를 또 눌렀다고 중간 프레임에서 서버리면 곤란하다)
    bool mChargeHolding = false;

    // 이동계가 한두 프레임 만에 걷기↔달리기를 오가는 걸 막는 최소 유지 시간
    float mMinStateTime = 0.05f;

    // 애니메이션이 끝나지 않는 사고에 대비한 안전장치. 이 시간이 지나면 강제로 푼다.
    float mActionTimeout = 5.f;

    // 스프라이트 원본이 오른쪽을 보고 있다는 전제
    bool mFacingRight = true;

    // 이번 프레임에 들어온 이동 의도
    FVector2 mMoveDir;
    bool     mWantRun = false;

    // ── 상태 전진(MoveDist) ──
    // 공격처럼 이동 입력이 막힌 상태에서도 이 컴포넌트가 직접 밀어준다.
    // 그래서 이동 컴포넌트를 여기서도 물고 있어야 한다.
    std::weak_ptr<class CMovementComponent> mMovement;

    float mStepRemain = 0.f;   // 남은 거리
    float mStepSpeed  = 0.f;   // 초당 이동 거리 (거리 / 애니메이션 길이)
    float mStepDirX   = 0.f;   // 밀고 나가는 방향. +1 오른쪽 / -1 왼쪽
    float mSavedSpeed = 0.f;   // 밀기 전에 이동 컴포넌트가 쓰던 속도
    bool  mStepping   = false;

    // ── 점프 ──
    // 실제로 몸을 띄우는 계산은 CHeightComponent가 한다. 여기는 "언제 뛰는가"와
    // 튜닝값만 들고 있다가 넘긴다. 상태 머신이 물리를 직접 적분하면
    // 피격으로 뜨는 것·다운·바운드까지 전부 이 클래스로 몰리기 때문이다.
    std::weak_ptr<class CHeightComponent> mHeight;

    // 뛰어오르는 높이(월드 유닛). 이 월드는 캐릭터 한 칸이 0.8이라 0.35면 반쯤 뛴다.
    float mJumpHeight = 0.35f;

    // 공중에 머무는 시간. 0 이하면 점프 애니메이션 길이를 그대로 쓴다.
    // (그래야 착지 프레임이 나오는 순간과 실제로 땅에 닿는 순간이 맞는다)
    float mJumpTime = 0.f;

    // 공중에 떠 있는 동안 방향키 이동에 곱하는 배율.
    // 1이면 땅에서와 같은 속도고, 0으로 두면 뛴 자리에 그대로 떨어진다.
    // 실제로 속도를 곱하는 건 입력 컴포넌트다. (속도를 넘기는 쪽이 거기라서)
    float mJumpMoveScale = 1.f;

    // 착지 통보를 받을 준비가 됐는지. 높이 컴포넌트를 찾은 뒤 한 번만 건다.
    bool mLandHooked = false;

    // 쓰러진 뒤 그대로 누워 있는 시간. 이 시간이 지나야 일어나기 시작한다.
    // 애니메이션이 끝나는 것과 별개로 재는 이유는, 눕는 동작이 짧아도
    // 다운 무적을 충분히 주고 싶기 때문이다.
    float mDownTime = 0.7f;

    // 지금 눕고 나서 흐른 시간
    float mDownTimer = 0.f;

public:
    void SetAnimComp(const std::weak_ptr<class CAnimation2DComponent>& Comp) { mAnimComp = Comp; }

    // ── 의도 넣기 ────────────────────────────────────────────────────────────

    // 매 프레임 이동 의도를 넣어준다. Dir은 정규화하지 않아도 된다.
    // 길이가 0에 가까우면 Idle, bRun이면 Run, 아니면 Walk로 본다.
    void SetMoveInput(const FVector2& Dir, bool bRun)
    {
        mMoveDir = Dir;
        mWantRun = bRun;
    }

    // 공격키를 누르고 있는지 매 프레임 알려준다. (모아치기 판정에 쓴다)
    void SetAttackHeld(bool bHeld) { mAttackHeld = bHeld; }

    // 지금 모아치기로 멈춰 서 있는지.
    bool IsChargeHolding() const { return mChargeHolding && mAttackHeld; }

    // 액션을 요청한다. 우선순위에 밀리면 false를 돌려주고 아무 일도 일어나지 않는다.
    bool RequestState(EActionState::Type State);

    // 공격 버튼 하나로 1타 → 2타 → 3타가 이어지도록 해준다.
    bool RequestAttack();

    // 피격 / 사망. (사망은 무엇으로도 덮이지 않는다)
    //
    // LaunchPower가 0보다 크면 경직(Hit) 대신 띄우기(Airborne)로 간다.
    // KnockPower는 뒤로 밀어내는 속도다. DirX는 밀려나는 쪽(+1 오른쪽 / -1 왼쪽)이고
    // 0이면 지금 바라보는 반대쪽, 즉 맞아서 뒤로 밀리는 방향으로 잡는다.
    bool RequestHit(float LaunchPower = 0.f, float KnockPower = 0.f, float DirX = 0.f);
    bool RequestDead() { return RequestState(EActionState::Dead); }

    // 이 상태가 상대에게 주는 힘. 판정이 붙으면 맞은 액터의 RequestHit에 넘긴다.
    float GetLaunchPower(EActionState::Type State) const { return GetStateDef(State).LaunchPower; }
    float GetKnockPower(EActionState::Type State) const  { return GetStateDef(State).KnockPower; }

    // 지금 하고 있는 동작이 주는 힘. (공격 판정에서 그대로 꺼내 쓰라고 둔다)
    float GetCurrentLaunchPower() const { return mDef[mState].LaunchPower; }
    float GetCurrentKnockPower() const  { return mDef[mState].KnockPower; }

    // 띄우기 계열인지. 이 동안에는 피격을 다시 받지 않는다.
    static bool IsKnockdown(EActionState::Type State)
    {
        return State == EActionState::Airborne
            || State == EActionState::Down
            || State == EActionState::GetUp;
    }

    // ── 상태 물어보기 ────────────────────────────────────────────────────────

    EActionState::Type GetState() const { return mState; }
    bool IsFacingRight() const          { return mFacingRight; }

    // 이동/조작 쪽이 매 프레임 물어본다. 공격·피격 중이면 false다.
    bool CanMove() const { return mDef[mState].bCanMove; }
    bool CanTurn() const { return mDef[mState].bCanTurn; }

    bool IsDead() const { return mState == EActionState::Dead; }

    // 이동계(Idle/Walk/Run)인지. 나머지는 전부 잠금이 걸리는 액션계다.
    static bool IsLocomotion(EActionState::Type State)
    {
        return State == EActionState::Idle
            || State == EActionState::Walk
            || State == EActionState::Run;
    }

    // 기본 공격 3종인지.
    static bool IsAttack(EActionState::Type State)
    {
        return State == EActionState::Attack1
            || State == EActionState::Attack2
            || State == EActionState::Attack3;
    }

    // 공격 상태를 타수(1~3)로. 공격이 아니면 0이다.
    static int GetAttackStage(EActionState::Type State)
    {
        switch (State)
        {
        case EActionState::Attack1: return 1;
        case EActionState::Attack2: return 2;
        case EActionState::Attack3: return 3;
        default:                    return 0;
        }
    }

    // 타수(1~3)를 상태로. 범위를 벗어나면 1타로 본다.
    static EActionState::Type GetAttackState(int Stage)
    {
        switch (Stage)
        {
        case 2:  return EActionState::Attack2;
        case 3:  return EActionState::Attack3;
        default: return EActionState::Attack1;
        }
    }

    // 점프인지. (이동계도 공격도 아닌 별개의 액션이다)
    static bool IsJump(EActionState::Type State) { return State == EActionState::Jump; }

    // 지금 몇 타까지 이어져 있는지. (0이면 끊긴 상태)
    int GetComboStage() const { return mComboStage; }

    // 다음 타 접수 시간이 아직 남아 있는지.
    bool IsComboWindowOpen() const { return mComboWindow > 0.f; }
    float GetComboWindow() const   { return mComboWindow; }

    // 콤보를 그 자리에서 끊는다.
    void ResetCombo()
    {
        mComboStage      = 0;
        mComboWindow     = 0.f;
        mComboMoveDist   = 0.f;
        mComboMoveValid  = false;
    }

    // 접수 시간 동안 허용되는 이동 거리
    float GetComboMoveLimit() const      { return mComboMoveLimit; }
    void  SetComboMoveLimit(float Value) { mComboMoveLimit = (Value < 0.f) ? 0.f : Value; }

    static const char* GetStateName(EActionState::Type State);
    static EActionState::Type FindStateByName(const std::string& Name);

    // ── 상태 정의 ────────────────────────────────────────────────────────────

    const FActionStateDef& GetStateDef(EActionState::Type State) const
    {
        static const FActionStateDef Empty;
        return (State >= 0 && State < EActionState::End) ? mDef[State] : Empty;
    }

    void SetStateDef(EActionState::Type State, const FActionStateDef& Def)
    {
        if (State >= 0 && State < EActionState::End) mDef[State] = Def;
    }

    void SetStateAnim(EActionState::Type State, const std::string& AnimName)
    {
        if (State >= 0 && State < EActionState::End) mDef[State].Anim = AnimName;
    }

    const std::string& GetStateAnim(EActionState::Type State) const
    {
        static const std::string Empty;
        return (State >= 0 && State < EActionState::End) ? mDef[State].Anim : Empty;
    }

    // 이동계가 한두 프레임 만에 걷기↔달리기를 오가는 걸 막는 최소 유지 시간
    float GetMinStateTime() const     { return mMinStateTime; }
    void  SetMinStateTime(float Time) { mMinStateTime = Time; }

    // 애니메이션이 끝나지 않는 사고에 대비한 강제 해제 시간
    float GetActionTimeout() const     { return mActionTimeout; }
    void  SetActionTimeout(float Time) { mActionTimeout = Time; }

    // 공격이 끝난 뒤 다음 타를 기다려주는 시간
    float GetComboResetTime() const     { return mComboResetTime; }
    void  SetComboResetTime(float Time) { mComboResetTime = (Time < 0.f) ? 0.f : Time; }

    // 쓰러진 뒤 누워 있는 시간
    float GetDownTime() const     { return mDownTime; }
    void  SetDownTime(float Time) { mDownTime = (Time < 0.f) ? 0.f : Time; }

    // ── 점프 ──
    float GetJumpHeight() const        { return mJumpHeight; }
    void  SetJumpHeight(float Height)  { mJumpHeight = (Height < 0.f) ? 0.f : Height; }

    // 0이면 점프 애니메이션 길이에 맞춘다.
    float GetJumpTime() const     { return mJumpTime; }
    void  SetJumpTime(float Time) { mJumpTime = (Time < 0.f) ? 0.f : Time; }

    // 공중에서 방향키 이동에 곱하는 배율. 입력 컴포넌트가 매 프레임 물어본다.
    float GetJumpMoveScale() const       { return mJumpMoveScale; }
    void  SetJumpMoveScale(float Scale)  { mJumpMoveScale = (Scale < 0.f) ? 0.f : Scale; }

    // 지금 공중에 떠 있는지. 떠 있는 동안에는 점프 상태가 풀리지 않는다.
    // 판단은 높이 컴포넌트가 하고 여기서는 물어보기만 한다.
    bool  IsInAir() const;
    float GetAirHeight() const;

    // 실제로 적용되는 체공 시간. JumpTime이 0이면 애니메이션 길이가 나온다.
    // (인스펙터가 "0"이 무슨 뜻인지 보여주는 데 쓴다)
    float GetAirTime() const;

    // 지금 예약된 선입력. (없으면 End)
    EActionState::Type GetQueuedState() const { return mQueued; }

public:
    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;

    virtual void Save(std::ofstream& File) const override;
    virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;

    virtual CActionStateComponent* Clone() const override { return new CActionStateComponent(*this); }
    virtual std::string GetTypeName() const override      { return "CActionStateComponent"; }

private:
    // 소유 액터에서 애니메이션 컴포넌트를 찾아 물린다.
    bool BindAnimCompFromOwner();

    // 소유 액터에서 이동 컴포넌트를 찾아 물린다. (상태 전진에 쓴다)
    bool BindMovementFromOwner();

    // 상태에 들어갈 때 전진 거리를 준비하고, 매 프레임 조금씩 밀어준다.
    void BeginStep(EActionState::Type State);
    void UpdateStep(float DeltaTime);
    void EndStep();

    // 높이 컴포넌트에 도약을 요청한다. 실제 포물선은 저쪽이 굴린다.
    void BeginJump();

    // 소유 액터의 높이 컴포넌트를 찾아 물리고 착지 통보를 건다.
    bool BindHeightFromOwner();

    // 착지 통보. 지금 상태에 따라 다음 상태를 정한다.
    void OnLanded(float ImpactVel);

    // 시퀀스가 실제로 재생되는 시간. 못 찾으면 0이다.
    float GetAnimDuration(const std::string& Anim) const;

    // 지금 이동 의도로부터 이동계 상태를 정한다.
    EActionState::Type DecideLocomotion() const;

    // 상태에 진입한다. 애니메이션 교체는 여기서만 일어난다.
    void EnterState(EActionState::Type State);

    // 액션이 끝났는지 애니메이션에게 물어본다.
    bool IsActionFinished() const;

    // 콤보 선입력을 받아줄 시간인지.
    bool IsCancelOpen() const;

    // 현재 바라보는 방향을 애니메이션의 Symmetry로 반영한다.
    void ApplyFacing();

    // 소유 액터의 지면 위치(루트). 높이는 자식 메시가 갖고 있으므로 여기 안 섞인다.
    bool GetGroundPos(FVector3& Out) const;

    // 접수 시간 동안 움직인 거리를 재고, 한계를 넘으면 콤보를 버린다.
    void UpdateComboMove(float DeltaTime);
};
