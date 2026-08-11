#include "MonsterMoveState.h"
#include "World/AIComponent.h"
#include "World/BlackBoard.h"

#include "World/Actor.h"

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
		auto BB = OwnerAI->GetBlackBoard().lock();

		if (BB)
		{
			int Count = BB->GetInt("MoveCount");
			++Count;
			BB->SetInt("MoveCount", Count);

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
