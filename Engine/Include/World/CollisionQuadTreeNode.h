#pragma once

#include "../EngineInfo.h"

//쿼드트리 노드
//4개의 분할된 공간중 하나의 노드
//노드는 새로운 4개의 공간으로 분할할수 있다.

//너무 잘게 노드를 쪼개게되면 쪼개는 의미가 사라지게 된다.
//따라서 노드는 적절한 크기까지만 쪼개면 된다.

#define QUADTREE_DEPTHMAX 4
#define QUADTREE_DIVISION_COUNT 5

class CCollisionQuadTreeNode : public std::enable_shared_from_this<CCollisionQuadTreeNode>
{
public:
	CCollisionQuadTreeNode();
	~CCollisionQuadTreeNode();

private:
	std::weak_ptr<class CWorld> mWorld;
	std::weak_ptr<CCollisionQuadTreeNode> mParent;
	std::weak_ptr<class CCollisionQuadTree> mTree;

	std::shared_ptr<CCollisionQuadTreeNode> mChild[4];

	//현재 노드의 중심 위치
	FVector3 mCenter;
	//노드의 크기
	FVector3 mSize;

	//현재 노드가 얼마나 쪼개졌는지 확인하는 깊이 탐색 용도
	int mDepth = 0;

	std::list<std::weak_ptr<class CCollider>> mColliderList;

#ifdef _DEBUG

	std::shared_ptr<class CCBufferTransform> mCBufferTransform;

#endif
public:
	void SetWorld(const std::weak_ptr<class CWorld> World)
	{
		mWorld = World;
	}

	void SetTree(const std::weak_ptr<class CCollisionQuadTree>& Tree)
	{
		mTree = Tree;
	}

	void SetCenter(const FVector3& Center)
	{
		mCenter = Center;
	}

	void SetSize(const FVector3& Size)
	{
		mSize = Size;
	}

	void SetSize(float x, float y)
	{
		mSize = FVector3(x, y, 0.f);
	}
public:
	//오브젝트풀을 이용해서 노드를 가져올 함수
	void AddCollider(std::weak_ptr<class CCollider> Collider, std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool);
	//충돌 체크해줄 함수
	void Collision(float DeltaTime);
	bool CollisionMouse(std::weak_ptr<class CCollider>& Result, float DeltaTime, const FVector2& MousePos);

	void Render(const std::weak_ptr<class CMesh> Mesh, const std::weak_ptr<class CShader> Shader);
	void ReturnNodePool(std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool);

public:
	std::shared_ptr<CCollisionQuadTreeNode> GetThisPtr()
	{
		return shared_from_this();
	}

public:
	bool IsInCollider(const std::weak_ptr<class CCollider>& Collider);
	bool IsInCollider(const FVector2& MousePos);

private:
	void CreateChild(std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool);
	static bool SortCollisionMouse(const std::weak_ptr<class CCollider> Src, const std::weak_ptr<class CCollider> Dest);

};

