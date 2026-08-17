#pragma once

#include "World/ActorComponent.h"

// 방향키 입력만 담당하는 컴포넌트. (플레이어 전용)
//
// 하는 일은 두 가지뿐이다.
//   1. 방향키 4개를 읽어 이동 방향을 만든다
//   2. 같은 방향키를 짧은 간격으로 두 번 누르면 "달리기"로 판정한다
//
// 결과는 매 프레임 CAnimStateComponent(애니메이션)와
// CMovementComponent(실제 이동)에 넘겨준다.
// 애니메이션 쪽은 입력을 모르기 때문에 몬스터도 같은 컴포넌트를 그대로 쓴다.
//
// 좌우 반전 판단은 여기서 하지 않는다. 애니메이션 컴포넌트의 몫이다.

class CDirectionInputComponent : public CActorComponent
{
public:
    CDirectionInputComponent();
    CDirectionInputComponent(const CDirectionInputComponent& src);
    virtual ~CDirectionInputComponent();

protected:
    std::weak_ptr<class CAnimStateComponent>  mAnimState;
    std::weak_ptr<class CMovementComponent>   mMovement;

    bool mKeysRegistered = false;

    // ── 더블탭(연속 입력) 판정 ──
    // 같은 키를 뗐다가 이 시간 안에 다시 누르면 달리기로 본다.
    float mDoubleTapTime = 0.28f;

    unsigned char mLastTapKey  = 0;
    float         mLastTapTime = -10.f;
    float         mTimeAccum   = 0.f;

    // 달리기는 그 방향키를 누르고 있는 동안만 유지된다.
    bool          mRunning   = false;
    unsigned char mRunKey    = 0;

    // 걷기 속도와 달리기 속도
    float mWalkSpeed = 150.f;
    float mRunSpeed  = 280.f;

    FVector2 mMoveDir;

public:
    const FVector2& GetMoveDir() const { return mMoveDir; }
    bool  IsRunning() const            { return mRunning; }

    void SetDoubleTapTime(float Time) { mDoubleTapTime = Time; }
    void SetWalkSpeed(float Speed)    { mWalkSpeed = Speed; }
    void SetRunSpeed(float Speed)     { mRunSpeed = Speed; }

    float GetWalkSpeed() const { return mWalkSpeed; }
    float GetRunSpeed() const  { return mRunSpeed; }

public:
    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;

    virtual void Save(std::ofstream& File) const override;
    virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;

    virtual CDirectionInputComponent* Clone() const override { return new CDirectionInputComponent(*this); }
    virtual std::string GetTypeName() const override         { return "CDirectionInputComponent"; }

private:
    // 인풋은 등록된 키만 GetKey로 조회할 수 있다. VK 코드로 넣어야 한다.
    void RegisterKeys();

    // 소유 액터에서 애니메이션 상태/이동 컴포넌트를 찾아 물린다.
    void BindFromOwner();

    // 같은 키를 짧은 간격으로 다시 눌렀는지 본다.
    void UpdateDoubleTap(unsigned char Key, bool bPressed, bool bHeld);
};
