#include "MonsterAttackState.h"
#include "World/AIComponent.h"
#include "World/BlackBoard.h"

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
		auto BB = OwnerAI->GetBlackBoard().lock();

		if (BB)
		{
			int Count = BB->GetInt("AttackCount");
			++Count;
			BB->SetInt("AttackCount", Count);

			LOG_DEBUG("Monster Attack Start AttackCount:", Count);
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
