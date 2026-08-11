#include "CollisionQuadTreeNode.h"

#include "CollisionQuadTree.h"
#include "Collider.h"
#include "Actor.h"

#include "../Asset/Mesh.h"
#include "../Asset/Shader.h"
#include "../Asset/CBufferTransform.h"

#include "World.h"
#include "CameraManager.h"

#include "../LogManager.h"

CCollisionQuadTreeNode::CCollisionQuadTreeNode()
{
#ifdef _DEBUG
	mCBufferTransform = std::make_shared<CCBufferTransform>();

	mCBufferTransform->Init();
#endif
}

CCollisionQuadTreeNode::~CCollisionQuadTreeNode()
{}

void CCollisionQuadTreeNode::AddCollider(std::weak_ptr<class CCollider> Collider, std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool)
{
	//먼저 충돌체가 현재 해당 노드에 존재하는지 판단한다.
	//영역에 이 충돌체가 있는지를 판단하는것
	if (!IsInCollider(Collider))
	{
		return;
	}

	//자식노드가 없을경우
	//현재 노드가 충돌을 계산하는 노드이기때문에
	//노드에 충돌체를 넣어준다.
	if (!mChild[0])
	{
		mColliderList.push_back(Collider);

		//충돌체가 1개일땐 충돌 연산이 필요없지만
		//2개일땐 충돌연산을 해야되므로 
		//쿼드트리의 충돌노드 리스트를 갱신해준다.
		auto Tree = mTree.lock();

		if (mColliderList.size() == 2)
		{
			Tree->AddCollisionNodeList(GetThisPtr());
		}

		//1개 이상일땐 마우스와 충돌이 가능하다.
		else if (mColliderList.size() == 1)
		{
			Tree->AddMouseCollisionNodeList(GetThisPtr());
		}

		//뎁스가 아직 최대가 아닌상태에서
		//분할이 필요한 충돌체 갯수가 되었다면 노드를 분할한다.
		//그 이후에 분할된 자식에게 충돌체를 이전한다.
		if (mColliderList.size() >= QUADTREE_DIVISION_COUNT && mDepth < QUADTREE_DEPTHMAX)
		{
			CreateChild(NodePool);

			auto iter = mColliderList.begin();
			auto iterEnd = mColliderList.end();

			//리스트에 있는 모든 충돌체를 자식노드에 나눠준다.
			for (; iter != iterEnd; ++iter)
			{
				for (int i = 0; i < 4; ++i)
				{
					if (mChild[i]->IsInCollider(*iter))
					{
						mChild[i]->AddCollider(*iter, NodePool);
					}
				}
			}

			//분배가 끝나면 목록을 비운다.
			mColliderList.clear();

			//더이상 충돌노드가 아니기 때문에 트리에 있는 충돌 노드 리스트에서 제거한다.
			Tree->EraseCollisionNodeList(GetThisPtr());

			Tree->EraseMouseCollisionNodeList(GetThisPtr());
		}

	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			mChild[i]->AddCollider(Collider, NodePool);
		}
	}

}

