#include "Shield.h"

#include "World/MeshComponent.h"

CShield::CShield()
{}

CShield::CShield(const CShield& src)
{}

CShield::CShield(CShield&& src) noexcept
{}

CShield::~CShield()
{}

bool CShield::Init()
{
	mMeshComp = CreateComponent<CMeshComponent>("Root");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetMesh("ColorRect");
		Mesh->SetShader("Color2D");
		Mesh->SetRelativeScale(20.f, 20.f);
	}
	
	mActorTag = "Player";
	//SetRadius(100.f);

	return true;
}

void CShield::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);

	auto Owner = mOwner.lock();

	if (Owner)
	{
		//플레이어 위치 기준으로 공전
		mAngle += mRotationSpeed * DeltaTime;
		if (mAngle >= 360.f)
		{
			mAngle -= 360.f;
		}

		float Radian = DirectX::XMConvertToRadians(mAngle);
		FVector3 Offset(cosf(Radian) * mDistance, sinf(Radian) * mDistance, 0.f);
		SetWorldPos(Owner->GetWorldPos() + Offset);
	}



}

void CShield::Destroy()
{
	CActor::Destroy();
}

void CShield::OnCollision(const std::weak_ptr<CActor>& HitActor)
{

}
