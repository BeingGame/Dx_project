#include "MovementComponent.h"


#include "SceneComponent.h"

CMovementComponent::CMovementComponent()
{}

CMovementComponent::CMovementComponent(const CMovementComponent& src)
	:CActorComponent(src)
{
	mMoveDir = src.mMoveDir;
	mMoveSpeed = src.mMoveSpeed;
}

CMovementComponent::CMovementComponent(CMovementComponent&& src) noexcept
	:CActorComponent(std::move(src))
{}

CMovementComponent::~CMovementComponent()
{}

void CMovementComponent::SetMoveDir(const FVector3& Dir)
{
	mMoveDir = Dir;
}

bool CMovementComponent::Init()
{
	CActorComponent::Init();

	return true;
}

void CMovementComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);

	auto Comp = mUpdateComp.lock();

	if (Comp)
	{
		if (!mMoveDir.IsZero())
		{
			mMoveDir.Normalize();

			Comp->AddRelativePos(mMoveDir * mMoveSpeed * DeltaTime);
		}
	}

	//mMoveDir = FVector3::Zero;
}

void CMovementComponent::PostUpdate(float DeltaTime)
{
	CActorComponent::PostUpdate(DeltaTime);
}

void CMovementComponent::Destroy()
{
	CActorComponent::Destroy();
}

void CMovementComponent::Save(std::ofstream& File) const
{
	CComponent::Save(File);
	File << "Speed=" << mMoveSpeed << "\n";
}

void CMovementComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
	auto it = Props.find("Speed");
	if (it != Props.end()) mMoveSpeed = std::stof(it->second);
}
