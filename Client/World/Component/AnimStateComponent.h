#pragma once

#include "World/ActorComponent.h"

#include <string>
#include <unordered_map>

// 이동 상태에 맞는 애니메이션을 골라주는 컴포넌트.
//
// 입력을 직접 읽지 않는다. 누군가 매 프레임 SetMoveInput()으로
// "어느 방향으로, 달리는 중인지"만 알려주면 나머지를 처리한다.
//   · 플레이어  → CDirectionInputComponent 가 호출
//   · 몬스터    → AI/이동 로직이 호출
// 그래서 애니메이션 동작은 플레이어와 몬스터가 똑같이 쓴다.
//
// 좌우 반전:
//   스프라이트가 오른쪽을 보고 그려져 있으므로, 왼쪽으로 갈 때만 Symmetry를 켠다.
//   위/아래만 눌렀을 때는 바라보던 방향을 그대로 유지한다.

namespace EAnimMoveState
{
    enum Type
    {
        Idle,
        Walk,
        Run,
        End
    };
}

class CAnimStateComponent : public CActorComponent
{
public:
    CAnimStateComponent();
    CAnimStateComponent(const CAnimStateComponent& src);
    virtual ~CAnimStateComponent();

protected:
    // 조종할 애니메이션 컴포넌트. 비워두면 소유 액터에서 알아서 찾는다.
    std::weak_ptr<class CAnimation2DComponent> mAnimComp;

    // 상태별 애니메이션 이름 (비어 있으면 그 상태는 건너뛴다)
    std::string mStateAnim[EAnimMoveState::End];

    EAnimMoveState::Type mState        = EAnimMoveState::Idle;
    EAnimMoveState::Type mPendingState = EAnimMoveState::Idle;

    // 스프라이트 원본이 오른쪽을 보고 있다는 전제
    bool mFacingRight = true;

    // 이번 프레임에 들어온 이동 의도
    FVector2 mMoveDir;
    bool     mWantRun = false;

    // 상태가 잠깐 튀는 걸 막는 최소 유지 시간
    float mStateHold     = 0.f;
    float mMinStateTime  = 0.05f;

public:
    void SetAnimComp(const std::weak_ptr<class CAnimation2DComponent>& Comp) { mAnimComp = Comp; }

    // 상태별로 재생할 애니메이션 이름을 지정한다.
    void SetStateAnim(EAnimMoveState::Type State, const std::string& AnimName)
    {
        if (State >= 0 && State < EAnimMoveState::End)
            mStateAnim[State] = AnimName;
    }

    const std::string& GetStateAnim(EAnimMoveState::Type State) const
    {
        static const std::string Empty;
        return (State >= 0 && State < EAnimMoveState::End) ? mStateAnim[State] : Empty;
    }

    // 매 프레임 이동 의도를 넣어준다.
    // Dir은 정규화하지 않아도 된다. 길이가 0에 가까우면 Idle로 본다.
    // bRun이 true면 Run, 아니면 Walk.
    void SetMoveInput(const FVector2& Dir, bool bRun)
    {
        mMoveDir  = Dir;
        mWantRun  = bRun;
    }

    EAnimMoveState::Type GetState() const { return mState; }
    bool  IsFacingRight() const           { return mFacingRight; }

    // 피격/공격처럼 이동과 무관한 애니메이션을 잠시 재생시킬 때 쓴다.
    // 다음 SetMoveInput 결과가 달라지면 자동으로 이동 상태로 돌아온다.
    void PlayOnce(const std::string& AnimName);

public:
    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;

    virtual void Save(std::ofstream& File) const override;
    virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;

    virtual CAnimStateComponent* Clone() const override { return new CAnimStateComponent(*this); }
    virtual std::string GetTypeName() const override    { return "CAnimStateComponent"; }

private:
    // 소유 액터에서 애니메이션 컴포넌트를 찾아 물린다.
    bool BindAnimCompFromOwner();

    // 지금 이동 의도로부터 상태를 결정한다.
    EAnimMoveState::Type DecideState() const;

    // 상태에 맞는 애니메이션으로 갈아탄다. 같은 애니메이션이면 아무것도 안 한다.
    void ApplyState(EAnimMoveState::Type State);

    // 현재 바라보는 방향을 애니메이션의 Symmetry로 반영한다.
    void ApplyFacing();
};
