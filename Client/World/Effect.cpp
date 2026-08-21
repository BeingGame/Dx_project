#include "Effect.h"

#include "World/MeshComponent.h"
#include "World/Animation2DComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/AnimationManager.h"
#include "Asset/Animation2D.h"

CEffect::CEffect()
{}

CEffect::CEffect(const CEffect& src)
    : CActor(src)
{}

CEffect::~CEffect()
{}

bool CEffect::Init()
{
    CActor::Init();

    mMeshComp = CreateComponent<CMeshComponent>("Root");
    mAnimComp = CreateComponent<CAnimation2DComponent>("Anim");

    if (auto Mesh = mMeshComp.lock())
    {
        Mesh->SetMesh("TexRect");
        Mesh->SetShader("Animation2D");   // 스프라이트 시트 UV를 읽는 셰이더
        Mesh->SetBlendState(0, "AlphaBlend");
        Mesh->SetRelativeScale(200.f, 200.f);
    }

    if (auto Anim = mAnimComp.lock())
    {
        // 애니 컴포넌트가 메시의 텍스처/UV를 갱신하도록 물려준다.
        Anim->SetUpdateComponent(mMeshComp);
    }

    return true;
}

void CEffect::Play(const std::string& AnimName, const FVector3& Pos, bool Flip, float Scale,
    bool FlipY, float TiltRad, bool Loop)
{
    mAnimName = AnimName;
    SetWorldPos(Pos);

    //반복 효과는 애니가 스스로 안 끝나므로(IsCurrentAnimationEnd가 계속 false) 외부에서
    //Destroy()로 꺼주는 게 정상 경로다. 그래도 방치되면 새지 않도록 안전 시간을 넉넉히 둔다.
    if (Loop)
    {
        mMaxLife = 60.f;
    }

    if (auto Mesh = mMeshComp.lock())
    {
        Mesh->SetRelativeScale(Scale, Scale);
    }

    if (auto Anim = mAnimComp.lock())
    {
        if (!AnimName.empty())
        {
            // .anim2d 에셋에 저장된 PlayRate를 반영한다.
            // 예전에는 항상 1.0으로 등록해서, 애님 에디터에서 PlayRate를 늘려
            // 저장해도 이펙트로 재생할 때는 원래 속도로만 나왔다. 에셋이 재생
            // 설정을 들고 있으면(HasPlaySettings) 그 PlayRate를 그대로 쓴다.
            float Rate = 1.f;
            if (auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D))
            {
                if (auto Asset = AnimMgr->FindAnimation(AnimName).lock(); Asset && Asset->HasPlaySettings())
                    Rate = Asset->GetPlayRate();
            }

            // Loop=false면 비반복(끝을 IsCurrentAnimationEnd로 감지). true면 계속 돈다.
            Anim->AddAnimation(AnimName, 1.f, Rate, Loop);
            Anim->PlayAnimation(AnimName);   // 처음부터 재생
            Anim->SetSymmetry(AnimName, Flip);       // 좌우
            Anim->SetSymmetryV(AnimName, FlipY);     // 상하
            Anim->SetSequenceRotation(AnimName, TiltRad);   // 기울기
        }
    }
}

void CEffect::SetFollow(const std::weak_ptr<class CActor>& Target, const FVector3& Offset)
{
    mFollowTarget = Target;
    mFollowOffset = Offset;
    mFollowing    = true;
}

void CEffect::Update(float DeltaTime)
{
    CActor::Update(DeltaTime);

    // ── 따라다니기(버프) ──
    // 대상 위치 + 오프셋에 매 프레임 붙는다. 대상이 사라지면 함께 소멸.
    // 따라다니는 동안에는 수명 제한 없이 유지된다. (외부에서 끄거나 대상 소멸로만 끝)
    if (mFollowing)
    {
        auto Target = mFollowTarget.lock();

        if (!Target)
        {
            Destroy();
            return;
        }

        SetWorldPos(Target->GetWorldPos() + mFollowOffset);
        return;
    }

    mLife += DeltaTime;

    bool bEnded = false;

    if (auto Anim = mAnimComp.lock(); Anim && !mAnimName.empty())
    {
        bEnded = Anim->IsCurrentAnimationEnd();
    }

    // 비반복 애니가 끝났거나, 안전 시간을 넘기면(애니가 없거나 안 끝나는 사고) 자멸.
    // (수명이 다하면 월드가 IsAlive()를 보고 알아서 목록에서 지운다)
    if (bEnded || mLife >= mMaxLife)
    {
        Destroy();
    }
}
