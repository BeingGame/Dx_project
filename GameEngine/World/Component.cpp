#include "Component.h"
#include "World.h"
#include "Actor.h"

CComponent::CComponent()
{}

CComponent::CComponent(const CComponent& src)
{
	mWorld = src.mWorld;
	mOwner = src.mOwner;
	mName = src.mName;
	mType = src.mType;
	mRenderEnable = src.mRenderEnable;
}

CComponent::CComponent(CComponent&& src) noexcept
{
	mWorld = src.mWorld;
	mOwner = src.mOwner;
	mName = src.mName;
	mType = src.mType;
	mRenderEnable = src.mRenderEnable;
}

CComponent::~CComponent()
{}

void CComponent::SetWorld(const std::weak_ptr<class CWorld>& World)
{
	mWorld = World;
}

void CComponent::SetOwner(const std::weak_ptr<class CActor>& Owner)
{
	mOwner = Owner;
}

bool CComponent::Init()
{
	return true;
}

void CComponent::Update(float DeltaTime)
{}

void CComponent::PostUpdate(float DeltaTime)
{}

void CComponent::Render()
{}

void CComponent::PostRender()
{}

void CComponent::Destroy()
{
	mAlive = false;
}
