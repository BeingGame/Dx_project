#include "MonsterAttackState.h"
#include "World/AIComponent.h"
#include "World/BlackBoard.h"

#include "World/Actor.h"
#include "Component/ActionStateComponent.h"

#include "LogManager.h"

CMonsterAttackState::CMonsterAttackState()
{}

CMonsterAttackState::~CMonsterAttackState()
{}

void CMonsterAttackState::Enter()
{
	auto OwnerAI = mOwnerAI.lock();

	if (OwnerAI)
	{
		auto BlackBoard = OwnerAI->GetBlackBoard().lock();

		if (BlackBoard)
		{
			int Count = BlackBoard->GetInt("AttackCount");
			++Count;
			BlackBoard->SetInt("AttackCount", Count);

			LOG_DEBUG("Monster Attack Start AttackCount:", Count);
		}
	}

	//공격 동작 자체는 액션 상태 컴포넌트가 재생한다.
	//AI는 "공격하겠다"만 말하고, 받아줄지는 저쪽이 우선순위를 보고 정한다.
	auto OwnerActor = mOwnerActor.lock();

	if (OwnerActor)
	{
		for (const auto& Comp : OwnerActor->GetActorCompList())
		{
			auto State = std::dynamic_pointer_cast<CActionStateComponent>(Comp);

			if (State)
			{
				State->RequestAttack();
				break;
			}
		}
	}
}

void CMonsterAttackState::Update(float DeltaTime)
{
	mChangeTime -= DeltaTime;

	if (mChangeTime <= 0.f)
	{
		mChangeTime = 2.f;
		auto OwnerAI = mOwnerAI.lock();

		if (OwnerAI)
		{
			OwnerAI->ChangeState("Move");
		}
	}
}

void CMonsterAttackState::Exit()
{
	LOG_DEBUG("Monster Attack End");
}
