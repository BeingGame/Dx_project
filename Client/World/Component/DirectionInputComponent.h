#pragma once

#include "World/ActorComponent.h"

// 플레이어 입력만 담당하는 컴포넌트.
//
// 하는 일은 세 가지뿐이다.
//   1. 방향키 4개를 읽어 이동 방향을 만든다
//   2. 같은 방향키를 짧은 간격으로 두 번 누르면 "달리기"로 판정한다
//   3. 공격키/점프키를 읽어 CActionStateComponent에 그 동작을 요청한다
//
// 여기서는 "무엇을 하고 싶은지"만 전달한다.
// 그 요청을 받아줄지, 지금 움직여도 되는지는 전부 CActionStateComponent가 정한다.
// (공격/피격 중이면 CanMove()가 false라서 이동 자체를 넘기지 않는다)
//
// 애니메이션은 절대 건드리지 않는다. 좌우 반전도 상태 컴포넌트의 몫이다.

class CDirectionInputComponent : public CActorComponent
{
public:
    CDirectionInputComponent();
    CDirectionInputComponent(const CDirectionInputComponent& src);
    virtual ~CDirectionInputComponent();

protected:
    std::weak_ptr<class CActionStateComponent> mActionState;
    std::weak_ptr<class CMovementComponent>    mMovement;

    bool mKeysRegistered = false;

    // 공격키. VK 코드다. (기본값 X)
    unsigned char mAttackKey = 'X';

    // 점프키. VK 코드다. (기본값 C)
    unsigned char mJumpKey = 'C';

    // 달리는 중 위/아래로만 갈 때 걷기 속도에 곱하는 배율.
    // 이 판은 위/아래가 화면 안쪽 깊이라, 달리기 속도를 그대로 쓰면
    // 좌우로 뛸 때와 같은 속도로 안쪽까지 파고들어 너무 빠르게 느껴진다.
    // 기준이 달리기가 아니라 걷기인 이유는 "걸을 때의 몇 배"로 잡는 값이라서다.
    float mRunVertScale = 2.f;

    // 좌우로 달리는 도중 위/아래를 누르면, 좌우 달리기를 유지한 채
    // 위/아래를 이 비율만큼만 섞어 얕은 대각선으로 흐르게 한다. (0이면 순수 좌우,
    // 1이면 45도) 방향 벡터의 Y 성분에 곱하는 값이라 속도(mRunVertScale)와는 별개다.
    float mRunDiagVertScale = 0.35f;

    // ── 더블탭(연속 입력) 판정 ──
    // 같은 키를 뗐다가 이 시간 안에 다시 누르면 달리기로 본다.
    float mDoubleTapTime = 0.28f;

    unsigned char mLastTapKey  = 0;
    float         mLastTapTime = -10.f;
    float         mTimeAccum   = 0.f;

    // 달리기는 그 방향키를 누르고 있는 동안만 유지된다.
    // 예외로 위/아래는 달리기를 끊지 않는다. 달리는 도중 위/아래를 누르면
    // 좌우를 무시하고 위/아래로만 달리고, 달리던 키를 놔도 그 키가 달리기를 이어받는다.
    bool          mRunning   = false;
    unsigned char mRunKey    = 0;

    // 걷기 속도와 달리기 속도 (초당 월드 유닛)
    // 150/280은 CPlayer가 쓰던 값인데, 그쪽은 카메라가 1유닛=1픽셀로 잡혀 있어서
    // 초당 150픽셀이라는 뜻이었다. 에디터에서 만드는 액터는 좌표 배율이 달라서
    // 그대로 쓰면 방향키 한 번에 화면 밖으로 나간다. 인스펙터에서 조정할 수 있다.
    float mWalkSpeed = 10.f;
    float mRunSpeed  = 20.f;

    FVector2 mMoveDir;

public:
    const FVector2& GetMoveDir() const { return mMoveDir; }
    bool  IsRunning() const            { return mRunning; }

    void SetDoubleTapTime(float Time) { mDoubleTapTime = Time; }
    float GetDoubleTapTime() const    { return mDoubleTapTime; }
    // 공격키를 바꾸면 그 키를 인풋에 새로 등록해야 한다.
    // (AddBindKey는 같은 이름이면 그냥 돌아가므로 이름에 키 코드를 붙여 구분한다)
    void SetAttackKey(unsigned char Key)
    {
        if (mAttackKey == Key) return;

        mAttackKey      = Key;
        mKeysRegistered = false;
    }
    unsigned char GetAttackKey() const   { return mAttackKey; }

    // 점프키도 같은 이유로 바꾸면 다시 등록해야 한다.
    void SetJumpKey(unsigned char Key)
    {
        if (mJumpKey == Key) return;

        mJumpKey        = Key;
        mKeysRegistered = false;
    }
    unsigned char GetJumpKey() const     { return mJumpKey; }
    // 달리는 중 위/아래 이동 속도 = WalkSpeed * 이 값.
    float GetRunVertScale() const       { return mRunVertScale; }
    void  SetRunVertScale(float Scale)  { mRunVertScale = (Scale < 0.f) ? 0.f : Scale; }

    // 좌우 달리기 중 위/아래를 섞는 대각선 비율 (0~1로 제한).
    float GetRunDiagVertScale() const      { return mRunDiagVertScale; }
    void  SetRunDiagVertScale(float Scale) { mRunDiagVertScale = (Scale < 0.f) ? 0.f : ((Scale > 1.f) ? 1.f : Scale); }

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

    // 소유 액터에서 액션 상태/이동 컴포넌트를 찾아 물린다.
    void BindFromOwner();

    // 같은 키를 짧은 간격으로 다시 눌렀는지 본다.
    void UpdateDoubleTap(unsigned char Key, bool bPressed, bool bHeld);
};
