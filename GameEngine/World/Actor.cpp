#include "Actor.h"

#include "SceneComponent.h"
#include "ActorComponent.h"

#include "World.h"

#include "../LogManager.h"

#include <fstream>

CActor::CActor()
{}

CActor::CActor(const CActor& src)
{
	mWorld = src.mWorld;
	mName = src.mName;
	mRenderEnable = src.mRenderEnable;

	//씬컴포넌트 리스트를 복사한다.
	auto iter = src.mSceneCompList.begin();
	auto iterEnd = src.mSceneCompList.end();

	for (; iter != iterEnd; ++iter)
	{
		//shared_ptr로 원시포인터를 감싸줘
		//새로운 shared_ptr로 만들어준다.
		auto NewComp = std::shared_ptr<CSceneComponent>((*iter)->Clone());
		NewComp->SetOwner(GetThisPtr<CActor>());

		if (mSceneCompList.empty())
		{
			mRoot = NewComp;
		}

		if (!(*iter)->GetParent().expired())
		{
			auto Parent = (*iter)->GetParent().lock();
			auto iter1 = mSceneCompList.begin();
			auto iter1End = mSceneCompList.end();

			//이제것 들어간 복사된 컴포넌트 리스트를 순회해
			//현재 부모 컴포넌트의 이름과 동일한 컴포넌트를 찾아서
			//AddChild함수를 호출해준다.
			for (; iter1 != iter1End; ++iter1)
			{
				if (Parent->GetName() == (*iter1)->GetName())
				{
					(*iter1)->AddChild(NewComp);
					break;
				}
			}
		}

		//완료되면 내 컴포넌트리스트에 등록한다.
		mSceneCompList.push_back(NewComp);

	}


	auto iter2 = src.mActorCompList.begin();
	auto iter2End = src.mActorCompList.end();

	for (; iter2 != iter2End; ++iter2)
	{
		auto NewComp = std::shared_ptr<CActorComponent>((*iter2)->Clone());
		NewComp->SetOwner(GetThisPtr<CActor>());

		mActorCompList.push_back(NewComp);
	}
}

CActor::CActor(CActor&& src) noexcept
{
	mWorld = src.mWorld;
	mName = src.mName;

	//리스트들을 넘겨주면 이동완료다
	mRoot = src.mRoot;
	mSceneCompList = src.mSceneCompList;
	mActorCompList = src.mActorCompList;
}

CActor::~CActor()
{}

void CActor::SetWorld(const std::weak_ptr<class CWorld>& World)
{
	mWorld = World;
}

void CActor::AttachActorToActor(const std::weak_ptr<CActor> OtherActor)
{
	auto Root = mRoot.lock();

	//부착할 액터의 컴포넌트리스트를
	//다른 액터의 컴포넌트리스트에 넣으면 문제가 생긴다.
	if (Root)
	{
		Root->AttachToActor(OtherActor);
	}
}

void CActor::DetachActorFromActor()
{
	auto Root = mRoot.lock();

	if (Root)
	{
		Root->DetachFromParent();
	}
}

bool CActor::Init()
{
	return true;
}

void CActor::Update(float DeltaTime)
{
	//auto Root = mRoot.lock();

	//if (Root)
	//{
	//	Root->Update(DeltaTime);
	//}

	//액터가 컴포넌트를 관리하는 주체역할이 될수있게
	//모든 컴포넌트 리스트를 업데이트 해준다.

	auto SceneIter = mSceneCompList.begin();
	auto SceneIterEnd = mSceneCompList.end();

	for (; SceneIter != SceneIterEnd;)
	{
		//Update이전에 먼저 삭제할 오브젝트는 제거해준다.
		if (!(*SceneIter)->IsAlive())
		{
			SceneIter = mSceneCompList.erase(SceneIter);
			SceneIterEnd = mSceneCompList.end();
			continue;
		}
		else if (!(*SceneIter)->IsEnable())
		{
			++SceneIter;
			continue;
		}

		(*SceneIter)->Update(DeltaTime);
		++SceneIter;
	}

	//업데이트 도중에 컴포넌트가 새로 붙을 수 있으므로 반복자 대신 인덱스로 돈다.
	//(예: 액션 상태 컴포넌트가 없는 높이 컴포넌트를 스스로 만들어 붙인다)
	//push_back 한 번이면 반복자는 전부 무효가 되어 그대로 터진다.
	//개수를 미리 재두었으므로 이번 프레임에 붙은 것은 다음 프레임부터 돈다.
	size_t ActorCompCount = mActorCompList.size();

	for (size_t i = 0; i < ActorCompCount;)
	{
		if (!mActorCompList[i]->IsAlive())
		{
			mActorCompList.erase(mActorCompList.begin() + i);
			--ActorCompCount;
			continue;
		}

		if (!mActorCompList[i]->IsEnable())
		{
			++i;
			continue;
		}

		mActorCompList[i]->Update(DeltaTime);

		++i;
	}
}

