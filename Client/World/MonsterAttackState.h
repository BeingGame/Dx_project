#pragma once

#include "World/AIState.h"

class CMonsterAttackState :
    public CAIState
{
public:
	CMonsterAttackState();
	virtual ~CMonsterAttackState();

protected:
	float mChangeTime = 2.f;

public:
	//상태에 진입했을때
	virtual void Enter();
	//상태가 진행되는중
	virtual void Update(float DeltaTime);
	//상태가 종료되었을때
	virtual void Exit();
};

