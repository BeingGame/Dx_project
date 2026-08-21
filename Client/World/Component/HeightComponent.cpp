#include "HeightComponent.h"

#include "World/Actor.h"
#include "World/SceneComponent.h"

#include <fstream>

CHeightComponent::CHeightComponent()
{}

CHeightComponent::CHeightComponent(const CHeightComponent& src)
    : CActorComponent(src)
{
    mGravity      = src.mGravity;
    mBounce       = src.mBounce;
    mMinBounceVel = src.mMinBounceVel;
    mKnockDamp    = src.mKnockDamp;
}

CHeightComponent::~CHeightComponent()
{}

bool CHeightComponent::Init()
{
    CActorComponent::Init();
    return true;
}

bool CHeightComponent::BindVisualFromOwner()
{
    auto Owner = mOwner.lock();

    if (!Owner)
    {
        return false;
    }

    //메시가 곧 루트인 액터는 지면 위치와 높이를 나눌 데가 없다.
    //빈 루트를 끼워 넣으면 옛 루트(메시)가 자식으로 내려온다.
    Owner->InsertBareRoot();

    auto Root = Owner->GetRootComponent().lock();

    if (!Root)
    {
        return false;
    }

    //루트의 첫 자식을 들어올린다. 그림자처럼 땅에 남아야 하는 것을 나중에 붙이려면
    //그때 이름으로 고르게 바꾸면 된다.
    for (const auto& Child : Root->GetChildList())
    {
        if (auto Comp = Child.lock())
        {
            mVisual = Comp;
            return true;
        }
    }

    return false;
}

void CHeightComponent::Launch(float VelZ)
{
    if (VelZ <= 0.f)
    {
        return;
    }

    if (mVisual.expired())
    {
        BindVisualFromOwner();
    }

    if (mVisual.expired())
    {
        return;
    }

    //이미 떠 있어도 속도만 갈아끼운다. 높이는 그대로 두어야
    //공중에서 다시 맞았을 때 바닥으로 순간이동하지 않는다.
    mVelZ  = VelZ;
    mInAir = true;
}

void CHeightComponent::LaunchArc(float Height, float Time)
{
    if (Height <= 0.f || Time <= 0.f)
    {
        return;
    }

    //꼭대기가 정확히 절반 시점에 오는 포물선.
    //  올라가는 처음 속도 v = 4h / T,  중력 g = 2v / T
    const float VelZ = 4.f * Height / Time;

    mGravity = 2.f * VelZ / Time;

    Launch(VelZ);
}

void CHeightComponent::AddKnockback(const FVector2& Dir, float Speed)
{
    if (Speed <= 0.f)
    {
        return;
    }

    FVector2 Normalized = Dir;
    const float Len = sqrtf(Dir.x * Dir.x + Dir.y * Dir.y);

    if (Len <= 0.0001f)
    {
        return;
    }

    Normalized.x /= Len;
    Normalized.y /= Len;

    mKnockVel.x += Normalized.x * Speed;
    mKnockVel.y += Normalized.y * Speed;
}

void CHeightComponent::ForceLand()
{
    if (!mInAir)
    {
        return;
    }

    if (auto Visual = mVisual.lock())
    {
        Visual->AddRelativePos(0.f, -mAppliedOffset, 0.f);
    }

    mAppliedOffset = 0.f;
    mHeight        = 0.f;
    mVelZ          = 0.f;
    mInAir         = false;
}

void CHeightComponent::ApplyHeight()
{
    auto Visual = mVisual.lock();

    if (!Visual)
    {
        return;
    }

    //이번 프레임에 늘어난 만큼만 더한다.
    Visual->AddRelativePos(0.f, mHeight - mAppliedOffset, 0.f);
    mAppliedOffset = mHeight;
}

void CHeightComponent::Update(float DeltaTime)
{
    CActorComponent::Update(DeltaTime);

    if (mVisual.expired())
    {
        BindVisualFromOwner();
    }

    // ── 수평 넉백 ──
    // 지면 위치를 밀어야 하므로 루트에 더한다. 높이와는 별개의 축이다.
    if (mKnockVel.x != 0.f || mKnockVel.y != 0.f)
    {
        if (auto Owner = mOwner.lock())
        {
            if (auto Root = Owner->GetRootComponent().lock())
            {
                Root->AddRelativePos(mKnockVel.x * DeltaTime, mKnockVel.y * DeltaTime, 0.f);
            }
        }

        //지수 감쇠. 프레임 시간이 들쭉날쭉해도 결과가 크게 안 변한다.
        const float Damp = 1.f - mKnockDamp * DeltaTime;

        if (Damp <= 0.f)
        {
            mKnockVel = FVector2(0.f, 0.f);
        }
        else
        {
            mKnockVel.x *= Damp;
            mKnockVel.y *= Damp;    

            //거의 멈췄으면 딱 끊는다. 안 그러면 영원히 아주 조금씩 흐른다.
            if (fabsf(mKnockVel.x) < 0.01f) mKnockVel.x = 0.f;
            if (fabsf(mKnockVel.y) < 0.01f) mKnockVel.y = 0.f;
        }
    }

    // ── 높이 ──
    if (!mInAir)
    {
        return;
    }

    mVelZ  -= mGravity * DeltaTime;
    mHeight += mVelZ * DeltaTime;

    if (mHeight > 0.f)
    {
        ApplyHeight();
        return;
    }

    //바닥에 닿았다. 부딪힌 속도는 다운으로 보낼지 판단하는 쪽에서 쓴다.
    const float ImpactVel = -mVelZ;

    mHeight = 0.f;
    ApplyHeight();

    //반발이 있고 충분히 세게 부딪혔으면 한 번 더 튄다.
    if (mBounce > 0.f && ImpactVel * mBounce >= mMinBounceVel)
    {
        mVelZ = ImpactVel * mBounce;

        if (mOnLanded)
        {
            mOnLanded(ImpactVel);
        }

        return;
    }

    mVelZ  = 0.f;
    mInAir = false;

    if (mOnLanded)
    {
        mOnLanded(ImpactVel);
    }
}

void CHeightComponent::Save(std::ofstream& File) const
{
    CActorComponent::Save(File);

    File << "Gravity="   << mGravity   << "\n";
    File << "Bounce="    << mBounce    << "\n";
    File << "KnockDamp=" << mKnockDamp << "\n";
}

void CHeightComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
    CActorComponent::Load(Props);

    auto GetF = [&Props](const char* Key, float& Out)
    {
        auto Found = Props.find(Key);
        if (Found == Props.end()) return;
        try { Out = std::stof(Found->second); }
        catch (...) {}
    };

    GetF("Gravity",   mGravity);
    GetF("Bounce",    mBounce);
    GetF("KnockDamp", mKnockDamp);

    //씬 컴포넌트가 먼저 복원되므로 여기서 메시를 찾을 수 있다.
    BindVisualFromOwner();
}