void CCollisionQuadTreeNode::Collision(float DeltaTime)
{
	if (!mColliderList.empty())
	{
		//충돌의 계산은 2^n만큼 계산한다.
		//오브젝트(액터)끼리 충돌체가 있을때 1:1로 계산되기 때문

		//충돌연산할때 자기자신과의 충돌연산은 의미가 없기 때문에
		//미리 한칸 당겨서 다른 반복자와 충돌 연산을 시도한다.
		auto iter = mColliderList.begin();
		auto iterEnd = mColliderList.end();
		--iterEnd;

		//다른 반복자의 끝도 도중에 컴포넌트가 제거될수 있기 때문에
		//반복문의 바깥에 선언해서 제거될때마다 갱신한다.
		auto iter1End = mColliderList.end();

		for (; iter != iterEnd;)
		{
			//반복자 무효화 방지
			if (mColliderList.size() < 2)
			{
				break;
			}

			if (iter->expired())
			{
				iter = mColliderList.erase(iter);
				iterEnd = mColliderList.end();
				iter1End = iterEnd;
				--iterEnd;
				continue;
			}

			auto SrcCollider = iter->lock();

			if (!SrcCollider->IsAlive())
			{
				iter = mColliderList.erase(iter);
				iterEnd = mColliderList.end();
				iter1End = iterEnd;
				--iterEnd;
				continue;
			}

			if (!SrcCollider->IsEnable())
			{
				++iter;
				continue;
			}

			//컴포넌트가 파괴되지 않고, 활성화 상태라면 충돌 연산을 시작한다.

			//먼저 프로파일을 확인해서 프로파일이 enable인지 확인한다.
			std::shared_ptr<FCollisionProfile> SrcProfile = SrcCollider->GetCollisionProfile();

			if (!SrcProfile || !SrcProfile->Enable)
			{
				++iter;
				continue;
			}

			//문제가 없으면 현재 충돌체의 다음 충돌체부터 충돌 연산을 시작한다.
			auto iter1 = iter;
			++iter1;

			for (; iter1 != iter1End;)
			{
				//반복자 무효화 방지
				if (mColliderList.size() < 2)
				{
					break;
				}

				if (iter1->expired())
				{
					iter1 = mColliderList.erase(iter1);
					iterEnd = mColliderList.end();
					iter1End = iterEnd;
					--iterEnd;
					continue;
				}

				auto DestCollider = iter1->lock();

				if (!DestCollider->IsAlive())
				{
					iter1 = mColliderList.erase(iter1);
					iterEnd = mColliderList.end();
					iter1End = iterEnd;
					--iterEnd;
					continue;
				}

				if (!DestCollider->IsEnable())
				{
					++iter1;
					continue;
				}

				std::shared_ptr<FCollisionProfile> DestProfile = DestCollider->GetCollisionProfile();

				//두 충돌체가 각각의 채널이 충돌할수 있는지를 확인한다.
				if (!DestProfile || !DestProfile->Enable)
				{
					++iter1;
					continue;
				}
				//프로파일 둘중 하나라도 ignore 채널이면 충돌처리를 건너뛴다.
				else if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Ignore ||
					DestProfile->Interaction[SrcProfile->Channel->Channel] == ECollisionInteraction::Ignore)
				{
					++iter1;
					continue;
				}
				//두 프로파일의 상호작용이 다르면 건너뛴다.
				else if (SrcProfile->Interaction[DestProfile->Channel->Channel] != DestProfile->Interaction[SrcProfile->Channel->Channel])
				{
					++iter1;
					continue;
				}

				//충돌처리를 진행할때 필요한 벡터와 float 변수를 선언한다.
				FVector3 HitPoint, Normal;
				float Depth = 0.f;

				//SrcCollider를 기준으로 충돌 연산을 진행한다.
				//Collision이 true라면 충돌이 되었다는 뜻이된다.
				if (SrcCollider->Collision(HitPoint, Normal, Depth, DestCollider))
				{
					//hit랑 overlap두개의 조건을 세워준다.
					//블락 상태일떄
					if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Block)
					{
						//블락상태일때
						//hit처리를 하기전 먼저 두 충돌체의 겹칩을 없애기 위해 depth만큼 뒤로 밀어준다.

						//먼저 두 충돌체의 액터를 가져와준다.
						auto SrcActor = SrcCollider->GetOwner().lock();
						auto DestActor = DestCollider->GetOwner().lock();

						FVector3 SrcVelocity = SrcActor->GetVelocity();
						FVector3 DestVelocity = DestActor->GetVelocity();

						if (!SrcVelocity.IsZero() && !DestVelocity.IsZero())
						{
							//충돌로 발생한 normal과 뚫고간 크기인 depth를 이용해서 두 충돌체의 액터를
							//절반씩 밀어준다.
							//오차로 인해 Hit이 연속적으로 발생하지 않게 0.01만큼 추가
							SrcActor->AddWorldPos(-Normal * (Depth + 0.01f) * 0.5f);
							DestActor->AddWorldPos(Normal * (Depth + 0.01f) * 0.5f);

							SrcCollider->UpdateInfo();
							DestCollider->UpdateInfo();
						}
						// 하나의 액터만 움직일때는 그 액터만 움직이게끔 설정한다.
						else if (!SrcVelocity.IsZero())
						{
							//오차로 인해 Hit이 연속적으로 발생하지 않게 0.01만큼 추가
							SrcActor->AddWorldPos(-Normal * (Depth + 0.01f));

							SrcCollider->UpdateInfo();
						}
						else if (!DestVelocity.IsZero())
						{
							//오차로 인해 Hit이 연속적으로 발생하지 않게 0.01만큼 추가
							DestActor->AddWorldPos(Normal * (Depth + 0.01f));

							DestCollider->UpdateInfo();
						}

						SrcCollider->CallCollisionHit(HitPoint, Normal, DestCollider);
						DestCollider->CallCollisionHit(HitPoint, -Normal, SrcCollider);

					}
					//블락이 아닌 오버랩 상태일때
					else
					{
						if (!SrcCollider->CheckCollisionObject(DestCollider))
						{
							LOG_DEBUG("BeginOverlap ", SrcCollider->GetName(), " ", DestCollider->GetName());

							SrcCollider->CallCollisionBeginOverlap(HitPoint, Normal, DestCollider);
							DestCollider->CallCollisionBeginOverlap(HitPoint, -Normal, SrcCollider);
						}
					}
				}
				//충돌이 아니게되었을때 이 두 충돌체가 이전엔 충돌상태였으면
				//endoverlapfunc을 호출해준다.
				else if (SrcCollider->CheckCollisionObject(DestCollider))
				{
					if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Overlap)
					{
						SrcCollider->CallCollisionEndOverlap(DestCollider);
						DestCollider->CallCollisionEndOverlap(SrcCollider);
					}
				}

				++iter1;
			}

			//충돌체가 2개일때 반복자 무효 방어 코드
			if (iter == iterEnd)
			{
				break;
			}

			++iter;
		}
	}
}