void CActor::PostUpdate(float DeltaTime)
{
	/*auto Root = mRoot.lock();

	if (Root)
	{
		Root->PostUpdate(DeltaTime);
	}*/

	//Collision();


	auto SceneIter = mSceneCompList.begin();
	auto SceneIterEnd = mSceneCompList.end();

	for (; SceneIter != SceneIterEnd;)
	{
		if (!(*SceneIter)->IsAlive())
		{
			SceneIter = mSceneCompList.erase(SceneIter);
			SceneIterEnd = mSceneCompList.end();
			continue;
		}
		else if (!(*SceneIter)->IsEnable())
		{
			++SceneIter;
			continue;
		}

		(*SceneIter)->PostUpdate(DeltaTime);
		++SceneIter;
	}

	auto ActorIter = mActorCompList.begin();
	auto ActorIterEnd = mActorCompList.end();

	for (; ActorIter != ActorIterEnd;)
	{
		if (!(*ActorIter)->IsAlive())
		{
			ActorIter = mActorCompList.erase(ActorIter);
			ActorIterEnd = mActorCompList.end();
			continue;
		}
		else if (!(*ActorIter)->IsEnable())
		{
			++ActorIter;
			continue;
		}

		(*ActorIter)->PostUpdate(DeltaTime);

		++ActorIter;
	}

}

void CActor::Render()
{
	/*auto Root = mRoot.lock();

	if (Root)
	{
		Root->Render();
	}*/

	/*auto SceneIter = mSceneCompList.begin();
	auto SceneIterEnd = mSceneCompList.end();

	for (; SceneIter != SceneIterEnd;)
	{
		if (!(*SceneIter)->IsAlive())
		{
			SceneIter = mSceneCompList.erase(SceneIter);
			SceneIterEnd = mSceneCompList.end();
			continue;
		}
		else if (!(*SceneIter)->IsEnable())
		{
			++SceneIter;
			continue;
		}

		(*SceneIter)->Render();
		++SceneIter;
	}*/

}

void CActor::PostRender()
{
	/*auto Root = mRoot.lock();

	if (Root)
	{
		Root->Render();
	}*/

	//auto SceneIter = mSceneCompList.begin();
	//auto SceneIterEnd = mSceneCompList.end();

	//for (; SceneIter != SceneIterEnd;)
	//{
	//	if (!(*SceneIter)->IsAlive())
	//	{
	//		SceneIter = mSceneCompList.erase(SceneIter);
	//		SceneIterEnd = mSceneCompList.end();
	//		continue;
	//	}
	//	else if (!(*SceneIter)->IsEnable())
	//	{
	//		++SceneIter;
	//		continue;
	//	}

	//	(*SceneIter)->PostRender();
	//	++SceneIter;
	//}
}

void CActor::Destroy()
{
	mAlive = false;

	/*auto Root = mRoot.lock();

	if (Root)
	{
		Root->Destroy();
	}*/

	size_t Size = mSceneCompList.size();

	for (int i = 0; i < Size; ++i)
	{
		mSceneCompList[i]->Destroy();
	}

	Size = mActorCompList.size();

	for (int i = 0; i < Size; ++i)
	{
		mActorCompList[i]->Destroy();
	}

}

float CActor::TakeDamage(float fDamage, const std::string& Instigator, const std::weak_ptr<CActor>& CauseActor)
{
	return fDamage;
}

