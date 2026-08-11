#include "Chaser.h"

#include "World/MeshComponent.h"
#include "World/CharacterMovementComponent.h"

#include "MainWorld.h"
#include "Bullet.h"

#include "LogManager.h"

CChaser::CChaser()
{}

CChaser::CChaser(const CChaser& src)
{}

CChaser::CChaser(CChaser&& src) noexcept
{}

CChaser::~CChaser()
{}

bool CChaser::Init()
{
	mMeshComp = CreateComponent<CMeshComponent>("Root");

	mMoveComp = CreateComponent<CCharacterMovementComponent>("MoveComp");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetMesh("ColorRect");
		Mesh->SetShader("Color2D");
		Mesh->SetRelativeScale(20.f, 20.f);
	}


	auto MoveComp = mMoveComp.lock();

	if (MoveComp)
	{
		MoveComp->SetUpdateComp(mRoot);
		MoveComp->SetSpeed(50.f);
	}


	return true;
}

void CChaser::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);

	auto Target = mTarget.lock();

	if (Target)
	{
		FVector3 Dir = Target->GetWorldPos() - GetWorldPos();
		Dir.Normalize();

		auto MoveComp = mMoveComp.lock();

		if (MoveComp)
		{
			MoveComp->SetMoveDir(Dir);
		}

		float TargetDist = Target->GetWorldPos().Distance(GetWorldPos());
		
		if (TargetDist <= mDistance)
		{
			Destroy();
		}

	}

}

void CChaser::Destroy()
{
	CActor::Destroy();
}
