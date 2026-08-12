#include "ColliderSphere2D.h"

#include "Collision.h"

#include "../Asset/AssetManager.h"
#include "../Asset/MeshManager.h"
#include "../Asset/Mesh.h"
#include "../Asset/ShaderFrame.h"
#include "../Asset/ShaderManager.h"
#include "../Asset/CBufferCollider.h"
#include "World.h"

CColliderSphere2D::CColliderSphere2D()
{
	mColliderType = EColliderType::Sphere2D;
}

CColliderSphere2D::CColliderSphere2D(const CColliderSphere2D& src)
	:CCollider(src)
{}

CColliderSphere2D::CColliderSphere2D(CColliderSphere2D&& src) noexcept
	:CCollider(std::move(src))
{}

CColliderSphere2D::~CColliderSphere2D()
{}

void CColliderSphere2D::SetDebugDraw(bool DebugDraw)
{
	CCollider::SetDebugDraw(DebugDraw);

	if (mDebugDraw)
	{
		if (mShader.expired())
		{
			auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

			mShader = ShaderMgr->FindShader("FrameColor2D");
		}

		if (mMesh.expired())
		{
			auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);

			mMesh = MeshMgr->FindMesh("FrameSphere");
		}

		mCBufferCollider.reset();

		mCBufferCollider = std::make_shared<CCBufferCollider>();

		mCBufferCollider->Init();
	}
}

bool CColliderSphere2D::Init()
{
	CCollider::Init();

	if (mDebugDraw)
	{
		if (mShader.expired())
		{
			auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

			mShader = ShaderMgr->FindShader("FrameColor2D");
		}

		if (mMesh.expired())
		{
			auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);

			mMesh = MeshMgr->FindMesh("FrameSphere");
		}

		mCBufferCollider = std::make_shared<CCBufferCollider>();

		mCBufferCollider->Init();

	}

	return true;
}

void CColliderSphere2D::Update(float DeltaTime)
{
	CCollider::Update(DeltaTime);
}

void CColliderSphere2D::PostUpdate(float DeltaTime)
{
	CCollider::PostUpdate(DeltaTime);
}

void CColliderSphere2D::Destroy()
{
	CCollider::Destroy();
}

void CColliderSphere2D::UpdateInfo()
{
	//원의 중심점 갱신
	mInfo.Center = mWorldPos;

	//원의 min, max값은 항상 동일하다.
	mMin = mInfo.Center - mInfo.Radius;
	mMax = mInfo.Center + mInfo.Radius;

	//스케일 조절
	mWorldScale.x = mInfo.Radius;
	mWorldScale.y = mInfo.Radius;
	mWorldScale.z = 1.f;
}

CColliderSphere2D* CColliderSphere2D::Clone() const
{
	return new CColliderSphere2D(*this);
}

bool CColliderSphere2D::Collision(FVector3& HitPoint, FVector3& Normal, float& Depth, std::shared_ptr<CCollider>& Dest)
{
	switch (Dest->GetColliderType())
	{
	case EColliderType::Box2D:
		return CCollision::CollisionSphere2DToBox2D(HitPoint, Normal, Depth, this, dynamic_cast<CColliderBox2D*>(Dest.get()));
	case EColliderType::Sphere2D:
		return CCollision::CollisionSphere2DToSphere2D(HitPoint, Normal, Depth, this, dynamic_cast<CColliderSphere2D*>(Dest.get()));
	}

	return false;
}

bool CColliderSphere2D::CollisionMouse(const FVector2& MousePos)
{
	FVector3 HitPoint, Normal, Point;
	float Depth;

	Point.x = MousePos.x;
	Point.y = MousePos.y;

	return CCollision::CollisionPointToSphere2D(HitPoint, Normal, Depth, GetInfo(), Point);
}

void CColliderSphere2D::Save(std::ofstream& File) const
{
	CCollider::Save(File);
	File << "Radius=" << mInfo.Radius << "\n";
}

void CColliderSphere2D::Load(const std::unordered_map<std::string, std::string>& Props)
{
	CCollider::Load(Props);
	auto it = Props.find("Radius");
	if (it != Props.end()) mInfo.Radius = std::stof(it->second);
}