void CActor::UpdateTransform()
{
	/*auto Root = mRoot.lock();

	if (Root)
	{
		Root->UpdateTransform();
	}*/

	size_t Size = mSceneCompList.size();

	for (int i = 0; i < Size; ++i)
	{
		mSceneCompList[i]->UpdateTransform();
	}
}

void CActor::SetRelativeScale(const FVector3& Scale)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeScale(Scale);
	}
}

void CActor::SetRelativeScale(const FVector2& Scale)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeScale(Scale);
	}
}

void CActor::SetRelativeScale(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeScale(x, y, z);
	}
}

void CActor::SetRelativeScale(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeScale(x, y);
	}
}

void CActor::AddRelativeScale(const FVector3& Scale)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeScale(Scale);
	}
}

void CActor::AddRelativeScale(const FVector2& Scale)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeScale(Scale);
	}
}

void CActor::AddRelativeScale(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeScale(x, y, z);
	}
}

void CActor::AddRelativeScale(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeScale(x, y);
	}
}

void CActor::SetRelativeRotation(const FVector3& Rot)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeRotation(Rot);
	}
}

void CActor::SetRelativeRotation(const FVector2& Rot)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeRotation(Rot);
	}
}

void CActor::SetRelativeRotation(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeRotation(x, y, z);
	}
}

void CActor::SetRelativeRotation(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeRotation(x, y);
	}
}

void CActor::SetRelativeRotationX(float x)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeRotationX(x);
	}
}

void CActor::SetRelativeRotationY(float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeRotationY(y);
	}
}

void CActor::SetRelativeRotationZ(float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativeRotationZ(z);
	}
}

void CActor::AddRelativeRotation(const FVector3& Rot)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeRotation(Rot);
	}
}

void CActor::AddRelativeRotation(const FVector2& Rot)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeRotation(Rot);
	}
}

void CActor::AddRelativeRotation(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeRotation(x, y, z);
	}
}

void CActor::AddRelativeRotation(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeRotation(x, y);
	}
}

void CActor::AddRelativeRotationX(float x)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeRotationX(x);
	}
}

void CActor::AddRelativeRotationY(float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeRotationY(y);
	}
}

void CActor::AddRelativeRotationZ(float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativeRotationZ(z);
	}
}

void CActor::SetRelativePos(const FVector3& Pos)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativePos(Pos);
	}
}

void CActor::SetRelativePos(const FVector2& Pos)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativePos(Pos);
	}
}

void CActor::SetRelativePos(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativePos(x, y, z);
	}
}

void CActor::SetRelativePos(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetRelativePos(x, y);
	}
}

void CActor::AddRelativePos(const FVector3& Pos)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativePos(Pos);
	}
}

void CActor::AddRelativePos(const FVector2& Pos)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativePos(Pos);
	}
}

void CActor::AddRelativePos(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativePos(x, y, z);
	}
}

void CActor::AddRelativePos(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddRelativePos(x, y);
	}
}

void CActor::SetWorldScale(const FVector3& Scale)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldScale(Scale);
	}
}

void CActor::SetWorldScale(const FVector2& Scale)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldScale(Scale);
	}
}

void CActor::SetWorldScale(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldScale(x, y, z);
	}
}

void CActor::SetWorldScale(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldScale(x, y);
	}
}

void CActor::AddWorldScale(const FVector3& Scale)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldScale(Scale);
	}
}

void CActor::AddWorldScale(const FVector2& Scale)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldScale(Scale);
	}
}

void CActor::AddWorldScale(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldScale(x, y, z);
	}
}

void CActor::AddWorldScale(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldScale(x, y);
	}
}

void CActor::SetWorldRotation(const FVector2& Rot)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldRotation(Rot);
	}
}

void CActor::SetWorldRotation(const FVector3& Rot)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldRotation(Rot);
	}
}

void CActor::SetWorldRotation(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldRotation(x, y, z);
	}
}

void CActor::SetWorldRotation(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldRotation(x, y);
	}
}

void CActor::SetWorldRotationX(float x)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldRotationX(x);
	}
}

