#pragma once

#include "ActorComponent.h"

//FSM(유한 상태 기계)
//유한한 상태를 가진 어떠한 기계
//상태란 예를들어 몬스터의 하나의 행동을 상태라고 표현한다.

//유지보수, 가독성, 확장성
//유지보수: 각 상태를 각각 클래스로 제작하기 때문에, 하나의 상태에 문제가 생기면 그 상태에대한 코드만 고치면된다.
//가독성: 각 상태별로 코드를 제작하기 때문에, 각 상태별로 Update를 진행한다.
//확장성: 액터별로 행동을 작성하기 좋고, 새로운 행동이 추가될때 코드가 변경될 점이 없다.

//AIComponent
//각 상태별로 map에 저장되고, 필요시에 map에서 상태를 꺼내 업데이트 해준다.
//상태는 단 하나씩만 동작하도록 설계한다.
//FSM의 뼈대로 사용된다.

class CAIComponent :
	public CActorComponent
{
public:
	CAIComponent();
	CAIComponent(const CAIComponent& src);
	CAIComponent(CAIComponent&& src) noexcept;
	virtual ~CAIComponent();

protected:
	std::unordered_map<std::string, std::shared_ptr<class CAIState>> mStateMap;
	std::shared_ptr<class CAIState> mCurrentState;
	std::shared_ptr<class CBlackBoard> mBlackBoard;

public:
	void ChangeState(const std::string& Name);
	std::weak_ptr<class CAIState> FindState(const std::string& Name);
	std::shared_ptr<class CAIState> GetCurrentState() const
	{
		return mCurrentState;
	}

	std::weak_ptr<class CBlackBoard> GetBlackBoard() const
	{
		return mBlackBoard;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Destroy();

public:
	virtual CAIComponent* Clone() const
	{
		return new CAIComponent(*this);
	}

	virtual std::string GetTypeName() const override { return "CAIComponent"; }

public:
	template<typename T>
	void AddState(const std::string& Name)
	{
		std::shared_ptr<CAIState> State = std::make_shared<T>();

		State->mOwnerAI = GetThisPtr<CAIComponent>();
		State->mOwnerActor = mOwner;

		mStateMap.insert(std::make_pair(Name, State));
	}
};

