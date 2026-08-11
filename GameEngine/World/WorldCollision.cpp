#include "WorldCollision.h"

#include "Collider.h"
#include "Actor.h"

#include "CollisionQuadTree.h"

#include "../LogManager.h"

#include "World.h"
#include "Input.h"
#include "UIManager.h"

CWorldCollision::CWorldCollision()
{}

CWorldCollision::~CWorldCollision()
{}

void CWorldCollision::AddCollider(const std::weak_ptr<class CCollider>& Collider)
{
	mColliderList.push_back(Collider);
}

bool CWorldCollision::Init()
{
	mQuadTree = std::make_shared<CCollisionQuadTree>();

	mQuadTree->SetWorld(mWorld);

	if (!mQuadTree->Init())
	{
		return false;
	}

	return true;
}

void CWorldCollision::Update(float DeltaTime)
{
	mQuadTree->Update(DeltaTime);

	auto iter = mColliderList.begin();
	auto iterEnd = mColliderList.end();

	for (; iter != iterEnd;)
	{
		if (iter->expired())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			continue;
		}

		auto Collider = iter->lock();

		if (!Collider->IsAlive())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			continue;
		}

		if (!Collider->IsEnable())
		{
			++iter;
			continue;
		}

		Collider->UpdateInfo();

		mQuadTree->AddCollider(Collider);

		++iter;
	}

	//충돌 연산을 진행하기전에 마우스와 위젯 충돌을 실행한다.

	auto World = mWorld.lock();

	auto Input = World->GetInput().lock();

	auto UIManager = World->GetUIManager().lock();

	FVector2 MousePos = Input->GetMousePos();

	if (!UIManager->CollisionMouse(DeltaTime, MousePos))
	{
		//UI의 충돌이 없다면 월드상 액터와의 충돌연산을 진행한다.
		FVector2 WorldMousePos = Input->GetWorldMousePos();
		mQuadTree->CollisionMouse(mHoveredActor, DeltaTime, WorldMousePos);
	}
	else if (!mHoveredActor.expired())
	{
		mHoveredActor.reset();
	}

	//쿼드트리에 충돌체가 전부 들어갔으면 충돌연산을 실행한다.
	mQuadTree->Collision(DeltaTime);

	//if (!mColliderList.empty())
	//{
	//	//충돌의 계산은 2^n만큼 계산한다.
	//	//오브젝트(액터)끼리 충돌체가 있을때 1:1로 계산되기 때문

	//	//충돌연산할때 자기자신과의 충돌연산은 의미가 없기 때문에
	//	//미리 한칸 당겨서 다른 반복자와 충돌 연산을 시도한다.
	//	auto iter = mColliderList.begin();
	//	auto iterEnd = mColliderList.end();
	//	--iterEnd;

	//	//다른 반복자의 끝도 도중에 컴포넌트가 제거될수 있기 때문에
	//	//반복문의 바깥에 선언해서 제거될때마다 갱신한다.
	//	auto iter1End = mColliderList.end();

	//	for (; iter != iterEnd;)
	//	{
	//		//반복자 무효화 방지
	//		if (mColliderList.size() < 2)
	//		{
	//			break;
	//		}

	//		if (iter->expired())
	//		{
	//			iter = mColliderList.erase(iter);
	//			iterEnd = mColliderList.end();
	//			iter1End = iterEnd;
	//			--iterEnd;
	//			continue;
	//		}

	//		auto SrcCollider = iter->lock();

	//		if (!SrcCollider->IsAlive())
	//		{
	//			iter = mColliderList.erase(iter);
	//			iterEnd = mColliderList.end();
	//			iter1End = iterEnd;
	//			--iterEnd;
	//			continue;
	//		}

	//		if (!SrcCollider->IsEnable())
	//		{
	//			++iter;
	//			continue;
	//		}

	//		//컴포넌트가 파괴되지 않고, 활성화 상태라면 충돌 연산을 시작한다.

	//		//먼저 프로파일을 확인해서 프로파일이 enable인지 확인한다.
	//		std::shared_ptr<FCollisionProfile> SrcProfile = SrcCollider->GetCollisionProfile();

	//		if (!SrcProfile || !SrcProfile->Enable)
	//		{
	//			++iter;
	//			continue;
	//		}

	//		//문제가 없으면 현재 충돌체의 다음 충돌체부터 충돌 연산을 시작한다.
	//		auto iter1 = iter;
	//		++iter1;

	//		for (; iter1 != iter1End;)
	//		{
	//			//반복자 무효화 방지
	//			if (mColliderList.size() < 2)
	//			{
	//				break;
	//			}

	//			if (iter1->expired())
	//			{
	//				iter1 = mColliderList.erase(iter1);
	//				iterEnd = mColliderList.end();
	//				iter1End = iterEnd;
	//				--iterEnd;
	//				continue;
	//			}

	//			auto DestCollider = iter1->lock();

	//			if (!DestCollider->IsAlive())
	//			{
	//				iter1 = mColliderList.erase(iter1);
	//				iterEnd = mColliderList.end();
	//				iter1End = iterEnd;
	//				--iterEnd;
	//				continue;
	//			}

	//			if (!DestCollider->IsEnable())
	//			{
	//				++iter1;
	//				continue;
	//			}

	//			std::shared_ptr<FCollisionProfile> DestProfile = DestCollider->GetCollisionProfile();

	//			//두 충돌체가 각각의 채널이 충돌할수 있는지를 확인한다.
	//			if (!DestProfile || !DestProfile->Enable)
	//			{
	//				++iter1;
	//				continue;
	//			}
	//			//프로파일 둘중 하나라도 ignore 채널이면 충돌처리를 건너뛴다.
	//			else if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Ignore ||
	//				DestProfile->Interaction[SrcProfile->Channel->Channel] == ECollisionInteraction::Ignore)
	//			{
	//				++iter1;
	//				continue;
	//			}
	//			//두 프로파일의 상호작용이 다르면 건너뛴다.
	//			else if (SrcProfile->Interaction[DestProfile->Channel->Channel] != DestProfile->Interaction[SrcProfile->Channel->Channel])
	//			{
	//				++iter1;
	//				continue;
	//			}

	//			//충돌처리를 진행할때 필요한 벡터와 float 변수를 선언한다.
	//			FVector3 HitPoint, Normal;
	//			float Depth = 0.f;

	//			//SrcCollider를 기준으로 충돌 연산을 진행한다.
	//			//Collision이 true라면 충돌이 되었다는 뜻이된다.
	//			if (SrcCollider->Collision(HitPoint, Normal, Depth, DestCollider))
	//			{
	//				//hit랑 overlap두개의 조건을 세워준다.
	//				//블락 상태일떄
	//				if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Block)
	//				{
	//					//블락상태일때
	//					//hit처리를 하기전 먼저 두 충돌체의 겹칩을 없애기 위해 depth만큼 뒤로 밀어준다.

	//					//먼저 두 충돌체의 액터를 가져와준다.
	//					auto SrcActor = SrcCollider->GetOwner().lock();
	//					auto DestActor = DestCollider->GetOwner().lock();

	//					FVector3 SrcVelocity = SrcActor->GetVelocity();
	//					FVector3 DestVelocity = DestActor->GetVelocity();

	//					if (!SrcVelocity.IsZero() && !DestVelocity.IsZero())
	//					{
	//						//충돌로 발생한 normal과 뚫고간 크기인 depth를 이용해서 두 충돌체의 액터를
	//						//절반씩 밀어준다.
	//						SrcActor->AddWorldPos(-Normal * Depth * 0.5f);
	//						DestActor->AddWorldPos(Normal * Depth * 0.5f);

	//						SrcCollider->UpdateInfo();
	//						DestCollider->UpdateInfo();
	//					}
	//					// 하나의 액터만 움직일때는 그 액터만 움직이게끔 설정한다.
	//					else if (!SrcVelocity.IsZero())
	//					{
	//						SrcActor->AddWorldPos(-Normal * Depth);

	//						SrcCollider->UpdateInfo();
	//					}
	//					else if (!DestVelocity.IsZero())
	//					{
	//						DestActor->AddWorldPos(Normal * Depth);

	//						DestCollider->UpdateInfo();
	//					}

	//					SrcCollider->CallCollisionHit(HitPoint, Normal, DestCollider);
	//					DestCollider->CallCollisionHit(HitPoint, Normal, SrcCollider);

	//				}
	//				//블락이 아닌 오버랩 상태일때
	//				else
	//				{
	//					if (!SrcCollider->CheckCollisionObject(DestCollider))
	//					{
	//						LOG_DEBUG("BeginOverlap ", SrcCollider->GetName(), " ", DestCollider->GetName());

	//						SrcCollider->CallCollisionBeginOverlap(HitPoint, Normal, DestCollider);
	//						DestCollider->CallCollisionBeginOverlap(HitPoint, Normal, SrcCollider);
	//					}
	//				}
	//			}
	//			//충돌이 아니게되었을때 이 두 충돌체가 이전엔 충돌상태였으면
	//			//endoverlapfunc을 호출해준다.
	//			else if (SrcCollider->CheckCollisionObject(DestCollider))
	//			{
	//				if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Overlap)
	//				{
	//					SrcCollider->CallCollisionEndOverlap(DestCollider);
	//					DestCollider->CallCollisionEndOverlap(SrcCollider);
	//				}
	//			}

	//			++iter1;
	//		}

	//		++iter;
	//	}
	//}
}

void CWorldCollision::Render()
{
	mQuadTree->Render();
}

void CWorldCollision::ReturnNodePool()
{
	mQuadTree->ReturnNodePool();
}

void CWorldCollision::UpdateInfo()
{
	//충돌체의 정보를 갱신한다.

	auto iter = mColliderList.begin();
	auto iterEnd = mColliderList.end();

	for (; iter != iterEnd;)
	{
		if (iter->expired())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			continue;
		}

		auto Collider = iter->lock();

		if (!Collider->IsAlive())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			continue;
		}

		Collider->UpdateInfo();

		++iter;
	}

}