void CActor::SetWorldRotationY(float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldRotationY(y);
	}
}

void CActor::SetWorldRotationZ(float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldRotationZ(z);
	}
}

void CActor::AddWorldRotation(const FVector2& Rot)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldRotation(Rot);
	}
}

void CActor::AddWorldRotation(const FVector3& Rot)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldRotation(Rot);
	}
}

void CActor::AddWorldRotation(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldRotation(x, y, z);
	}
}

void CActor::AddWorldRotation(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldRotation(x, y);
	}
}

void CActor::AddWorldRotationX(float x)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldRotationX(x);
	}
}

void CActor::AddWorldRotationY(float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldRotationY(y);
	}
}

void CActor::AddWorldRotationZ(float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldRotationZ(z);
	}
}

void CActor::SetWorldPos(const FVector3& Pos)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldPos(Pos);
	}
}

void CActor::SetWorldPos(const FVector2& Pos)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldPos(Pos);
	}
}

void CActor::SetWorldPos(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldPos(x, y, z);
	}
}

void CActor::SetWorldPos(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->SetWorldPos(x, y);
	}
}

void CActor::AddWorldPos(const FVector3& Pos)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldPos(Pos);
	}
}

void CActor::AddWorldPos(const FVector2& Pos)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldPos(Pos);
	}
}

void CActor::AddWorldPos(float x, float y, float z)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldPos(x, y, z);
	}
}

void CActor::AddWorldPos(float x, float y)
{
	auto Root = mRoot.lock();
	if (Root)
	{
		Root->AddWorldPos(x, y);
	}
}

//void CActor::Collision()
//{
//	auto World = mWorld.lock();
//
//	if (World)
//	{
//		auto ActorList = World->GetActorList();
//
//		auto iter = ActorList.begin();
//		auto iterEnd = ActorList.end();
//
//		//제곱수로 이뤄지는 충돌체크는
//		//필요하다면 건너뛸수 있게 만들어야된다.
//		//충돌에서 들어가는 연산량이 꽤나 크게 잡히기 때문에
//		//성능 저하가 발생한다.
//
//
//		for (; iter != iterEnd; ++iter)
//		{
//			if (iter->second == GetThisPtr())
//			{
//				continue;
//			}
//
//			if (mActorTag != iter->second->GetActorTag())
//			{
//				FVector3 OtherWorldPos = iter->second->GetWorldPos();
//				float TotalRaiuds = mRadius + iter->second->mRadius;
//				float Distance = OtherWorldPos.Distance(GetWorldPos());
//
//				//LOG_DEBUG("Distance : ", Distance);
//
//				if (Distance < TotalRaiuds)
//				{
//					LOG_DEBUG("Collision : ", iter->second->GetName());
//					OnCollision(iter->second);
//				}
//			}
//
//		}
//
//	}
//}

void CActor::OnCollision(const std::weak_ptr<CActor>& HitActor)
{}

std::string CActor::GetParentName(const std::shared_ptr<CSceneComponent>& Comp) const
{
	auto Parent = Comp->GetParent().lock();
	return Parent ? Parent->GetName() : "";
}

//---- 컴포넌트 제거 ----

