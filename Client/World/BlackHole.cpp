#include "BlackHole.h"
#include "World/MeshComponent.h"
#include "World/ColliderSphere2D.h"

CBlackHole::CBlackHole()
{}

CBlackHole::CBlackHole(const CBlackHole & src)
{}

CBlackHole::CBlackHole(CBlackHole && src) noexcept
{}

CBlackHole::~CBlackHole()
{}

bool CBlackHole::Init()
{
	mMeshComp = CreateComponent<CMeshComponent>("Root");
	mSphere = CreateComponent<CColliderSphere2D>("Sphere");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetMesh("TexRect");
		Mesh->SetShader("Material");
		Mesh->AddTexture(0, "BlackHole", TEXT("blackhole.png"));
		Mesh->SetRelativeScale(250.f, 250.f);
	}

	auto Sphere = mSphere.lock();

	if (Sphere)
	{
		Sphere->SetInheritScale(false);
		Sphere->SetRadius(200.f);
		Sphere->SetDebugDraw(true);
		Sphere->SetCollisionProfile("OverlapAll");
		Sphere->SetBeginOverlapFunc(this, &CBlackHole::OnBeginOverlap);
		Sphere->SetEndOverlapFunc(this, &CBlackHole::OnEndOverlap);
	}

	return true;
}

void CBlackHole::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);


	//충돌체에 닿은 액터들을 끌어당긴다.

	auto iter = mActorArray.begin();
	auto iterEnd = mActorArray.end();

	for (; iter != iterEnd;)
	{
		auto Actor = iter->lock();

		if (Actor && Actor->IsAlive())
		{
			//액터에게 끌어당기는 힘을 추가한다.
			FVector3 PullDir = GetWorldPos() - Actor->GetWorldPos();

			float Dist = PullDir.Length();

			if (Dist > 5.f)
			{
				PullDir.Normalize();

				Actor->AddWorldPos(PullDir * mPullSpeed * DeltaTime);
			}
			
			++iter;
		}
		else
		{
			iter = mActorArray.erase(iter);
		}
	}
}

void CBlackHole::OnBeginOverlap(const FVector3 & HitPoint, const FVector3 & Normal, std::weak_ptr<class CCollider> Collider)
{
	if (Collider.expired())
	{
		return;
	}

	auto Owner = Collider.lock()->GetOwner().lock();

	if (Owner)
	{
		mActorArray.push_back(Owner);
	}
}

void CBlackHole::OnEndOverlap(std::weak_ptr<class CCollider> Collider)
{
	if (Collider.expired())
	{
		return;
	}

	auto Owner = Collider.lock()->GetOwner().lock();

	if (!Owner)
	{
		return;
	}

	auto iter = mActorArray.begin();
	auto iterEnd = mActorArray.end();

	for (; iter != iterEnd;)
	{
		if (iter->lock() == Owner)
		{
			iter = mActorArray.erase(iter);
			break;
		}

		++iter;
	}

}
