#include "MovementComponent.h"


#include "SceneComponent.h"
#include "Actor.h"

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

bool CMovementComponent::BindUpdateCompFromOwner()
{
	//에디터에서 붙이거나 월드에서 불러온 컴포넌트는 아무도 짝을 지어주지 않는다.
	//(SetUpdateComp를 직접 부르는 건 Player/Monster 같은 하드코딩 액터뿐)
	//그대로 두면 이동값을 받아도 아무도 안 움직이므로 여기서 루트를 잡는다.
	auto Owner = mOwner.lock();

	if (!Owner)
	{
		return false;
	}

	auto Root = Owner->GetRootComponent().lock();

	if (!Root)
	{
		return false;
	}

	mUpdateComp = Root;

	return true;
}

void CMovementComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);

	if (mUpdateComp.expired())
	{
		BindUpdateCompFromOwner();
	}

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