bool CActor::RemoveSceneComponent(std::shared_ptr<CSceneComponent> Comp)
{
	if (!Comp)
	{
		return false;
	}

	//내 액터가 실제로 소유한 컴포넌트인지 확인한다.
	size_t Size = mSceneCompList.size();
	size_t Index = Size;

	for (size_t i = 0; i < Size; ++i)
	{
		if (mSceneCompList[i] == Comp)
		{
			Index = i;
			break;
		}
	}

	if (Index == Size)
	{
		LOG_WARNING("[Actor] %s: RemoveSceneComponent 실패 - 소유하지 않은 컴포넌트 %s",
			mName.c_str(), Comp->GetName().c_str());
		return false;
	}

	//DetachFromParent가 자식 목록을 변경하므로 미리 복사해둔다.
	std::vector<std::shared_ptr<CSceneComponent>> Children;

	for (const auto& ChildWeak : Comp->GetChildList())
	{
		auto Child = ChildWeak.lock();

		if (Child)
		{
			Children.push_back(Child);
		}
	}

	auto Root = mRoot.lock();
	bool IsRoot = (Root == Comp);

	//DetachFromParent가 부모 링크를 끊으므로 미리 확보해둔다.
	auto OldParent = Comp->GetParent().lock();

	//대상을 계층에서 떼어낸다.
	Comp->DetachFromParent();

	if (IsRoot)
	{
		//루트가 제거되면 첫 번째 자식을 새 루트로 승격시킨다.
		if (Children.empty())
		{
			mRoot.reset();
		}
		else
		{
			auto NewRoot = Children[0];
			NewRoot->DetachFromParent();
			mRoot = NewRoot;

			//나머지 자식은 새 루트 밑으로 옮긴다.
			for (size_t i = 1; i < Children.size(); ++i)
			{
				Children[i]->DetachFromParent();
				NewRoot->AddChild(Children[i]);
			}
		}
	}
	else
	{
		//자식들을 제거 대상의 부모(없으면 루트)로 재부착한다.
		auto NewParent = OldParent;

		if (!NewParent)
		{
			NewParent = mRoot.lock();
		}

		for (auto& Child : Children)
		{
			Child->DetachFromParent();

			if (NewParent)
			{
				NewParent->AddChild(Child);
			}
		}
	}

	//Destroy를 먼저 호출해 콜라이더 충돌 종료 처리 등을 진행시킨다.
	Comp->Destroy();

	//shared_ptr을 리스트에서 제거하면 실제로 소멸된다.
	//렌더매니저/월드콜리전은 weak_ptr로만 들고 있어 만료된 항목을 스스로 정리한다.
	mSceneCompList.erase(mSceneCompList.begin() + Index);

	UpdateTransform();

	LOG_DEBUG("[Actor] %s: SceneComponent 제거 - %s", mName.c_str(), Comp->GetName().c_str());

	return true;
}

bool CActor::RemoveActorComponent(std::shared_ptr<CActorComponent> Comp)
{
	if (!Comp)
	{
		return false;
	}

	size_t Size = mActorCompList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		if (mActorCompList[i] != Comp)
		{
			continue;
		}

		Comp->Destroy();
		mActorCompList.erase(mActorCompList.begin() + i);

		LOG_DEBUG("[Actor] %s: ActorComponent 제거 - %s", mName.c_str(), Comp->GetName().c_str());

		return true;
	}

	LOG_WARNING("[Actor] %s: RemoveActorComponent 실패 - 소유하지 않은 컴포넌트 %s",
		mName.c_str(), Comp->GetName().c_str());

	return false;
}

bool CActor::RemoveComponent(std::shared_ptr<CComponent> Comp)
{
	if (!Comp)
	{
		return false;
	}

	if (Comp->GetCompType() == EComponentType::Scene)
	{
		return RemoveSceneComponent(std::dynamic_pointer_cast<CSceneComponent>(Comp));
	}

	return RemoveActorComponent(std::dynamic_pointer_cast<CActorComponent>(Comp));
}

bool CActor::RemoveComponent(const std::string& Name)
{
	size_t SceneSize = mSceneCompList.size();

	for (size_t i = 0; i < SceneSize; ++i)
	{
		if (mSceneCompList[i]->GetName() == Name)
		{
			return RemoveSceneComponent(mSceneCompList[i]);
		}
	}

	size_t ActorSize = mActorCompList.size();

	for (size_t i = 0; i < ActorSize; ++i)
	{
		if (mActorCompList[i]->GetName() == Name)
		{
			return RemoveActorComponent(mActorCompList[i]);
		}
	}

	LOG_WARNING("[Actor] %s: RemoveComponent 실패 - %s 를 찾지 못함", mName.c_str(), Name.c_str());

	return false;
}

