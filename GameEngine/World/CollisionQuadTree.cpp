#include "CollisionQuadTree.h"

#include "World.h"
#include "CameraManager.h"
#include "../Device.h"

#include "../Asset/AssetManager.h"
#include "../Asset/MeshManager.h"
#include "../Asset/Mesh.h"
#include "../Asset/ShaderManager.h"
#include "../Asset/Shader.h"
#include "../Asset/CBufferCollider.h"

CCollisionQuadTree::CCollisionQuadTree()
{}

CCollisionQuadTree::~CCollisionQuadTree()
{}

void CCollisionQuadTree::AddCollisionNodeList(const std::weak_ptr<CCollisionQuadTreeNode>& Node)
{
	mCollisionNodeList.push_back(Node);
}

void CCollisionQuadTree::EraseCollisionNodeList(const std::weak_ptr<CCollisionQuadTreeNode>& Node)
{
	auto iter = mCollisionNodeList.begin();
	auto iterEnd = mCollisionNodeList.end();

	auto EraseNode = Node.lock();

	for (; iter != iterEnd; ++iter)
	{
		auto iterNode = iter->lock();

		if (iterNode == EraseNode)
		{
			mCollisionNodeList.erase(iter);
			break;
		}
	}

}

void CCollisionQuadTree::AddMouseCollisionNodeList(const std::weak_ptr<CCollisionQuadTreeNode>& Node)
{
	mMouseCollisionNodeList.push_back(Node);
}

void CCollisionQuadTree::EraseMouseCollisionNodeList(const std::weak_ptr<CCollisionQuadTreeNode>&Node)
{
	auto iter = mMouseCollisionNodeList.begin();
	auto iterEnd = mMouseCollisionNodeList.end();

	auto EraseNode = Node.lock();

	for (; iter != iterEnd; ++iter)
	{
		auto iterNode = iter->lock();

		if (iterNode == EraseNode)
		{
			mMouseCollisionNodeList.erase(iter);
			break;
		}
	}
}

bool CCollisionQuadTree::Init()
{

#ifdef _DEBUG

	auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (MeshMgr)
	{
		mMesh = MeshMgr->FindMesh("FrameRect");
	}

	if (ShaderMgr)
	{
		mShader = ShaderMgr->FindShader("FrameColor2D");
	}

	mCBufferCollider = std::make_shared<CCBufferCollider>();

	mCBufferCollider->Init();

	mCBufferCollider->SetColor(FVector4::Blue);

#endif
	//오브젝트 풀을 위해서 먼저 풀에 노드들을 만들어준다.
	int PoolSize = 0;
	int LevelNodeCount = 4;
	for (int i = 0; i < QUADTREE_DEPTHMAX; ++i)
	{
		PoolSize += LevelNodeCount;
		LevelNodeCount *= 4;
	}

	mNodePool.resize(PoolSize);
	for (int i = 0; i < PoolSize; ++i)
	{
		mNodePool[i] = std::make_shared<CCollisionQuadTreeNode>();
	}

	return true;
}

void CCollisionQuadTree::Update(float DeltaTime)
{
	//루트노드가 존재하면 센터를 매번 카메라의 중앙으로 만들어준다.
	if (mRoot)
	{
		auto World = mWorld.lock();

		if (World)
		{
			auto CameraMgr = World->GetCameraManager().lock();

			if (CameraMgr)
			{
				mRoot->SetCenter(CameraMgr->GetMainCameraPos());
			}
		}
	}
}

void CCollisionQuadTree::AddCollider(const std::weak_ptr<class CCollider>& Collider)
{
	//충돌체를 노드에게 넘겨준다.
	if (!mRoot)
	{
		mRoot = std::make_shared<CCollisionQuadTreeNode>();

		mRoot->SetWorld(mWorld);
		mRoot->SetTree(GetThisPtr());

		auto World = mWorld.lock();

		auto CameraMgr = World->GetCameraManager().lock();

		//루트노드의 위치는 카메라의 정중앙
		if (CameraMgr)
		{
			mRoot->SetCenter(CameraMgr->GetMainCameraPos());
		}

		//크기
		//쿼드트리 크기는 해상도의 1.5배로 적용한다.
		const FResolution& RS = CDevice::GetInst()->GetResolution();

		mRoot->SetSize(RS.Width * 1.5f, RS.Height * 1.5f);
	}

	mRoot->AddCollider(Collider, mNodePool);
}

void CCollisionQuadTree::Collision(float DeltaTime)
{
	auto iter = mCollisionNodeList.begin();
	auto iterEnd = mCollisionNodeList.end();

	for (; iter != iterEnd; ++iter)
	{
		auto Node = iter->lock();

		Node->Collision(DeltaTime);
	}

	//충돌연산이 완료되면 다음 프레임에 업데이트될 충돌체를 다시 검사하기 위해
	//충돌노드 리스트를 비워준다.
	mCollisionNodeList.clear();
}

void CCollisionQuadTree::CollisionMouse(std::weak_ptr<class CCollider>& Result, float DeltaTime, const FVector2& MousePos)
{
	//마우스 충돌, 마우스랑 충돌된 액터를 확인해서 바로 충돌을 반환해준다.
	auto iter = mMouseCollisionNodeList.begin();
	auto iterEnd = mMouseCollisionNodeList.end();

	for (; iter != iterEnd; ++iter)
	{
		auto Node = iter->lock();

		//현재 마우스 위치가 노드 안에 존재하는지 확인한다.
		if (Node->IsInCollider(MousePos))
		{
			//마우스 충돌을 확인한다.
			if (Node->CollisionMouse(Result, DeltaTime, MousePos))
			{
				mMouseCollisionNodeList.clear();
				return;
			}
		}
	}
}

void CCollisionQuadTree::Render()
{
#ifdef _DEBUG
	if (mRoot)
	{
		mCBufferCollider->UpdateBuffer();

		mRoot->Render(mMesh, mShader);
	}
#endif

}

void CCollisionQuadTree::ReturnNodePool()
{
	if (mRoot)
	{
		mRoot->ReturnNodePool(mNodePool);
	}
}
