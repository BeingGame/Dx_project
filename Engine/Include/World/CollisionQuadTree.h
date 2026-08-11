#pragma once

#include "CollisionQuadTreeNode.h"

class CCollisionQuadTree : public std::enable_shared_from_this<CCollisionQuadTree>
{
public:
	CCollisionQuadTree();
	~CCollisionQuadTree();

private:
	std::weak_ptr<class CWorld> mWorld;
	std::shared_ptr<CCollisionQuadTreeNode> mRoot;
	std::vector<std::shared_ptr<CCollisionQuadTreeNode>> mNodePool;
	std::vector<std::weak_ptr<CCollisionQuadTreeNode>> mCollisionNodeList;
	std::vector<std::weak_ptr<CCollisionQuadTreeNode>> mMouseCollisionNodeList;

#ifdef _DEBUG

	//디버그용 렌더링에 사용할 메쉬랑 셰이더
	std::weak_ptr<class CMesh> mMesh;
	std::weak_ptr<class CShader> mShader;

	std::shared_ptr<class CCBufferCollider> mCBufferCollider;

#endif

public:
	void SetWorld(const std::weak_ptr<class CWorld>& World)
	{
		mWorld = World;
	}

	void AddCollisionNodeList(const std::weak_ptr<CCollisionQuadTreeNode>& Node);
	void EraseCollisionNodeList(const std::weak_ptr<CCollisionQuadTreeNode>& Node);

	void AddMouseCollisionNodeList(const std::weak_ptr<CCollisionQuadTreeNode>& Node);
	void EraseMouseCollisionNodeList(const std::weak_ptr<CCollisionQuadTreeNode>& Node);

public:
	bool Init();
	void Update(float DeltaTime);

	void AddCollider(const std::weak_ptr<class CCollider>& Collider);

	void Collision(float DeltaTime);
	void CollisionMouse(std::weak_ptr<class CCollider>& Result, float DeltaTime, const FVector2& MousePos);

	void Render();
	void ReturnNodePool();

public:
	std::shared_ptr<CCollisionQuadTree> GetThisPtr()
	{
		return shared_from_this();
	}


};

