#include "AIComponent.h"
#include "AIState.h"
#include "BlackBoard.h"

CAIComponent::CAIComponent()
{}

CAIComponent::CAIComponent(const CAIComponent & src)
	:CActorComponent(src)
{}

CAIComponent::CAIComponent(CAIComponent && src) noexcept
	:CActorComponent(std::move(src))
{}

CAIComponent::~CAIComponent()
{}

void CAIComponent::ChangeState(const std::string & Name)
{
	auto iter = mStateMap.find(Name);

	if (iter == mStateMap.end())
	{
		return;
	}

	if (mCurrentState == iter->second)
	{
		return;
	}

	if (mCurrentState)
	{
		mCurrentState->Exit();
	}

	mCurrentState = iter->second;
	mCurrentState->Enter();
}

std::weak_ptr<class CAIState> CAIComponent::FindState(const std::string& Name)
{
	auto iter = mStateMap.find(Name);

	if (iter == mStateMap.end())
	{
		return std::weak_ptr<class CAIState>();
	}

	return iter->second;
}

bool CAIComponent::Init()
{
	CActorComponent::Init();

	mBlackBoard = std::make_shared<CBlackBoard>();

	return true;
}

void CAIComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);

	if (mCurrentState)
	{
		mCurrentState->Update(DeltaTime);
	}

}

void CAIComponent::Destroy()
{
	CActorComponent::Destroy();
}
