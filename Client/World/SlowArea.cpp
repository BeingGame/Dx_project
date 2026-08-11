#include "SlowArea.h"

#include "World/ColliderSphere2D.h"
#include "World/MeshComponent.h"

#include "MainWorld.h"

#include "Player.h"
#include "Bullet.h"

CSlowArea::CSlowArea()
{}

CSlowArea::CSlowArea(const CSlowArea& src)
{}

CSlowArea::CSlowArea(CSlowArea&& src) noexcept
{}

CSlowArea::~CSlowArea()
{}

bool CSlowArea::Init()
{
	mSphere = CreateComponent<CColliderSphere2D>("Root");

	auto Sphere = mSphere.lock();

	if (Sphere)
	{
		Sphere->SetDebugDraw(true);
		Sphere->SetRadius(450.f);
		Sphere->SetCollisionProfile("Skill");

		Sphere->SetBeginOverlapFunc<CSlowArea>(this, &CSlowArea::OnBeginOverlap);
		Sphere->SetEndOverlapFunc<CSlowArea>(this, &CSlowArea::OnEndOverlap);
	}

	mActorTag = "Monster";
	//SetRadius(150.f);

	return true;
}

void CSlowArea::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);

	//현재 영역에 들어온 액터가 있는지 확인
	//auto World = mWorld.lock();

	//if (World)
	//{
	//	auto List = World->GetActorList();


	//	for (auto Actor : List)
	//	{
	//		auto iter = mSlowedActormap.find(Actor.second);

	//		if (iter != mSlowedActormap.end())
	//		{
	//			continue;
	//		}

	//		float Speed = 0.f;

	//		if (auto _Actor = std::dynamic_pointer_cast<CPlayer>(Actor.second))
	//		{
	//			Speed = _Actor->GetSpeed();
	//		}
	//		else if (auto _Actor = std::dynamic_pointer_cast<CBullet>(Actor.second))
	//		{
	//			Speed = _Actor->GetSpeed();
	//		}

	//		FVector3 ActorPos = Actor.second->GetWorldPos();

	//		float Dist = ActorPos.Distance(GetWorldPos());

	//		if (Dist <= mRadius)
	//		{
	//			mSlowedActormap.insert(std::make_pair(Actor.second, Speed));
	//		}
	//	}
	//}

	////액터맵에 데이터가 액터가 존재하면 해당 액터들의 속도를 갱신해준다.
	//for (auto Actor : mSlowedActormap)
	//{
	//	if (auto _Actor = std::dynamic_pointer_cast<CPlayer>(Actor.first))
	//	{
	//		_Actor->SetSpeed(Actor.second / mSlowRatio);
	//	}
	//	else if (auto _Actor = std::dynamic_pointer_cast<CBullet>(Actor.first))
	//	{
	//		_Actor->SetSpeed(Actor.second / mSlowRatio);
	//	}
	//}

	////액터가 범위를 빠져나가면 원래 속도로 돌려준다.
	//auto iter = mSlowedActormap.begin();
	//auto iterEnd = mSlowedActormap.end();

	//for (;iter !=iterEnd ; ++iter)
	//{
	//	FVector3 ActorPos = iter->first->GetWorldPos();

	//	float Dist = ActorPos.Distance(GetWorldPos());


	//	if (Dist > mRadius)
	//	{
	//		if (auto _Actor = std::dynamic_pointer_cast<CPlayer>(iter->first))
	//		{
	//			_Actor->SetSpeed(_Actor->GetCurrentSpeed());
	//		}
	//		else if (auto _Actor = std::dynamic_pointer_cast<CBullet>(iter->first))
	//		{
	//			_Actor->SetSpeed(_Actor->GetCurrentSpeed());
	//		}

	//		iter = mSlowedActormap.erase(iter);
	//		iterEnd = mSlowedActormap.end();

	//	}

	//}
}

void CSlowArea::Destroy()
{
	CActor::Destroy();
}

void CSlowArea::OnBeginOverlap(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<CCollider> Collider)
{
	auto _Collider = Collider.lock();

	if (_Collider)
	{

		auto Actor = _Collider->GetOwner().lock();

		if (Actor)
		{
			if (auto _Actor = std::dynamic_pointer_cast<CPlayer>(Actor))
			{
				_Actor->SetSpeed(_Actor->GetSpeed() / mSlowRatio);
			}
			else if (auto _Actor = std::dynamic_pointer_cast<CBullet>(Actor))
			{
				_Actor->SetSpeed(_Actor->GetSpeed() /  mSlowRatio);
			}
		}
	}
}

void CSlowArea::OnEndOverlap(std::weak_ptr<class CCollider> Collider)
{
	auto _Collider = Collider.lock();

	if (_Collider)
	{

		auto Actor = _Collider->GetOwner().lock();

		if (Actor)
		{
			if (auto _Actor = std::dynamic_pointer_cast<CPlayer>(Actor))
			{
				_Actor->SetSpeed(_Actor->GetSpeed() * mSlowRatio);
			}
			else if (auto _Actor = std::dynamic_pointer_cast<CBullet>(Actor))
			{
				_Actor->SetSpeed(_Actor->GetSpeed() * mSlowRatio);
			}
		}
	}
}
