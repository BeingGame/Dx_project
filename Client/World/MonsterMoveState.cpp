#include "MonsterMoveState.h"
#include "World/AIComponent.h"
#include "World/BlackBoard.h"

#include "World/Actor.h"
#include "Component/ActionStateComponent.h"

#include "LogManager.h"

CMonsterMoveState::CMonsterMoveState()
{}

CMonsterMoveState::~CMonsterMoveState()
{}

void CMonsterMoveState::Enter()
{
	auto OwnerAI = mOwnerAI.lock();

	if (OwnerAI)
	{
		auto BlackBoard = OwnerAI->GetBlackBoard().lock();

		if (BlackBoard)
		{
			int Count = BlackBoard->GetInt("MoveCount");
			++Count;
			BlackBoard->SetInt("MoveCount", Count);

			LOG_DEBUG("Monster Move Start MoveCount:", Count);
		}
	}
}

void CMonsterMoveState::Update(float DeltaTime)
{
	auto OwnerActor = mOwnerActor.lock();

	if (OwnerActor)
	{
		OwnerActor->AddWorldPos(OwnerActor->GetAxis(EAxis::Y) * 100.f * DeltaTime);

		//액션 상태 컴포넌트가 붙어 있으면 이동 의도를 넘겨준다.
		//플레이어의 DirectionInput과 완전히 같은 통로다. (애니메이션은 저쪽이 고른다)
		for (const auto& Comp : OwnerActor->GetActorCompList())
		{
			auto State = std::dynamic_pointer_cast<CActionStateComponent>(Comp);

			if (State)
			{
				//전진 중이므로 위쪽 방향, 달리기는 아님
				State->SetMoveInput(FVector2(0.f, 1.f), false);
				break;
			}
		}
	}



	/*mChangeTime -= DeltaTime;

	if (mChangeTime <= 0.f)
	{
		mChangeTime = 2.f;
		auto OwnerAI = mOwnerAI.lock();

		if (OwnerAI)
		{
			OwnerAI->ChangeState("Attack");
		}
	}*/
}

void CMonsterMoveState::Exit()
{
	LOG_DEBUG("Monster Move End");
}
