#pragma once

#include "World/Actor.h"
#include <string>

// 짧게 한 번 재생하고 스스로 사라지는 이펙트 액터.
//
// 공격 타격 등에서 World->CreateActor<CEffect>() 로 소환한 뒤 Play()만 부르면 된다.
// 비반복 애니메이션이 끝나거나(IsCurrentAnimationEnd) 안전 시간을 넘기면
// Destroy()로 자멸한다. (수명 관리를 부르는 쪽이 신경 쓸 필요가 없다)
class CEffect : public CActor
{
public:
    CEffect();
    CEffect(const CEffect& src);
    virtual ~CEffect();

protected:
    std::weak_ptr<class CMeshComponent>        mMeshComp;
    std::weak_ptr<class CAnimation2DComponent> mAnimComp;

    std::string mAnimName;
    float mLife    = 0.f;    // 누적 생존 시간
    float mMaxLife = 3.f;    // 애니가 안 끝나는 사고 대비 강제 소멸 시간

    // ── 따라다니기(버프용) ──
    // 대상 액터를 매 프레임 따라간다. 대상이 사라지면 함께 소멸한다.
    // 따라다니는 동안에는 수명 제한을 두지 않는다. (외부에서 Destroy하거나 대상 소멸로만 끝)
    std::weak_ptr<class CActor> mFollowTarget;
    FVector3                    mFollowOffset;
    bool                        mFollowing = false;

public:
    // AnimName: 애니메이션 매니저에 등록된 이펙트 애니. Pos: 월드 위치.
    // Flip: 좌우 반전(Symmetry). Scale: 메시 크기(정사각).
    // FlipY: 상하 반전. TiltRad: 기울기(라디안).
    // Loop: 반복 재생. 켜면 애니가 끝나도 안 죽고 계속 돈다. (홀드 중 차징 효과 등)
    //       반복 효과는 스스로 안 끝나므로, 부르는 쪽이 Destroy()로 끄거나 안전 시간에 소멸한다.
    void Play(const std::string& AnimName, const FVector3& Pos, bool Flip, float Scale = 200.f,
        bool FlipY = false, float TiltRad = 0.f, bool Loop = false);

    // 대상 액터를 따라다니게 한다. (캐릭터 버프처럼) 대상 위치 + Offset에 매 프레임 붙는다.
    void SetFollow(const std::weak_ptr<class CActor>& Target, const FVector3& Offset);

    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;

    virtual std::string GetTypeName() const override { return "CEffect"; }
};
