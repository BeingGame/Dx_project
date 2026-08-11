#pragma once

#include "../EngineInfo.h"

//월드 콜리전 클래스
//월드에 존재하는 콜라이더 컴포넌트를 이용해
//충돌을 진행할 클래스
//월드마다 각자 존재할수 있기 때문에 싱글톤x

class CWorldCollision
{
public:
	CWorldCollision();
	~CWorldCollision();

private:
	std::weak_ptr<class CWorld> mWorld;

	//충돌체를 관리하기위해 리스트를 만들어준다.
	//동적배열인 벡터로 만들면 매번 갱신되는 시간이 늘어나버린다.
	//따라서 list로 만들어준다.
	std::list<std::weak_ptr<class CCollider>> mColliderList;

	std::shared_ptr<class CCollisionQuadTree> mQuadTree;

	std::weak_ptr<class CCollider> mHoveredActor;

	//콜리전 체크시간
	//필요하다면 콜리전 체크에 텀을 두게 만들어준다.
	float mInterval = 0.f;
	float mIntervalTime = 0.f;

public:
	void SetWorld(const std::weak_ptr<class CWorld> World)
	{
		mWorld = World;
	}

	void SetInterval(float Interval)
	{
		mInterval = Interval;
	}

	void AddCollider(const std::weak_ptr<class CCollider>& Collider);

public:
	bool Init();
	void Update(float DeltaTime);
	void Render();
	void ReturnNodePool();
	void UpdateInfo();


};