void CActor::SaveScene(std::ofstream& File, int ActorIdx) const
{
	FVector3 Pos   = GetWorldPos();
	FVector3 Scale = GetWorldScale();
	FVector3 Rot   = GetWorldRot();

	bool IsGeneric = (GetTypeName() == "CActor");

	File << "[Actor:" << ActorIdx << "]\n";
	File << "TypeName=" << GetTypeName() << "\n";
	File << "Name=" << mName << "\n";
	File << "Tag=" << mActorTag << "\n";
	File << "WorldPos=" << Pos.x << " " << Pos.y << " " << Pos.z << "\n";
	File << "WorldScale=" << Scale.x << " " << Scale.y << " " << Scale.z << "\n";
	File << "WorldRot=" << Rot.x << " " << Rot.y << " " << Rot.z << "\n";
	File << "SceneCompCount=" << (IsGeneric ? mSceneCompList.size() : 0) << "\n";
	File << "ActorCompCount=" << (IsGeneric ? mActorCompList.size() : 0) << "\n";
	File << "\n";

	if (!IsGeneric) return;

	for (size_t i = 0; i < mSceneCompList.size(); ++i)
	{
		const auto& Comp = mSceneCompList[i];
		File << "[SceneComp:" << ActorIdx << ":" << i << "]\n";
		Comp->Save(File);
		File << "\n";
	}

	for (size_t i = 0; i < mActorCompList.size(); ++i)
	{
		const auto& Comp = mActorCompList[i];
		File << "[ActorComp:" << ActorIdx << ":" << i << "]\n";
		Comp->Save(File);
		File << "\n";
	}
}

std::weak_ptr<CSceneComponent> CActor::InsertBareRoot(const std::string& Name)
{
	auto OldRoot = mRoot.lock();

	if (!OldRoot)
	{
		return std::weak_ptr<CSceneComponent>();
	}

	//이미 빈 루트를 쓰고 있으면 그대로 둔다.
	if (OldRoot->GetTypeName() == "CSceneComponent")
	{
		return OldRoot;
	}

	auto NewRoot = std::make_shared<CSceneComponent>();

	NewRoot->SetName(Name);
	NewRoot->SetWorld(mWorld);
	NewRoot->SetOwner(GetThisPtr<CActor>());
	NewRoot->Init();

	//트랜스폼은 새 루트가 통째로 가져간다.
	//옛 루트를 원점/기본값으로 되돌려 자식으로 달면 최종 위치가 그대로 유지된다.
	//(월드 트랜스폼이 부모와 곱해져 내려가므로 결과가 달라지지 않는다)
	NewRoot->SetRelativePos(OldRoot->GetRelativePos());
	NewRoot->SetRelativeScale(OldRoot->GetRelativeScale());
	NewRoot->SetRelativeRotation(OldRoot->GetRelativeRot());

	OldRoot->SetRelativePos(0.f, 0.f, 0.f);
	OldRoot->SetRelativeScale(1.f, 1.f, 1.f);
	OldRoot->SetRelativeRotation(0.f, 0.f, 0.f);

	mRoot = NewRoot;
	NewRoot->AddChild(OldRoot);

	//저장할 때 루트가 먼저 나와야 불러올 때도 루트로 잡힌다.
	mSceneCompList.insert(mSceneCompList.begin(), NewRoot);

	return NewRoot;
}

void CActor::LoadSceneComp(std::shared_ptr<CSceneComponent> Comp, const std::string& Parent)
{
	Comp->SetWorld(mWorld);
	Comp->SetOwner(GetThisPtr<CActor>());
	Comp->Init();

	if (mSceneCompList.empty())
	{
		mRoot = Comp;
	}
	else
	{
		std::shared_ptr<CSceneComponent> ParentComp;
		if (Parent.empty() || Parent == "Root")
		{
			ParentComp = mRoot.lock();
		}
		else
		{
			for (const auto& SC : mSceneCompList)
			{
				if (SC->GetName() == Parent) { ParentComp = SC; break; }
			}
			if (!ParentComp) ParentComp = mRoot.lock();
		}
		if (ParentComp) ParentComp->AddChild(Comp);
	}

	mSceneCompList.push_back(Comp);
}

void CActor::LoadActorComp(std::shared_ptr<CActorComponent> Comp)
{
	Comp->SetWorld(mWorld);
	Comp->SetOwner(GetThisPtr<CActor>());
	Comp->Init();
	mActorCompList.push_back(Comp);
}
