#include "ActorComponent.h"

CActorComponent::CActorComponent()
{
	mType = EComponentType::Actor;
}

CActorComponent::CActorComponent(const CActorComponent& src)
	:CComponent(src)
{}

CActorComponent::CActorComponent(CActorComponent&& src) noexcept
	:CComponent(std::move(src))
{}

CActorComponent::~CActorComponent()
{}

bool CActorComponent::Init()
{
	CComponent::Init();

	return true;
}

//액터 컴포넌트는 주로 업데이트와 post업데이트를 사용한다.

void CActorComponent::Update(float DeltaTime)
{
	CComponent::Update(DeltaTime);
}

void CActorComponent::PostUpdate(float DeltaTime)
{
	CComponent::PostUpdate(DeltaTime);
}

void CActorComponent::Render()
{
	CComponent::Render();
}

void CActorComponent::PostRender()
{
	CComponent::PostRender();
}

void CActorComponent::Destroy()
{
	CComponent::Destroy();
}