bool CCollisionQuadTreeNode::CollisionMouse(std::weak_ptr<class CCollider>& Result, float DeltaTime, const FVector2& MousePos)
{
	//마우스 충돌 연산전 리스트들을 정렬해준다.
	if (mColliderList.size() >= 2)
	{
		mColliderList.sort(CCollisionQuadTreeNode::SortCollisionMouse);
	}

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

		auto SrcCollider = iter->lock();

		if (!SrcCollider->IsAlive())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			continue;
		}

		if (!SrcCollider->IsEnable())
		{
			++iter;
			continue;
		}

		//충돌이 성공했으므로 Result로 액터를 반환한다.
		if (SrcCollider->CollisionMouse(MousePos))
		{
			auto ResultActor = Result.lock();

			//현재 충돌한 액터랑 지금 충돌된 액터가 다르면
			//Result를 갱신한다.
			if (ResultActor != SrcCollider)
			{
				Result = *iter;
				LOG_DEBUG("Mouse Hit");
			}

			return true;
		}


		++iter;
	}

	return false;
}

void CCollisionQuadTreeNode::Render(const std::weak_ptr<class CMesh> Mesh, const std::weak_ptr<class CShader> Shader)
{
#ifdef _DEBUG

	if (Mesh.expired() || Shader.expired())
	{
		return;
	}

	FMatrix ViewMat;
	FMatrix ProjMat;

	auto World = mWorld.lock();

	if (World)
	{
		auto CameraMgr = World->GetCameraManager().lock();

		if (CameraMgr)
		{
			ViewMat = CameraMgr->GetViewMat();
			ProjMat = CameraMgr->GetProjMat();
		}
	}

	auto _Mesh = Mesh.lock();
	auto _Shader = Shader.lock();

	FMatrix ScaleMat, TranslationMat, WorldMat;

	ScaleMat.Scaling(mSize);
	TranslationMat.Translation(mCenter);

	WorldMat = ScaleMat * TranslationMat;

	mCBufferTransform->SetWorldMatrix(WorldMat);
	mCBufferTransform->SetViewMatrix(ViewMat);
	mCBufferTransform->SetProjMatrix(ProjMat);

	mCBufferTransform->UpdateBuffer();

	_Shader->SetShader();
	_Mesh->Render();

	if (mChild[0])
	{
		for (int i = 0; i < 4; ++i)
		{
			mChild[i]->Render(Mesh, Shader);
		}
	}

#endif
}

