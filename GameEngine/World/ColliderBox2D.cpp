#include "ColliderBox2D.h"
#include "Collision.h"

#include "../Asset/AssetManager.h"
#include "../Asset/MeshManager.h"
#include "../Asset/Mesh.h"
#include "../Asset/ShaderFrame.h"
#include "../Asset/ShaderManager.h"
#include "../Asset/CBufferCollider.h"
#include "World.h"

CColliderBox2D::CColliderBox2D()
{
	mColliderType = EColliderType::Box2D;
}

CColliderBox2D::CColliderBox2D(const CColliderBox2D& src)
	:CCollider(src)
{}

CColliderBox2D::CColliderBox2D(CColliderBox2D&& src) noexcept
	:CCollider(std::move(src))
{}

CColliderBox2D::~CColliderBox2D()
{}

void CColliderBox2D::SetDebugDraw(bool DebugDraw)
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

			mMesh = MeshMgr->FindMesh("FrameRect");
		}

		mCBufferCollider.reset();

		mCBufferCollider = std::make_shared<CCBufferCollider>();

		mCBufferCollider->Init();
	}
}

bool CColliderBox2D::Init()
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

			mMesh = MeshMgr->FindMesh("FrameRect");
		}

		mCBufferCollider = std::make_shared<CCBufferCollider>();

		mCBufferCollider->Init();

	}

	return true;
}

void CColliderBox2D::Update(float DeltaTime)
{
	CCollider::Update(DeltaTime);
}

void CColliderBox2D::PostUpdate(float DeltaTime)
{
	CCollider::PostUpdate(DeltaTime);
}

void CColliderBox2D::Destroy()
{
	CCollider::Destroy();
}

void CColliderBox2D::UpdateInfo()
{
	//사각형의 중심점을 기준으로 현재 축이랑 min,max값을 갱신한다.
	mInfo.Center = mWorldPos;

	//축갱신
	mInfo.Axis[EAxis::X] = GetAxis(EAxis::X);
	mInfo.Axis[EAxis::Y] = GetAxis(EAxis::Y);

	//min, max값 갱신.

	//x축이랑 y축이 회전으로 인해서 뒤바뀌었을 가능성이 있기 때문에
	//4개의 꼭지점을 비교해서 값을 구한다.

	FVector3 Pos[4];

	//좌하단
	Pos[0] = mInfo.Center - mInfo.Axis[EAxis::X] * mInfo.Halfsize.x - mInfo.Axis[EAxis::Y] * mInfo.Halfsize.y;
	//좌상단
	Pos[1] = mInfo.Center - mInfo.Axis[EAxis::X] * mInfo.Halfsize.x + mInfo.Axis[EAxis::Y] * mInfo.Halfsize.y;
	//우하단
	Pos[2] = mInfo.Center + mInfo.Axis[EAxis::X] * mInfo.Halfsize.x - mInfo.Axis[EAxis::Y] * mInfo.Halfsize.y;
	//우상단
	Pos[3] = mInfo.Center + mInfo.Axis[EAxis::X] * mInfo.Halfsize.x + mInfo.Axis[EAxis::Y] * mInfo.Halfsize.y;

	mMin = Pos[0];
	mMax = Pos[0];

	for (int i = 0; i < 4; ++i)
	{
		mMin.x = mMin.x > Pos[i].x ? Pos[i].x : mMin.x;
		mMin.y = mMin.y > Pos[i].y ? Pos[i].y : mMin.y;

		mMax.x = mMax.x < Pos[i].x ? Pos[i].x : mMax.x;
		mMax.y = mMax.y < Pos[i].y ? Pos[i].y : mMax.y;
	}

	//스케일 조절
	mWorldScale.x = mInfo.Halfsize.x * 2.f;
	mWorldScale.y = mInfo.Halfsize.y * 2.f;
	mWorldScale.z = 1.f;

}

CColliderBox2D* CColliderBox2D::Clone() const
{
	return new CColliderBox2D(*this);
}

bool CColliderBox2D::Collision(FVector3& HitPoint, FVector3& Normal, float& Depth, std::shared_ptr<CCollider>& Dest)
{
	switch (Dest->GetColliderType())
	{
	case EColliderType::Box2D:
		return CCollision::CollisionBox2DToBox2D(HitPoint, Normal, Depth, this, dynamic_cast<CColliderBox2D*>(Dest.get()));
	case EColliderType::Sphere2D:
		return CCollision::CollisionBox2DToSphere2D(HitPoint, Normal, Depth, this, dynamic_cast<CColliderSphere2D*>(Dest.get()));
	}

	return false;
}

bool CColliderBox2D::CollisionMouse(const FVector2& MousePos)
{
	FVector3 HitPoint, Normal, Point;
	float Depth;

	Point.x = MousePos.x;
	Point.y = MousePos.y;

	return CCollision::CollisionPointToOBB2D(HitPoint, Normal, Depth, GetInfo(), Point);
}

void CColliderBox2D::Save(std::ofstream& File) const
{
	CCollider::Save(File);
	File << "HalfX=" << mInfo.Halfsize.x << "\n";
	File << "HalfY=" << mInfo.Halfsize.y << "\n";
}

void CColliderBox2D::Load(const std::unordered_map<std::string, std::string>& Props)
{
	CCollider::Load(Props);
	float hx = 0.5f, hy = 0.5f;
	{
		auto it = Props.find("HalfX");
		if (it != Props.end()) hx = std::stof(it->second);
	}
	{
		auto it = Props.find("HalfY");
		if (it != Props.end()) hy = std::stof(it->second);
	}
	mInfo.Halfsize = FVector2(hx, hy);
}
