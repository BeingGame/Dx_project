#pragma once

#include "World/ActorComponent.h"

#include <functional>

// 세로축(높이)만 전담하는 컴포넌트.
//
// 이 판은 축이 세 개다.
//   X = 좌우,  Y = 깊이(앞뒤),  Z = 높이
// 그런데 화면은 2D라 Z를 따로 그릴 데가 없다. 그래서 높이는
// "메시를 화면 위쪽으로 밀어 올리는 양"으로만 표현한다.
//
// 액터 자체의 Y는 절대 건드리지 않는다. 렌더러가 Y로 정렬하기 때문에
// 높이를 Y에 섞으면 점프한 캐릭터가 뒤로 물러난 것으로 취급되고,
// 나중에 붙일 깊이 판정(같은 라인에 있는지)도 같이 어긋난다.
// 그래서 지면 위치는 루트가, 보이는 높이는 자식 메시가 나눠 갖는다.
//
// 점프도 피격으로 뜨는 것도 전부 여기로 들어온다. 상태 머신이 없는 액터
// (튕기는 오브젝트 등)도 이 컴포넌트만 붙이면 똑같이 뜬다.
class CHeightComponent : public CActorComponent
{
public:
    CHeightComponent();
    CHeightComponent(const CHeightComponent& src);
    virtual ~CHeightComponent();

protected:
    // 화면에서 밀어 올릴 대상. 액터의 루트가 아니라 그 자식 메시다.
    std::weak_ptr<class CSceneComponent> mVisual;

    // 지면에서 뜬 높이와 그 속도
    float mHeight = 0.f;
    float mVelZ   = 0.f;
    bool  mInAir  = false;

    // 끌어내리는 가속도 (유닛/초^2)
    float mGravity = 5.7f;

    // 착지 반발. 0이면 안 튀고 그 자리에 선다.
    // 다운될 때 한 번 통 튕기는 연출이 이 값으로 나온다.
    float mBounce = 0.f;

    // 이보다 느리게 부딪히면 그만 튕기고 눕는다. (무한히 잘게 튀는 걸 막는다)
    float mMinBounceVel = 0.35f;

    // 맞고 밀려나는 수평 속도. 지면 위치(루트)에 직접 더한다.
    FVector2 mKnockVel;

    // 넉백이 잦아드는 정도. 클수록 빨리 멈춘다.
    float mKnockDamp = 8.f;

    // 지금까지 메시에 얹어둔 높이. 착지할 때 정확히 이만큼만 되돌린다.
    float mAppliedOffset = 0.f;

    // 착지한 순간 알려준다. 인자는 부딪힌 속도(양수).
    // 다운으로 보낼지 그냥 서게 할지는 받는 쪽이 정한다.
    std::function<void(float)> mOnLanded;

public:
    // ── 띄우기 ──────────────────────────────────────────────────────────────

    // 이 속도로 위로 던진다. 이미 떠 있으면 속도를 갈아끼운다. (공중 콤보)
    void Launch(float VelZ);

    // 도달 높이와 체공 시간으로 던진다. 점프가 이걸 쓴다.
    // 꼭대기가 절반 시점에 오는 포물선이 되도록 중력까지 같이 정한다.
    void LaunchArc(float Height, float Time);

    // 맞고 밀려나기. Dir은 정규화하지 않아도 된다.
    void AddKnockback(const FVector2& Dir, float Speed);

    // 남은 높이를 되돌리고 그 자리에 세운다. (착지 콜백은 부르지 않는다)
    void ForceLand();

    // ── 물어보기 ────────────────────────────────────────────────────────────

    bool  IsInAir() const     { return mInAir; }
    float GetHeight() const   { return mHeight; }
    float GetVelZ() const     { return mVelZ; }

    void SetOnLanded(std::function<void(float)> Func) { mOnLanded = std::move(Func); }

    // ── 설정 ────────────────────────────────────────────────────────────────

    float GetGravity() const        { return mGravity; }
    void  SetGravity(float Value)   { mGravity = (Value < 0.01f) ? 0.01f : Value; }

    float GetBounce() const         { return mBounce; }
    void  SetBounce(float Value)    { mBounce = (Value < 0.f) ? 0.f : ((Value > 0.9f) ? 0.9f : Value); }

    float GetKnockDamp() const      { return mKnockDamp; }
    void  SetKnockDamp(float Value) { mKnockDamp = (Value < 0.f) ? 0.f : Value; }

public:
    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;

    virtual void Save(std::ofstream& File) const override;
    virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;

    virtual CHeightComponent* Clone() const override { return new CHeightComponent(*this); }
    virtual std::string GetTypeName() const override { return "CHeightComponent"; }

private:
    // 밀어 올릴 메시를 찾는다. 액터가 아직 메시를 루트로 쓰고 있으면
    // 그 위에 빈 루트를 끼워 넣어 지면 위치와 높이를 갈라놓는다.
    bool BindVisualFromOwner();

    // 이번 프레임에 늘어난 높이만 메시에 더한다.
    // 절대 위치로 덮어쓰면 그 사이에 들어온 다른 이동이 지워진다.
    void ApplyHeight();
};