void CCollisionQuadTreeNode::ReturnNodePool(std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool)
{
	//오브젝트 풀 반환
	//충돌연산 한번 실행한뒤
	//리스트를 비우면서 자식 노드를 풀에 반환한다.

	mColliderList.clear();

	if (mChild[0])
	{
		for (int i = 0; i < 4; ++i)
		{
			NodePool.push_back(mChild[i]);
			//반환한 자식노드도 풀을 반환하게 한다.
			mChild[i]->ReturnNodePool(NodePool);

			//반환한 자식의 링크를 끊어 반환을 완료한다.
			mChild[i].reset();
		}
	}

}

bool CCollisionQuadTreeNode::IsInCollider(const std::weak_ptr<class CCollider>& Collider)
{
	//충돌체가 현재 해당 노드의 영역에 있는지 판단한다.
	//AABB를 사용해서 노드에 포함되어 있는지 판단할수 있다.

	auto CurCollider = Collider.lock();

	if (CurCollider)
	{
		FVector3 ColliderMin = CurCollider->GetMin();
		FVector3 ColliderMax = CurCollider->GetMax();

		FVector3 NodeMin = mCenter - mSize * 0.5f;
		FVector3 NodeMax = NodeMin + mSize;

		//AABB충돌을 확인하다.
		if (ColliderMin.x > NodeMax.x)
		{
			return false;
		}

		if (ColliderMin.y > NodeMax.y)
		{
			return false;
		}

		if (ColliderMax.x < NodeMin.x)
		{
			return false;
		}

		if (ColliderMax.y < NodeMin.y)
		{
			return false;
		}
	}

	return true;
}

bool CCollisionQuadTreeNode::IsInCollider(const FVector2& MousePos)
{
	FVector3 NodeMin = mCenter - mSize * 0.5f;
	FVector3 NodeMax = NodeMin + mSize;

	//AABB충돌을 확인하다.
	if (MousePos.x > NodeMax.x)
	{
		return false;
	}

	if (MousePos.y > NodeMax.y)
	{
		return false;
	}

	if (MousePos.x < NodeMin.x)
	{
		return false;
	}

	if (MousePos.y < NodeMin.y)
	{
		return false;
	}

	return true;
}

void CCollisionQuadTreeNode::CreateChild(std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool)
{
	//노드 풀이 비어있을때 오브젝트풀에 새로운 노드 4개를 생성한다.

	if (NodePool.empty())
	{
		NodePool.resize(4);

		for (int i = 0; i < 4; ++i)
		{
			NodePool[i].reset(new CCollisionQuadTreeNode);
		}
	}

	//현재 풀에 있는 오브젝트를 뒤에서부터 가져와서
	//풀에서 제거하면서 child를 갱신한다.
	for (int i = 0; i < 4; ++i)
	{
		mChild[i] = NodePool.back();

		NodePool.pop_back();

		mChild[i]->SetWorld(mWorld);
		mChild[i]->SetTree(mTree);
		mChild[i]->mParent = GetThisPtr();
		mChild[i]->mDepth = mDepth + 1;
		mChild[i]->mSize = mSize * 0.5f;

		//노드를 분리하고 각 자식노드의 Center를 구해준다.
		//노드는 가운데를 기준으로 좌상, 우상, 좌하, 우하로 나눈다.

		switch (i)
		{
		case 0:
			mChild[i]->mCenter = FVector3(mCenter.x - mSize.x * 0.25f, mCenter.y + mSize.y * 0.25f, 0.f);
			break;
		case 1:
			mChild[i]->mCenter = FVector3(mCenter.x + mSize.x * 0.25f, mCenter.y + mSize.y * 0.25f, 0.f);
			break;
		case 2:
			mChild[i]->mCenter = FVector3(mCenter.x - mSize.x * 0.25f, mCenter.y - mSize.y * 0.25f, 0.f);
			break;
		case 3:
			mChild[i]->mCenter = FVector3(mCenter.x + mSize.x * 0.25f, mCenter.y - mSize.y * 0.25f, 0.f);
			break;
		}

	}
}

//Y정렬
bool CCollisionQuadTreeNode::SortCollisionMouse(const std::weak_ptr<class CCollider> Src, const std::weak_ptr<class CCollider> Dest)
{
	auto _Src = Src.lock();
	auto _Dest = Dest.lock();

	float SrcY = _Src->GetWorldPos().y;
	float DestY = _Dest->GetWorldPos().y;

	return SrcY < DestY;
}
