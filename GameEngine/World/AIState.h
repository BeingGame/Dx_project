#pragma once

#include "../EngineInfo.h"

class CAIState
{
	friend class CAIComponent;

public:
	CAIState();
	virtual ~CAIState();

protected:
	std::weak_ptr<class CAIComponent> mOwnerAI;
	std::weak_ptr<class CActor> mOwnerActor;

public:
	//상태에 진입했을때
	virtual void Enter() = 0;
	//상태가 진행되는중
	virtual void Update(float DeltaTime) = 0;
	//상태가 종료되었을때
	virtual void Exit() = 0;

};

