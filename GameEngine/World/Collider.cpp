#include "Collider.h"

#include "../Asset/CBufferTransform.h"
#include "../Asset/CBufferCollider.h"
#include "../Asset/Shader.h"
#include "../Asset/Mesh.h"

#include "World.h"
#include "WorldCollision.h"
#include "CameraManager.h"
#include "CollisionInfoManager.h"

CCollider::CCollider()
{}

CCollider::CCollider(const CCollider& src)
	:CSceneComponent(src)
{}

CCollider::CCollider(CCollider&& src) noexcept
	:CSceneComponent(std::move(src))
{}

CCollider::~CCollider()
{

}

void CCollider::SetDebugDraw(bool DebugDraw)
{
	mDebugDraw = DebugDraw;

	if (mDebugDraw)
	{
		mCBufferTransform.reset();

		mCBufferTransform = std::make_shared<CCBufferTransform>();

		mCBufferTransform->Init();
	}
}

void CCollider::SetCollisionProfile(const std::string& Name)
{
	mProfile = CCollisionInfoManager::GetInst()->FindProfile(Name);
}

bool CCollider::CheckCollisionObject(std::weak_ptr<CCollider> Collider)
{
	auto iter = mColliderMap.find(Collider.lock().get());

	if (iter == mColliderMap.end())
	{
		return false;
	}

	return true;
}

std::unordered_map<CCollider*, std::weak_ptr<CCollider>>::iterator CCollider::EraseCollisionObject(std::weak_ptr<CCollider> Collider)
{
	auto iter = mColliderMap.find(Collider.lock().get());

	if (iter == mColliderMap.end())
	{
		return iter;
	}

	iter = mColliderMap.erase(iter);

	//현재 충돌중인 컴포넌트가 없다면 충돌상태를 false로 돌려준다.
	if (mColliderMap.empty())
	{
		mCollision = false;
	}

	return iter;
}

bool CCollider::Init()
{
	CSceneComponent::Init();

	if (mDebugDraw)
	{
		mCBufferTransform = std::make_shared<CCBufferTransform>();

		mCBufferTransform->Init();
	}

	SetCollisionProfile("Static");

	auto World = mWorld.lock();

	if (World)
	{
		auto WorldCollision = World->GetWorldCollision().lock();

		if (WorldCollision)
		{
			WorldCollision->AddCollider(GetThisPtr<CCollider>());
		}
	}


	return true;
}

void CCollider::Update(float DeltaTime)
{
	CSceneComponent::Update(DeltaTime);
}

void CCollider::PostUpdate(float DeltaTime)
{
	CSceneComponent::PostUpdate(DeltaTime);
}

void CCollider::Render()
{
	CSceneComponent::Render();

	//debugdraw가 활성화되어있을때
	//렌더링을 시도한다.

	if (mDebugDraw)
	{
		if (mMesh.expired() || mShader.expired())
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

		auto Mesh = mMesh.lock();
		auto Shader = mShader.lock();

		FMatrix ScaleMat, RotMat, TranslationMat, WorldMat;

		ScaleMat.Scaling(mWorldScale);
		RotMat.Rotation(mWorldRot);
		TranslationMat.Translation(mWorldPos);

		WorldMat = ScaleMat * RotMat * TranslationMat;

		mCBufferTransform->SetWorldMatrix(WorldMat);
		mCBufferTransform->SetViewMatrix(ViewMat);
		mCBufferTransform->SetProjMatrix(ProjMat);

		mCBufferTransform->UpdateBuffer();

		if (mCollision)
		{
			mCBufferCollider->SetColor(FVector4::Red);
		}
		else
		{
			mCBufferCollider->SetColor(FVector4::Green);
		}

		mCBufferCollider->UpdateBuffer();

		Shader->SetShader();
		Mesh->Render();
	}
}

void CCollider::Destroy()
{
	CSceneComponent::Destroy();

	if (!mColliderMap.empty())
	{
		auto iter = mColliderMap.begin();

		for (; iter != mColliderMap.end();)
		{
			if (iter->second.expired())
			{
				iter = EraseCollisionObject(iter->second);
				continue;
			}

			auto Dest = iter->second.lock();

			if (Dest)
			{
				//CallCollisionEndOverlap(Dest);

				if (mEndOverlapFunc)
				{
					mEndOverlapFunc(Dest);
				}

				Dest->CallCollisionEndOverlap(GetThisPtr<CCollider>());

				Dest->EraseCollisionObject(GetThisPtr<CCollider>());
			}

			iter = EraseCollisionObject(iter->second);
		}
	}
}

void CCollider::CallCollisionBeginOverlap(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<CCollider> Collider)
{
	//콜라이더 맵에 새로운 충돌체를 추가해준다.
	auto _Collider = Collider.lock();

	mColliderMap.insert(std::make_pair(_Collider.get(), Collider));

	mCollision = true;

	if (mBeginOverlapFunc)
	{
		mBeginOverlapFunc(HitPoint, Normal, Collider);
	}

}

void CCollider::CallCollisionEndOverlap(std::weak_ptr<CCollider> Collider)
{
	//충돌이 끝났으니 충돌체를 맵에서 지워준다.
	EraseCollisionObject(Collider);

	if (mEndOverlapFunc)
	{
		mEndOverlapFunc(Collider);
	}

}

void CCollider::CallCollisionHit(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<CCollider> Collider)
{
	if (mHitFunc)
	{
		mHitFunc(HitPoint, Normal, Collider);
	}
}

void CCollider::Save(std::ofstream& File) const
{
	CSceneComponent::Save(File);
	File << "Profile=" << GetCollisionProfileName() << "\n";
}

void CCollider::Load(const std::unordered_map<std::string, std::string>& Props)
{
	CSceneComponent::Load(Props);
	auto it = Props.find("Profile");
	if (it != Props.end() && !it->second.empty())
		SetCollisionProfile(it->second);
}
